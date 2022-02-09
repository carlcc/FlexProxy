#include "FlexCommon/AddressParser.h"
#include "FlexCommon/Log.h"
#include <algorithm>
#include <cstdio>
#include <fstream>

namespace FS::AddressParser {

static bool ParseAddressAndPort(asio::ip::address& address, uint16_t& port, std::string_view ipPortStr)
{
    auto index = ipPortStr.rfind(':');
    if (index == std::string::npos || index == ipPortStr.length() - 1) {
        L_ERROR("Invalid socket address {}: expecting a port!", ipPortStr);
        return false;
    }

    boost::system::error_code ec;
    address = asio::ip::make_address(ipPortStr.substr(0, index), ec);
    if (ec.failed()) {
        L_ERROR("Invalid socket address {}: invalid ip {}!", ipPortStr, ipPortStr.substr(0, index));
        return false;
    }

    if (!std::all_of(ipPortStr.data() + index + 1, ipPortStr.data() + ipPortStr.length(), [](char c) {
            return std::isdigit(c);
        })) {
        L_ERROR("Invalid socket address {}: invalid a port {}!", ipPortStr, ipPortStr.substr(index + 1));
        return false;
    }

    int intPort = -1;
    sscanf(ipPortStr.substr(index + 1).data(), "%d", &intPort);
    if (intPort < 0 || intPort > 65535) {
        L_ERROR("Invalid socket address {}: invalid a port {}!", ipPortStr, ipPortStr.substr(index + 1));
        return false;
    }
    port = uint16_t(intPort);
    return true;
}

bool ParseSocketAddress(asio_tcp::endpoint& endpoint, std::string_view ipPortStr)
{
    asio::ip::address ipAddress;
    uint16_t port;
    if (!ParseAddressAndPort(ipAddress, port, ipPortStr)) {
        return false;
    }
    endpoint = asio_tcp::endpoint(ipAddress, port);
    return true;
}

bool ParseSocketAddress(asio::ip::udp::endpoint& endpoint, std::string_view ipPortStr)
{
    asio::ip::address ipAddress;
    uint16_t port;
    if (!ParseAddressAndPort(ipAddress, port, ipPortStr)) {
        return false;
    }
    endpoint = asio_udp::endpoint(ipAddress, port);
    return true;
}

}
