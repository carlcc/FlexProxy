#pragma once

#include "FlexCommon/Asio.h"
#include "FlexCommon/ikcp.h"
#include <memory>

namespace FS {

class Forwarder : public std::enable_shared_from_this<Forwarder> {
public:
    explicit Forwarder(asio::io_context& ioContext);
    ~Forwarder();

    bool Start(const asio::ip::udp::endpoint& localUdpEndpoint, const asio::ip::udp::endpoint& remoteUdpEndpoint,
        const asio::ip::tcp::endpoint& remoteTcpEndpoint);

    void OnUdpData(const void* data, size_t len);

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