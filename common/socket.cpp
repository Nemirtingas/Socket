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

#include <Socket/common/socket.h>

using namespace std;
using namespace PortableAPI;

socket_exception::socket_exception() :mywhat("Socket exception") {}
socket_exception::socket_exception(const char* mywhat) :mywhat(mywhat) {}
socket_exception::socket_exception(std::string const& mywhat) :mywhat(mywhat) {}
const char* socket_exception::what() const noexcept { return mywhat.c_str(); }

basic_addr::~basic_addr()
{}

Socket::socket_t Socket::accept(Socket::socket_t s, basic_addr &addr)
{
    Socket::socket_t res;
    socklen_t len = addr.len();
    if ((res = ::accept(s, &addr.addr(), &len)) == Socket::invalid_socket)
    {
#if defined(__WINDOWS__)
        int32_t error = WSAGetLastError();
#elif defined(__LINUX__) || defined(__APPLE__)
        int32_t error = errno;
#endif
        switch (error)
        {
#if defined(__WINDOWS__)
            case WSANOTINITIALISED: throw wsa_not_initialised();
            case WSAECONNRESET    : throw connection_reset();
            case WSAEINVAL        : throw error_in_value("The listen function was not invoked prior to accept.");
            case WSAENETDOWN      : throw wsa_net_down();
            case WSAEWOULDBLOCK   : throw would_block();
#elif defined(__LINUX__)
            case ECONNABORTED : throw connection_reset();
            case EINVAL       : throw error_in_value("The listen function was not invoked prior to accept.");
#if EAGAIN != EWOULDBLOCK
            case EAGAIN: 
#endif
            case EWOULDBLOCK : throw would_block();
#endif
            default               : throw socket_exception("accept exception: " + to_string(error));
        }
    }

    return res;
}

void Socket::bind(Socket::socket_t s, basic_addr &addr)
{
    if (::bind(s, &addr.addr(), addr.len()) == -1)
    {
#if defined(__WINDOWS__)
        int32_t error = WSAGetLastError();
#elif defined(__LINUX__) || defined(__APPLE__)
        int32_t error = errno;
#endif
        switch (error)
        {
#if defined(__WINDOWS__)
            case WSANOTINITIALISED: throw wsa_not_initialised();
            case WSAENETDOWN: throw wsa_net_down();
            case WSAEADDRINUSE: throw address_in_use("The given address is already in use.");
            case WSAEINVAL: throw error_in_value("The socket is already bound to an address.");
#elif defined(__LINUX__)
            case EADDRINUSE: throw address_in_use("The given address is already in use.");
            case EINVAL: throw error_in_value("The socket is already bound to an address.");
#endif
            default: throw socket_exception("bind exception: " + to_string(error));
        }
    }
}

void Socket::closeSocket(Socket::socket_t s)
{
#if defined(__WINDOWS__)
    ::closesocket(s);
#elif defined(__LINUX__) || defined(__APPLE__)
    ::close(s);
#endif
}

void Socket::connect(Socket::socket_t s, basic_addr &addr)
{
    if (::connect(s, &addr.addr(), addr.len()) == -1)
    {
#if defined(__WINDOWS__)
        int32_t error = WSAGetLastError();
#elif defined(__LINUX__) || defined(__APPLE__)
        int32_t error = errno;
#endif
        switch (error)
        {
#if defined(__WINDOWS__)
            case WSANOTINITIALISED: throw wsa_not_initialised();
            case WSAENETDOWN: throw wsa_net_down();
            case WSAEADDRINUSE: throw address_in_use("Local address is already in use.");
            case WSAEADDRNOTAVAIL: throw error_in_value("Address is not available.");
            case WSAECONNREFUSED: throw connection_refused();
            case WSAENETUNREACH: case WSAEHOSTUNREACH: throw network_unreachable();
            case WSAETIMEDOUT: throw connection_timeout();
            case WSAEISCONN: throw is_connected();
#elif defined(__LINUX__)
            case EINVAL: throw error_in_value("Address is not available.");
            case EADDRINUSE: throw address_in_use("Local address is already in use.");
            case ECONNREFUSED:  throw connection_refused();
            case ENETUNREACH: throw network_unreachable();
            case EISCONN: throw is_connected();
#endif
            default: throw socket_exception("connect exception: " + to_string(error));
        }
    }
}

int Socket::ioctlsocket(Socket::socket_t s, long cmd, unsigned long * arg)
{
#if defined(__WINDOWS__)
    return ::ioctlsocket(s, cmd, arg);
#elif defined(__LINUX__) || defined(__APPLE__)
    return ::ioctl(s, cmd, arg);
#endif
}

int Socket::setsockopt(Socket::socket_t s, int level, int optname, const void* optval, socklen_t optlen)
{
#if defined(__WINDOWS__)
    return ::setsockopt(s, level, optname, reinterpret_cast<const char*>(optval), optlen);
#elif defined(__LINUX__) || defined(__APPLE__)
    return ::setsockopt(s, level, optname, optval, static_cast<socklen_t>(optlen));
#endif
}

