#pragma once
#include "FlexCommon/Asio.h"
#include "FlexCommon/ikcp.h"
#include <memory>

namespace FS {

class ClientSideForwarder : public std::enable_shared_from_this<ClientSideForwarder> {
public:
    explicit ClientSideForwarder(asio::io_context& ioContext);
    ~ClientSideForwarder();

    bool Start(const asio::ip::udp::endpoint& remoteUdpEndpoint, asio_tcp::socket&& tcpSocket);

private:
    void Stop();

private:
    asio::io_context& ioContext_;
    asio_udp::socket udpSocket_;
    asio_tcp::socket tcpSocket_;
    uint32_t id_;
    ikcpcb* kcpCb_;
    bool isRunning_;
    bool isKcpAlive_;
};
}
