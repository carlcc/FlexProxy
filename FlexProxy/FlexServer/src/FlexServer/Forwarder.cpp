#include "FlexServer/Forwarder.h"
#include "FlexCommon/Log.h"
#include "FlexCommon/MiscUtils.h"
#include "FlexCommon/Time.h"
#include <chrono>

#define FORARDER_L_INFO(fmt, ...) L_INFO(fmt " ({})", ##__VA_ARGS__, id_)
#define FORARDER_L_ERROR(fmt, ...) L_ERROR(fmt " ({})", ##__VA_ARGS__, id_)
#define FORARDER_SCOPE_EXIT_INFO(fmt, ...) SCOPE_EXIT_INFO(fmt " ({})", ##__VA_ARGS__, id_)

namespace FS {

using namespace std::chrono_literals;

static uint32_t gIdCounter = 0;

Forwarder::Forwarder(asio::io_context& ioContext)
    : ioContext_ { ioContext }
    , udpSocket_ { ioContext }
    , tcpSocket_ { ioContext }
    , id_ { gIdCounter++ }
{
    FORARDER_L_INFO("Construct forwarder");
}

Forwarder::~Forwarder()
{
    L_ASSERT(!isRunning_, "Bug!");
    ikcp_release(kcpCb_);
    FORARDER_L_INFO("Destruct forwarder {}", (void*)this);
}

bool Forwarder::Start(const asio::ip::udp::endpoint& localUdpEndpoint, const asio::ip::udp::endpoint& remoteUdpEndpoint,
    const asio::ip::tcp::endpoint& remoteTcpEndpoint)
{
    auto spThis = shared_from_this();

    //  Create udp socket
    boost::system::error_code ec;
    udpSocket_.open(localUdpEndpoint.protocol(), ec);
    if (ec.failed()) {
        FORARDER_L_ERROR("Failed to open udp address {}:{}: {}", localUdpEndpoint.address().to_string(), localUdpEndpoint.port(), ec.message());
        return false;
    }
    udpSocket_.set_option(asio_udp::socket::reuse_address(true));
    udpSocket_.bind(localUdpEndpoint, ec);
    if (ec.failed()) {
        FORARDER_L_ERROR("Failed to bind udp address {}:{}: {}", localUdpEndpoint.address().to_string(), localUdpEndpoint.port(), ec.message());
        return false;
    }
    udpSocket_.connect(remoteUdpEndpoint, ec);
    if (ec.failed()) {
        FORARDER_L_ERROR("Failed to connect udp address {}:{}: {}", remoteUdpEndpoint.address().to_string(), remoteUdpEndpoint.port(), ec.message());
        return false;
    }

    asio::spawn(ioContext_, [this, spThis, remoteTcpEndpoint](asio::yield_context yield) noexcept {
        FORARDER_SCOPE_EXIT_INFO("Forward start coroutine complete...");

        // TODO:
        //  Create tcp socket and connect
        boost::system::error_code ec;
        tcpSocket_.open(remoteTcpEndpoint.protocol(), ec);
        if (ec.failed()) {
            FORARDER_L_ERROR("Failed to open tcp socket: {}", ec.message());
            return;
        }
        tcpSocket_.async_connect(remoteTcpEndpoint, yield[ec]);
        if (ec.failed()) {
            FORARDER_L_ERROR("Failed to connect to {}:{}: {}", remoteTcpEndpoint.address().to_string(), remoteTcpEndpoint.port(), ec.message());
            return;
        }

        isRunning_ = true;

        asio::spawn(ioContext_, [this, spThis](asio::yield_context yield) noexcept {
            FORARDER_SCOPE_EXIT_INFO("tcp to kcp coroutine complete...");
            // Receive from tcp, send to kcp
            boost::system::error_code ec;
            char receiveBufferMemory[4096];
            auto receiveBuffer = asio::buffer(receiveBufferMemory, sizeof(receiveBufferMemory));

            asio::steady_timer timer { ioContext_ };
            while (isRunning_) {
                auto n = tcpSocket_.async_receive(receiveBuffer, yield[ec]);
                if (ec.failed()) {
                    if (IsTryAgain(ec)) {
                        continue;
                    }
                    FORARDER_L_ERROR("Failed to receive tcp: {}", ec.message());
                    Stop();
                    return;
                }

                ikcp_send(kcpCb_, receiveBufferMemory, (int)n);

                // 下面似乎会导致问题，git clone 拉代码时链接断开
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
        });
        asio::spawn(ioContext_, [this, spThis](asio::yield_context yield) noexcept {
            FORARDER_SCOPE_EXIT_INFO("kcp to tcp coroutine complete...");
            using namespace std::chrono;
            asio::steady_timer timer { ioContext_ };
            boost::system::error_code ec;

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
                        asio::async_write(tcpSocket_, asio::const_buffer(receiveBuffer, n), yield[ec]);
                        if (ec.failed()) {
                            if (IsTryAgain(ec)) {
                                timer.expires_after(2ms);
                                timer.async_wait(yield[ec]);
                                continue;
                            }
                            FORARDER_L_INFO("Failed to write tcp: {}", ec.message());
                            Stop();
                            return;
                        }
                        break;
                    } while (true);
                } while (true);

                timer.expires_at(now + 10ms);
                timer.async_wait(yield[ec]);
                if (ec.failed()) {
                    L_ASSERT(false, "Unhandled error!");
                    return;
                }
                now = steady_clock::now();
            }
        });
        asio::spawn(ioContext_, [this, spThis](asio::yield_context yield) noexcept {
            FORARDER_SCOPE_EXIT_INFO("kcp feed coroutine complete...");
            uint8_t receiveBufferMemory[4096];
            auto receiveBuffer = asio::buffer(receiveBufferMemory, sizeof(receiveBufferMemory));
            boost::system::error_code ec;
            while (isRunning_) {
                auto n = udpSocket_.async_receive(receiveBuffer, yield[ec]);
                if (ec.failed()) {
                    if (IsTryAgain(ec)) {
                        continue;
                    }
                    FORARDER_L_ERROR("Failed to receive udp: {}", ec.message());
                    Stop();
                    return;
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
                MiscUtils::Encrypt(receiveBufferMemory , n);
                ikcp_input(kcpCb_, (const char*)receiveBufferMemory , (int)n);
#endif
            }
        });
        asio::spawn(ioContext_, [this, spThis](asio::yield_context yield) noexcept {
            FORARDER_SCOPE_EXIT_INFO("active timer coroutine complete...");
            using namespace std::chrono;
            asio::steady_timer timer { ioContext_ };
            boost::system::error_code ec;

            while (isRunning_) {
                timer.expires_after(5s);
                timer.async_wait(yield[ec]);
                L_ASSERT(!ec.failed(), "Bug unhandled error");

                if (isKcpAlive_) {
                    isKcpAlive_ = false;
                } else {
                    FORARDER_L_INFO("The user is inactive for too long, close()");
                    Stop();
                    return;
                }
            }
        });
    });

