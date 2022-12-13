
#pragma once

#include "FlexCommon/Asio.h"
#include "FlexServer/FlexServerConfig.h"
#include <map>

namespace FS {

class Forwarder;

class FlexServer {
public:
    FlexServer(asio::io_context& ioContext, const FlexServerConfig& config);
    ~FlexServer();

    bool Initialize();

    void Start();

private:
    asio::awaitable<void> UdpAcceptCoroutine();

private:
    asio::io_context& ioContext_;
    FlexServerConfig config_;
    asio_udp::socket udpSocket_;
    asio_udp::endpoint udpSocketEndpoint_; ///< The bind address of `udpSocket_`
    asio_tcp::endpoint tcpSocketEndpoint_; ///< The tcp address of the server we are proxying for

    /// Stores all the forwarders we have now
    std::map<asio_udp::endpoint, std::weak_ptr<Forwarder>> forwarderMap_;
};

} // namespace FS