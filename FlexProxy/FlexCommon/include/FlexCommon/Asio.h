#pragma once

#include <asio.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/ip/udp.hpp>
#include <asio/signal_set.hpp>
#include <asio/spawn.hpp>
#include <asio/steady_timer.hpp>
#include <asio/write.hpp>

using asio_tcp = asio::ip::tcp;
using asio_udp = asio::ip::udp;

namespace FS {
inline bool IsTryAgain(boost::system::error_code error)
{
    return error == std::errc::resource_unavailable_try_again || error == std::errc::operation_would_block;
}

template <typename Timer>
inline asio::awaitable<std::error_code> AsyncWait(Timer& timer)
{
    try {
        co_await timer.async_wait(asio::use_awaitable);
    } catch (std::system_error& e) {
        co_return e.code();
    }
    co_return std::error_code {};
}

template <typename Socket, typename Buffer>
inline asio::awaitable<std::error_code> AsyncWrite(Socket& socket, Buffer&& buffer)
{
    try {
        co_await asio::async_write(socket, std::forward<Buffer>(buffer), asio::use_awaitable);
    } catch (std::system_error& e) {
        co_return e.code();
    }
    co_return std::error_code {};
}

template <typename Socket, typename Address>
inline asio::awaitable<std::error_code> AsyncConnect(Socket& socket, Address&& address)
{
    try {
        co_await socket.async_connect(std::forward<Address>(address), asio::use_awaitable);
    } catch (std::system_error& e) {
        co_return e.code();
    }
    co_return std::error_code {};
}

}