int Socket::getsockopt(Socket::socket_t s, int level, int optname, void* optval, socklen_t* optlen)
{
#if defined(__WINDOWS__)
    return ::getsockopt(s, level, optname, reinterpret_cast<char*>(optval), optlen);
#elif defined(__LINUX__) || defined(__APPLE__)
    return ::getsockopt(s, level, optname, optval, optlen);
#endif
}

void Socket::listen(Socket::socket_t s, int waiting_connection)
{
    if (::listen(s, waiting_connection) == -1)
    {
#if defined(__WINDOWS__)
        int32_t error = WSAGetLastError();
#elif defined(__LINUX__) || defined(__APPLE__)
        int32_t error = errno;
#endif
        switch (error)
        {
#if defined(__WINDOWS__)
            case WSANOTINITIALISED: throw wsa_not_initialised();
            case WSAENETDOWN: throw wsa_net_down();
            case WSAEADDRINUSE: throw address_in_use();
#elif defined(__LINUX__)
            case EADDRINUSE: throw address_in_use();
#endif
            default: throw socket_exception("listen exception: " + to_string(error));
        }
    }
}

size_t Socket::recv(Socket::socket_t s, void* buffer, size_t len, Socket::socket_flags flags)
{
    int res = ::recv(s, reinterpret_cast<char*>(buffer), len, static_cast<int32_t>(flags));
    if (res == 0)
        throw connection_reset();
    else if( res == -1 )
    {
#if defined(__WINDOWS__)
        int32_t error = WSAGetLastError();
#elif defined(__LINUX__) || defined(__APPLE__)
        int32_t error = errno;
#endif
        switch (error)
        {
#if defined(__WINDOWS__)
            case WSANOTINITIALISED: throw wsa_not_initialised();
            case WSAENETDOWN: throw wsa_net_down();
            case WSAENOTCONN: throw not_connected();
#elif defined(__LINUX__)
            case ENOTCONN: throw not_connected("The socket is not connected.");
            case ECONNRESET: throw connection_reset("Connection reset by peer.");
#endif
            default: throw socket_exception("recv exception: " + to_string(error));
        }
    }
    return static_cast<size_t>(res);
}

size_t Socket::recvfrom(Socket::socket_t s, basic_addr &addr, void* buffer, size_t len, Socket::socket_flags flags)
{
    socklen_t slen = addr.len();
    int res = ::recvfrom(s, reinterpret_cast<char*>(buffer), len, static_cast<int32_t>(flags), &addr.addr(), &slen);
    if (res == -1)
    {
#if defined(__WINDOWS__)
        int32_t error = WSAGetLastError();
#elif defined(__LINUX__) || defined(__APPLE__)
        int32_t error = errno;
#endif
        switch (error)
        {
#if defined(__WINDOWS__)
            case WSANOTINITIALISED: throw wsa_not_initialised();
            case WSAENETDOWN: throw wsa_net_down();
            case WSAEINVAL: throw error_in_value("The socket is not bound to an address.");
            case WSAEWOULDBLOCK: res = 0; break;
            case WSAECONNRESET: throw connection_reset();
#elif defined(__LINUX__)
            case EINVAL: throw error_in_value("The socket is not bound to an address.");
    #if EAGAIN != EWOULDBLOCK
            case EAGAIN:
    #endif
            case EWOULDBLOCK: res = 0; break;
#endif
            default: throw socket_exception("recvfrom exception: " + to_string(error));
        }
    }
    return static_cast<size_t>(res);
}

size_t Socket::send(Socket::socket_t s, const void* buffer, size_t len, Socket::socket_flags flags)
{
    int res = ::send(s, reinterpret_cast<char const*>(buffer), len, static_cast<int32_t>(flags));

    if (res == -1)
    {
#if defined(__WINDOWS__)
        int32_t error = WSAGetLastError();
#elif defined(__LINUX__) || defined(__APPLE__)
        int32_t error = errno;
#endif
        switch (error)
        {
#if defined(__WINDOWS__)
            case WSANOTINITIALISED: throw wsa_not_initialised();
            case WSAENETDOWN: throw wsa_net_down();
            case WSAENOTCONN: throw not_connected("The socket is not connected.");
            case WSAECONNABORTED: throw connection_reset("Connection reset by peer.");
#elif defined(__LINUX__)
            case ENOTCONN: throw not_connected("The socket is not connected.");
#endif
            default: throw socket_exception("send exception: " + to_string(error));
        }
    }
    return res;
}

