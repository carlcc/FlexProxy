#include "FlexServer/FlexServer.h"
#include "FlexCommon/AddressParser.h"
#include "FlexCommon/Log.h"
#include "FlexServer/Forwarder.h"

namespace FS {

FlexServer::FlexServer(asio::io_context& ioContext, const FlexServerConfig& config)
    : ioContext_ { ioContext }
    , config_ { config }
    , udpSocket_ { ioContext }
{
}

FlexServer::~FlexServer()
{
}

bool FlexServer::Initialize()
{
    {
        if (!AddressParser::ParseSocketAddress(tcpSocketEndpoint_, config_.tcpProxyForAddr)) {
            L_ERROR("Failed to parse tcpProxyForAddr!");
            return false;
        }
        L_INFO("We will do reverse proxy for {}:{}", tcpSocketEndpoint_.address().to_string(), tcpSocketEndpoint_.port());
    }

    asio_udp::endpoint& ep = udpSocketEndpoint_;
    if (!AddressParser::ParseSocketAddress(ep, config_.kcpBindAddr)) {
        L_ERROR("Failed to parse serviceAddress!");
        return false;
    }
    L_INFO("Kcp server will listen on {}:{}", ep.address().to_string(), ep.port());

    std::error_code ec;
    udpSocket_.open(ep.address().is_v4() ? asio_udp::v4() : asio_udp::v6(), ec);
    if (ec) {
        L_ERROR("Failed to open udp address {}: {}", config_.kcpBindAddr, ec.message());
        return false;
    }

    udpSocket_.set_option(asio_udp::socket::reuse_address(true));

    udpSocket_.bind(ep, ec);
    if (ec) {
        L_ERROR("Failed to bind udp address {}: {}", config_.kcpBindAddr, ec.message());
        return false;
    }
    return true;
}

void FlexServer::Start()
{
    asio::co_spawn(
        ioContext_, [this]() noexcept -> asio::awaitable<void> {
            co_await UdpAcceptCoroutine();
        },
        asio::detached);
}

asio::awaitable<void> FlexServer::UdpAcceptCoroutine()
{
    char receiveBufferMemory[4096];
    auto receiveBuffer = asio::buffer(receiveBufferMemory);
    asio_udp::endpoint remoteEndpoint;
    while (true) {
        size_t n;
        try {
            n = co_await udpSocket_.async_receive_from(receiveBuffer, remoteEndpoint, asio::use_awaitable);
        } catch (std::system_error& e) {
            auto ec = e.code();
            if (IsTryAgain(ec)) {
                continue;
            }
            if (ec == std::errc::operation_canceled) {
                L_INFO("Kcp service socket closed, stop accepting.");
                break;
            }
            L_ASSERT(false, "Unhandled error {} during accepting kcp connection.", ec.message());
        }

        // TODO: For a udp protocol, socket address is not necessarily be the identifier of a session(or connection),
        //      use high level `sessionid` to identify a session
        auto it = forwarderMap_.find(remoteEndpoint);
        if (it == forwarderMap_.end()) {
            // It is a new socket address, let's create a new forwarder for it
            auto* newForwarder = new Forwarder(ioContext_);
            std::shared_ptr<Forwarder> spForwarder(newForwarder, [this, remoteEndpoint](Forwarder* p) {
                forwarderMap_.erase(remoteEndpoint); // Note: this may fail, but we don't care
                delete p;
            });

            if (newForwarder->Start(udpSocketEndpoint_, remoteEndpoint, tcpSocketEndpoint_)) {
                auto result = forwarderMap_.insert({ remoteEndpoint, spForwarder });
                L_ASSERT(result.second, "Failed to insert into forwarder map, bug!!!");
                newForwarder->OnUdpData(receiveBufferMemory, n);
            } else {
                L_INFO("Failed to start forwarder!");
                // TODO: Reply something to peer?
                co_return;
            }
        } else {
            // A forwarder has been created for it, let's just forward the data to them
            auto sp = it->second.lock();
            L_ASSERT(sp != nullptr, "Since this is a single thread server, this should not happen");
            sp->OnUdpData(receiveBufferMemory, n);
        }
    }

    L_INFO("Kcp accept coroutine stopped");
}

}