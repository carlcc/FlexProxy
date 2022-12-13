#include "FlexClient/ClientSideForwarder.h"
#include "FlexCommon/Log.h"
#include "FlexCommon/MiscUtils.h"
#include <chrono>

#define FORARDER_L_INFO(fmt, ...) L_INFO(" ({})" fmt, id_, ##__VA_ARGS__)
#define FORARDER_L_ERROR(fmt, ...) L_ERROR(" ({})" fmt, id_, ##__VA_ARGS__)
#define FORARDER_SCOPE_EXIT_INFO(fmt, ...) SCOPE_EXIT_INFO(" ({})" fmt, id_, ##__VA_ARGS__)

using namespace std::chrono_literals;

namespace FS {

static uint32_t gIdCounter = 0;

ClientSideForwarder::ClientSideForwarder(asio::io_context& ioContext)
    : ioContext_ { ioContext }
    , udpSocket_ { ioContext }
    , tcpSocket_ { ioContext }
    , id_ { gIdCounter++ }
{
    FORARDER_L_INFO("Construct forwarder");
}

ClientSideForwarder::~ClientSideForwarder()
{
    L_ASSERT(!isRunning_, "Bug!");
    ikcp_release(kcpCb_);
    FORARDER_L_INFO("Destruct forwarder");
}

bool ClientSideForwarder::Start(const asio::ip::udp::endpoint& remoteUdpEndpoint, asio_tcp::socket&& tcpSocket)
{
    auto spThis = shared_from_this();

    tcpSocket_ = std::move(tcpSocket);

    //  Create udp socket
    std::error_code ec;
    udpSocket_.open(remoteUdpEndpoint.protocol(), ec);
    if (ec) {
        FORARDER_L_ERROR("Failed to open udp for address {}:{}: {}", remoteUdpEndpoint.address().to_string(), remoteUdpEndpoint.port(), ec.message());
        return false;
    }
    udpSocket_.connect(remoteUdpEndpoint, ec);
    if (ec) {
        FORARDER_L_ERROR("Failed to connect udp address {}:{}: {}", remoteUdpEndpoint.address().to_string(), remoteUdpEndpoint.port(), ec.message());
        return false;
    }
    udpSocket_.set_option(asio_udp::socket::send_buffer_size(4096 * 1024));
    udpSocket_.set_option(asio_udp::socket::receive_buffer_size(4096 * 1024));

    // create kcp callback
    kcpCb_ = ikcp_create(0x5a3331fd, this);
    kcpCb_->output = [](const char* buf, int len, ikcpcb* kcp, void* user) -> int {
        auto* self = reinterpret_cast<ClientSideForwarder*>(user);
        MiscUtils::Encrypt((uint8_t*)buf, len);
        std::error_code ec;
#ifdef ENABLE_CHECK_SUM
        auto checkSum = MiscUtils::XorCheck((const uint8_t*)buf, len);
        std::vector<asio::const_buffer> buffers {
            asio::const_buffer(&checkSum, 1),
            asio::const_buffer(buf, len),
        };
        self->udpSocket_.send(buffers, 0, ec);
#else
        self->udpSocket_.send(asio::const_buffer(buf, len), 0, ec);
#endif

        return 0;
    };
    ikcp_wndsize(kcpCb_, 1024, 1024);
    ikcp_nodelay(kcpCb_, 2, 10, 2, 1);

    isRunning_ = true;
    asio::co_spawn(
        ioContext_, [this, spThis]() noexcept -> asio::awaitable<void> {
            FORARDER_SCOPE_EXIT_INFO("tcp to kcp coroutine complete...");
            // Receive from tcp, send to kcp
            char receiveBufferMemory[8192];
            auto receiveBuffer = asio::buffer(receiveBufferMemory, sizeof(receiveBufferMemory));

            asio::steady_timer timer { ioContext_ };
            while (isRunning_) {
                size_t n;
                try {
                    n = co_await tcpSocket_.async_receive(receiveBuffer, asio::use_awaitable);
                } catch (std::system_error& e) {
                    auto ec = e.code();
                    if (IsTryAgain(ec)) {
                        continue;
                    }
                    FORARDER_L_ERROR("Failed to receive tcp: {}", ec.message());
                    Stop();
                    co_return;
                }

                ikcp_send(kcpCb_, receiveBufferMemory, (int)n);
                // 1400 * (1024 * 10)  = 14MB, the send buffer is about 14MB
                // wait data to be flushed, to avoid to many data in memory
                // while (ikcp_waitsnd(kcpCb_) > 1024 * 10) {
                //     timer.expires_after(5ms);
                //     timer.async_wait(yield[ec]);
                //     if (ec.failed()) {
                //         L_ASSERT(false, "Failed to async wait, should not happen!");
                //         abort();
                //     }
                //     if (!isRunning_) {
                //         return;
                //     }
                // }
            }
        },
        asio::detached);
    asio::co_spawn(
        ioContext_, [this, spThis]() noexcept -> asio::awaitable<void> {
            FORARDER_SCOPE_EXIT_INFO("kcp to tcp coroutine complete...");
            // Receive from kcp, send to tcp
            using namespace std::chrono;
            asio::steady_timer timer { ioContext_ };

            auto now = steady_clock::now();

            char receiveBuffer[4 * 4096];
            while (isRunning_) {
                ikcp_update(kcpCb_, (IUINT32)duration_cast<milliseconds>(now.time_since_epoch()).count());
                do {
                    // Try to read date from kcp, and forward to tcp
                    auto n = ikcp_recv(kcpCb_, receiveBuffer, sizeof(receiveBuffer));
                    if (n <= 0) {
                        break;
                    }

                    do {
                        auto ec = co_await AsyncWrite(tcpSocket_, asio::const_buffer(receiveBuffer, n));
                        if (ec) {
                            if (IsTryAgain(ec)) {
                                timer.expires_after(2ms);
                                co_await AsyncWait(timer);
                                continue;
                            }
                            FORARDER_L_INFO("Failed to write tcp: {}", ec.message());
                            Stop();
                            co_return;
                        }
                        break;
                    } while (true);
                } while (true);

                timer.expires_at(now + 10ms);
                auto ec = co_await AsyncWait(timer);
                if (ec) {
                    L_ASSERT(false, "Unhandled timer error!");
                    co_return;
                }
                now = steady_clock::now();
            }
        },
        asio::detached);
    asio::co_spawn(
        ioContext_, [this, spThis]() noexcept -> asio::awaitable<void> {
            FORARDER_SCOPE_EXIT_INFO("kcp feed coroutine complete...");
            // feed data from network to kcp
            uint8_t receiveBufferMemory[4096];
            auto receiveBuffer = asio::buffer(receiveBufferMemory, sizeof(receiveBufferMemory));
            while (isRunning_) {
                size_t n;
                try {
                    n = co_await udpSocket_.async_receive(receiveBuffer, asio::use_awaitable);
                } catch (std::system_error& e) {
                    auto ec = e.code();
                    if (IsTryAgain(ec)) {
                        continue;
                    }
                    L_ERROR("Failed to receive udp: {}", ec.message());
                    Stop();
                    co_return;
                }
                isKcpAlive_ = true;

#ifdef ENABLE_CHECK_SUM
                if (n > 1) {
                    auto checkSum = MiscUtils::XorCheck((uint8_t*)receiveBufferMemory + 1, n - 1);
                    if (checkSum == receiveBufferMemory[0]) {
                        MiscUtils::Encrypt(receiveBufferMemory + 1, n - 1);
                        ikcp_input(kcpCb_, (const char*)receiveBufferMemory + 1, (int)n - 1);
                    } else {
                        FORARDER_L_ERROR("Checksum error!");
                    }
                } else {
                    FORARDER_L_ERROR("Invalid udp data!");
                }
#else
            MiscUtils::Encrypt(receiveBufferMemory , n );
            ikcp_input(kcpCb_, (const char*)receiveBufferMemory, (int)n);
#endif
            }
        },
        asio::detached);
    //    asio::spawn(ioContext_, [this, spThis](asio::yield_context yield) noexcept {
    //        FORARDER_SCOPE_EXIT_INFO("active timer coroutine complete...");
    //        using namespace std::chrono;
    //        asio::steady_timer timer { ioContext_ };
    //        boost::system::error_code ec;
    //
    //        while (isRunning_) {
    //            timer.expires_after(5s);
    //            timer.async_wait(yield[ec]);
    //            L_ASSERT(!ec.failed(), "Bug unhandled error");
    //
    //            if (isKcpAlive_) {
    //                isKcpAlive_ = false;
    //            } else {
    //                L_INFO("The user is inactive for too long, close()");
    //                Stop();
    //                return;
    //            }
    //        }
    //    });

    return true;
}

void ClientSideForwarder::Stop()
{
    isRunning_ = false;
    std::error_code ec;
    udpSocket_.close(ec);
    tcpSocket_.close(ec);
}

}