size_t Socket::sendto(Socket::socket_t s, basic_addr &addr, const void* buffer, size_t len, Socket::socket_flags flags)
{
    int res = ::sendto(s, reinterpret_cast<const char*>(buffer), len, static_cast<int32_t>(flags), &addr.addr(), addr.len());
    if (res == -1)
    {
#if defined(__WINDOWS__)
        int32_t error = WSAGetLastError();
#elif defined(__LINUX__) || defined(__APPLE__)
        int32_t error = errno;
#endif
        switch (error)
        {
#if defined(__WINDOWS__)
            case WSANOTINITIALISED: throw wsa_not_initialised();
            case WSAENETDOWN: throw wsa_net_down();
            case WSAENETUNREACH: throw network_unreachable();
#elif defined(__LINUX__)
            case ENETUNREACH: throw network_unreachable();
#endif
            default: throw socket_exception("sendto exception: " + to_string(error));
        }
    }
    return res;
}

int Socket::shutdown(Socket::socket_t s, Socket::shutdown_flags how)
{
    return ::shutdown(s, static_cast<int32_t>(how));
}

Socket::socket_t Socket::socket(Socket::address_family af, Socket::types type, Socket::protocols proto)
{
    Socket::socket_t s = ::socket(static_cast<int>(af), static_cast<int>(type), static_cast<int>(proto));
    if (s == Socket::invalid_socket)
    {
#if defined(__WINDOWS__)
        int32_t error = WSAGetLastError();
#elif defined(__LINUX__) || defined(__APPLE__)
        int32_t error = errno;
#endif
        switch (error)
        {
#if defined(__WINDOWS__)
            case WSANOTINITIALISED: throw wsa_not_initialised();
            case WSAENETDOWN: throw wsa_net_down();
#elif defined(__LINUX__)
#endif
            default: throw socket_exception("listen exception: " + to_string(error));
        }
    }
    return s;
}

int Socket::getaddrinfo(const char * _Nodename, const char * _Servicename, const addrinfo * _Hints, addrinfo ** _Result)
{
    return ::getaddrinfo(_Nodename, _Servicename, _Hints, _Result);
}

int Socket::getnameinfo(const sockaddr * _Addr, socklen_t _Addrlen, char * _Host, size_t _Hostlen, char * _Serv, size_t _Servlen, int _Flags)
{
    return ::getnameinfo(_Addr, _Addrlen, _Host, _Hostlen, _Serv, _Servlen, _Flags);
}

uint32_t Socket::inet_addr(std::string const& _Naddr)
{
    in_addr addr;
    ::inet_pton(static_cast<int>(Socket::address_family::inet), _Naddr.c_str(), &addr);
#if defined(__WINDOWS__)
    return addr.S_un.S_addr;
#elif defined(__LINUX__) || defined(__APPLE__)
    return addr.s_addr;
#endif
}

std::string Socket::inet_ntoa(in_addr & _In)
{
    char tmp[16];
    const char *str = ::inet_ntop(static_cast<int>(Socket::address_family::inet), &_In, tmp, 16);
    return std::string(str);
}

int Socket::select(int _Nfds, fd_set * _Readfd, fd_set * _Writefd, fd_set * _Exceptfd, timeval * _Timeout)
{
    return ::select(_Nfds, _Readfd, _Writefd, _Exceptfd, _Timeout);
}

#if(_WIN32_WINNT >= 0x0600)
int Socket::poll(pollfd *_Fds, unsigned long _Nfds, int _Timeout)
{
    return WSAPoll(_Fds, _Nfds, _Timeout);
}
#elif defined(__LINUX__) || defined(__APPLE__)
int Socket::poll(pollfd *_Fds, unsigned long _Nfds, int _Timeout)
{
    return ::poll(_Fds, _Nfds, _Timeout);
}
#endif

void Socket::InitSocket(uint32_t version)
{
    static Socket s(version);
}

void Socket::SetLastError(int error)
{
#if defined(__WINDOWS__)
    WSASetLastError(error);
#endif
}

/*
void Socket::FreeSocket()
{
#if defined(__WINDOWS__)
    WSACleanup();
#endif
}
*/

WSAData const& Socket::GetWSAData()
{
    static WSAData datas;
    return datas;
}

Socket::Socket(uint32_t version)
{
#if defined(__WINDOWS__)
    int err = WSAStartup(version, &const_cast<WSAData&>(Socket::GetWSAData()));
    switch (err)
    {
        case WSASYSNOTREADY    : throw(wsa_sys_not_ready());
        case WSAVERNOTSUPPORTED: throw(wsa_version_not_supported());
        case WSAEINPROGRESS    : throw(wsa_in_progress());
        case WSAEPROCLIM       : throw(wsa_proclim());
        case WSAEFAULT         : throw(wsa_fault());
        case 0: break;
        default: throw(socket_exception("Socket initialisation error: " + std::to_string(err)));
    }
#else
    (void)version;
#endif
}

Socket::~Socket() 
{
#if defined(__WINDOWS__)
    WSACleanup();
#endif
}
