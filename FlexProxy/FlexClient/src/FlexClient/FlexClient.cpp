#include "FlexClient/FlexClient.h"
#include "FlexClient/ClientSideForwarder.h"
#include "FlexCommon/AddressParser.h"
#include "FlexCommon/Log.h"

namespace FS {

FlexClient::FlexClient(asio::io_context& ioContext, const FlexClientConfig& config)
    : ioContext_ { ioContext }
    , config_ { config }
    , acceptor_ { ioContext }
{
}

FlexClient::~FlexClient()
{
}

bool FlexClient::Initialize()
{
    {
        if (!AddressParser::ParseSocketAddress(kcpSocketEndpoint_, config_.kcpServerAddr)) {
            L_ERROR("Failed to parse kcpServerAddr!");
            return false;
        }
        L_INFO("We will do reverse proxy for {}:{}", kcpSocketEndpoint_.address().to_string(), kcpSocketEndpoint_.port());
    }

    asio_tcp::endpoint ep;
    if (!AddressParser::ParseSocketAddress(ep, config_.tcpAcceptorAddr)) {
        L_ERROR("Failed to parse serviceAddress!");
        return false;
    }
    L_INFO("Tcp server will listen on {}:{}", ep.address().to_string(), ep.port());

    std::error_code ec;

    acceptor_.open(ep.protocol(), ec);
    if (ec) {
        L_ERROR("Failed to open acceptor for {}:{}: {}", ep.address().to_string(), ep.port(), ec.message());
        return false;
    }
    acceptor_.set_option(asio_tcp::socket::reuse_address(true));
    acceptor_.bind(ep, ec);
    if (ec) {
        L_ERROR("Failed to bind tcp address {}: {}", config_.tcpAcceptorAddr, ec.message());
        return false;
    }
    acceptor_.listen(100, ec);
    if (ec) {
        L_ERROR("Failed to listen on {}: {}", config_.tcpAcceptorAddr, ec.message());
        return false;
    }
    return true;
}

void FlexClient::Start()
{
    asio::co_spawn(
        ioContext_, [this]() noexcept -> asio::awaitable<void> {
            co_await TcpAcceptCoroutine();
        },
        asio::detached);
}

asio::awaitable<void> FlexClient::TcpAcceptCoroutine()
{
    asio_udp::endpoint remoteEndpoint;
    while (true) {
        asio_tcp::socket clientSocket { ioContext_ };
        try {
            co_await acceptor_.async_accept(clientSocket, asio::use_awaitable);
        } catch (std::system_error& e) {
            auto ec = e.code();
            if (IsTryAgain(ec)) {
                continue;
            }
            if (ec == std::errc::operation_canceled) {
                L_INFO("Tcp service socket closed, stop accepting.");
                break;
            }
            L_ASSERT(false, "Unhandled error {} during accepting tcp connection.", ec.message());
        }

        auto spClient = std::make_shared<ClientSideForwarder>(ioContext_);
        spClient->Start(kcpSocketEndpoint_, std::move(clientSocket));
    }

    L_INFO("Kcp accept coroutine stopped");
}

}