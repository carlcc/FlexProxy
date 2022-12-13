#pragma once
#include "FlexClient/FlexClientConfig.h"
#include "FlexCommon/Asio.h"

namespace FS {

class FlexClient {
public:
    FlexClient(asio::io_context& ioContext, const FlexClientConfig& config);
    ~FlexClient();

    bool Initialize();

    void Start();

private:
    asio::awaitable<void> TcpAcceptCoroutine();

private:
    asio::io_context& ioContext_;
    FlexClientConfig config_;
    asio_tcp::acceptor acceptor_;
    asio_udp::endpoint kcpSocketEndpoint_; ///< The kcp address we are going to connect to
};

}