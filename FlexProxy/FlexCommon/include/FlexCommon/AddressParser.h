
#pragma once

#include "FlexCommon/Asio.h"

namespace FS::AddressParser {

bool ParseSocketAddress(asio_tcp::endpoint& endpoint, std::string_view ipPortStr);
bool ParseSocketAddress(asio_udp::endpoint& endpoint, std::string_view ipPortStr);

}