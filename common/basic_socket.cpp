/* Copyright (C) 2019 Nemirtingas
 * This file is part of Socket.
 *
 * Socket is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Socket is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Socket.  If not, see <https://www.gnu.org/licenses/>
 */

#include <Socket/common/basic_socket.h>

using namespace PortableAPI;

bool basic_socket::isvalid() const
{
    return (_sock.get() != nullptr && *_sock != Socket::invalid_socket);
}

class LOCAL_API SocketDeleter
{
public:
    void operator()(Socket::socket_t*s)
    {
        if (s != nullptr)
        {
            if (*s != Socket::invalid_socket)
            {
                Socket::shutdown(*s, Socket::shutdown_flags::both);
                Socket::closeSocket(*s);
            }
            delete s;
        }
    }
};

basic_socket::basic_socket()
{}

basic_socket::basic_socket(basic_socket const& other) :_sock(other._sock)
{}

basic_socket::basic_socket(basic_socket&& other) : _sock(std::move(other._sock))
{}

basic_socket::basic_socket(Socket::socket_t s) : _sock(new Socket::socket_t(s), SocketDeleter())
{}

basic_socket::~basic_socket()
{}

basic_socket& basic_socket::operator =(basic_socket const& other)
{
    _sock = other._sock;
    return *this;
}

basic_socket& basic_socket::operator =(basic_socket&& other)
{
    _sock = std::move(other._sock);
    return *this;
}

void basic_socket::ioctlsocket(Socket::cmd_name cmd, unsigned long* arg)
{
    auto res = Socket::ioctlsocket(*_sock, static_cast<long>(cmd), arg);
    if(res)
    {
#if defined(__WINDOWS__)
        int error = WSAGetLastError();
#elif defined(__LINUX__) || defined(__APPLE__)
        int error = errno;
#endif
        throw socket_exception("ioctlsocket exception " + std::to_string(error));
    }
}

void basic_socket::setsockopt(Socket::level level, Socket::option_name optname, const void* optval, socklen_t optlen)
{
    auto res = Socket::setsockopt(*_sock, static_cast<int>(level), static_cast<int>(optname), optval, optlen);
    if (res)
    {
#if defined(__WINDOWS__)
        int error = WSAGetLastError();
#elif defined(__LINUX__) || defined(__APPLE__)
        int error = errno;
#endif
        throw socket_exception("setsockopt exception " + std::to_string(error));
    }
}

void basic_socket::getsockopt(Socket::level level, Socket::option_name optname, void* optval, socklen_t *optlen)
{
    auto res = Socket::getsockopt(*_sock, static_cast<int>(level), static_cast<int>(optname), optval, optlen);
    if (res)
    {
#if defined(__WINDOWS__)
        int error = WSAGetLastError();
#elif defined(__LINUX__) || defined(__APPLE__)
        int error = errno;
#endif
        throw socket_exception("getsockopt exception " + std::to_string(error));
    }
}

void basic_socket::shutdown(Socket::shutdown_flags how)
{
    if (Socket::shutdown(*_sock, how))
        throw socket_exception("shutdown exception");
}

void basic_socket::set_nonblocking(bool non_blocking)
{
    unsigned long mode = (non_blocking ? 1 : 0);
    ioctlsocket(Socket::cmd_name::fionbio, &mode);
}

void basic_socket::socket(Socket::address_family af, Socket::types type, Socket::protocols proto)
{
    Socket::socket_t s = Socket::socket(af, type, proto);
    if (s == Socket::invalid_socket)
        throw socket_exception("Cannot build socket");
    
    _sock.reset(new Socket::socket_t(s), SocketDeleter());
}

void basic_socket::bind(basic_addr& addr)
{
    Socket::bind(*_sock, addr);
}

Socket::socket_t basic_socket::get_sock() const
{
    return *_sock;
}

void basic_socket::reset_socket(Socket::socket_t s)
{
    _sock.reset(new Socket::socket_t(s), SocketDeleter());
}

///////////////////////////////////////////////////////////////////////////////
// connected_socket class
///////////////////////////////////////////////////////////////////////////////

connected_socket::connected_socket():basic_socket()
{}

connected_socket::connected_socket(connected_socket const &other) : basic_socket(other)
{}

connected_socket::connected_socket(connected_socket &&other) : basic_socket(std::move(other))
{}

connected_socket::connected_socket(Socket::socket_t s) : basic_socket(s)
{}

connected_socket::~connected_socket()
{}

void connected_socket::listen(int waiting_socks)
{
    Socket::listen(*_sock, waiting_socks);
}

void connected_socket::connect(basic_addr & addr)
{
    Socket::connect(*_sock, addr);
}

size_t connected_socket::recv(void* buffer, size_t len, Socket::socket_flags flags)
{
    return Socket::recv(*_sock, buffer, len, flags);
}

size_t connected_socket::send(const void* buffer, size_t len, Socket::socket_flags flags)
{
    return Socket::send(*_sock, buffer, len, flags);
}

///////////////////////////////////////////////////////////////////////////////
// unconnected_socket class
///////////////////////////////////////////////////////////////////////////////

unconnected_socket::unconnected_socket() :basic_socket()
{}

unconnected_socket::unconnected_socket(unconnected_socket const &other) : basic_socket(other)
{}

unconnected_socket::unconnected_socket(unconnected_socket &&other) : basic_socket(std::move(other))
{}

unconnected_socket::unconnected_socket(Socket::socket_t s) : basic_socket(s)
{}

unconnected_socket::~unconnected_socket()
{}

size_t unconnected_socket::recvfrom(basic_addr & addr, void* buffer, size_t len, Socket::socket_flags flags)
{
    return Socket::recvfrom(*_sock, addr, buffer, len, flags);
}

size_t unconnected_socket::sendto(basic_addr & addr, const void* buffer, size_t len, Socket::socket_flags flags)
{
    return Socket::sendto(*_sock, addr, buffer, len, flags);
}
