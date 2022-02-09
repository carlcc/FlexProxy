#pragma once

#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/write.hpp>

namespace asio = boost::asio;
using asio_tcp = boost::asio::ip::tcp;
using asio_udp = boost::asio::ip::udp;

namespace FS {
inline bool IsTryAgain(boost::system::error_code error)
{
    return error == std::errc::resource_unavailable_try_again || error == std::errc::operation_would_block;
}
}