    // create kcp callback
    kcpCb_ = ikcp_create(0x5a3331fd, this);
    kcpCb_->output = [](const char* buf, int len, ikcpcb* kcp, void* user) -> int {
        auto* self = reinterpret_cast<Forwarder*>(user);
        boost::system::error_code ec;
        MiscUtils::Encrypt((uint8_t*)buf, len);

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

    return true;
}

void Forwarder::OnUdpData(const void* data, size_t len)
{
    isKcpAlive_ = true;
#ifdef ENABLE_CHECK_SUM
    if (len > 1) {
        auto* ptr = (uint8_t*)data;
        auto checkSum = MiscUtils::XorCheck(ptr + 1, len - 1);
        if (checkSum == ptr[0]) {
            MiscUtils::Encrypt(ptr + 1, len - 1);
            ikcp_input(kcpCb_, (const char*)ptr + 1, (int)len - 1);
        } else {
            FORARDER_L_ERROR("Checksum error!");
        }
        // MiscUtils::Encrypt((uint8_t*)data, len);
        // ikcp_input(kcpCb_, (const char*)data, len);
    }
#else
    MiscUtils::Encrypt((uint8_t*)data, len);
    ikcp_input(kcpCb_, (const char*)data, len);
#endif
}

void Forwarder::Stop()
{
    isRunning_ = false;
    boost::system::error_code ec;
    udpSocket_.close(ec);
    tcpSocket_.close(ec);
}

}