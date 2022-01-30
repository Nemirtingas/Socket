/* Copyright (C) Nemirtingas
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

#include "internal_socket.h"

namespace NetworkLibrary {
namespace Internals {

#if defined(SOCKET_OS_WINDOWS)
    WinSockInitializer& WinSockInitializer::Inst() noexcept
    {
        static WinSockInitializer inst;
        return inst;
    }

    WinSockInitializer::WinSockInitializer() :_Datas{} {}
    WinSockInitializer::~WinSockInitializer() { WSACleanup(); }
    ::NetworkLibrary::Error WinSockInitializer::Initialize(uint32_t version) { return MakeErrorFromNative(WSAStartup(version, &Inst()._Datas)); }
#endif

    // NativeSocket

    NativeSocket::NativeSocket() :
        Socket(invalid_socket)
    {}

    NativeSocket::NativeSocket(NativeSocket&& other) :
        Socket(other.Socket)
    {
        other.Socket = invalid_socket;
    }

    NativeSocket& NativeSocket::operator=(NativeSocket&& other)
    {
        socket_t tmp = other.Socket;
        other.Socket = invalid_socket;
        Socket = tmp;

        return *this;
    }

    NativeSocket::~NativeSocket()
    {
        Close();
    }

    NetworkLibrary::Error NativeSocket::CreateSocket(Internals::AddressFamily af, Internals::SocketTypes type, Internals::SocketProtocols proto)
    {
        Close();
        return socket(af, type, proto, *this);
    }

    NetworkLibrary::Error NativeSocket::SetSockOption(int32_t option_name, const void* value, socklen_t optlen)
    {
        return Internals::setsockopt(*this, option_name, value, optlen);
    }

    NetworkLibrary::Error NativeSocket::SetNonBlocking(bool non_blocking)
    {
        unsigned long arg = non_blocking ? 1 : 0;
        return Internals::ioctlsocket(*this, CmdName::fionbio, &arg);
    }

    int32_t NativeSocket::GetWaitingSize() const
    {
        unsigned long arg = 0;
        Internals::ioctlsocket(*this, CmdName::fionread, &arg);
        return arg;
    }

    void NativeSocket::Close()
    {
        if (IsValid())
        {
            Internals::closeSocket(*this);
            Socket = NativeSocket::invalid_socket;
        }
    }

    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) MakeUnknownError(int native_error)
    {
        ::NetworkLibrary::Error error;
        error.NativeCode = native_error;
        error.ErrorCode = NetworkLibrary::Error::UnknownError;
        return error;
    }

    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) MakeErrorFromSocketCode(int socket_error)
    {
        ::NetworkLibrary::Error error;
        error.ErrorCode = socket_error;

        switch (socket_error)
        {
            default                                             : error.NativeCode = -1                ; break;
            case ::NetworkLibrary::Error::NoError               : error.NativeCode =  0                ; break;

#if defined(SOCKET_OS_WINDOWS)
            case ::NetworkLibrary::Error::OutOfMemory           : error.NativeCode = ERROR_OUTOFMEMORY ; break;
            case ::NetworkLibrary::Error::Access                : error.NativeCode = WSAEACCES         ; break;
            case ::NetworkLibrary::Error::AddrInUse             : error.NativeCode = WSAEADDRINUSE     ; break;
            case ::NetworkLibrary::Error::AddrNotAvailable      : error.NativeCode = WSAEADDRNOTAVAIL  ; break;
            case ::NetworkLibrary::Error::ConnectionRefused     : error.NativeCode = WSAECONNREFUSED   ; break;
            case ::NetworkLibrary::Error::ConnectionAborted     : error.NativeCode = WSAECONNABORTED   ; break;
            case ::NetworkLibrary::Error::ConnectionReset       : error.NativeCode = WSAECONNRESET     ; break;
            case ::NetworkLibrary::Error::Fault                 : error.NativeCode = WSAEFAULT         ; break;
            case ::NetworkLibrary::Error::IsConnected           : error.NativeCode = WSAEISCONN        ; break;
            case ::NetworkLibrary::Error::InProgress            : error.NativeCode = WSAEINPROGRESS    ; break;
            case ::NetworkLibrary::Error::InVal                 : error.NativeCode = WSAEINVAL         ; break;
            case ::NetworkLibrary::Error::NotConnected          : error.NativeCode = WSAENOTCONN       ; break;
            case ::NetworkLibrary::Error::NetworkUnreachable    : error.NativeCode = WSAENETUNREACH    ; break;
            case ::NetworkLibrary::Error::WouldBlock            : error.NativeCode = WSAEWOULDBLOCK    ; break;
            case ::NetworkLibrary::Error::AfNotSupported        : error.NativeCode = WSAEAFNOSUPPORT   ; break;
            case ::NetworkLibrary::Error::SockTypeNotSupported  : error.NativeCode = WSAESOCKTNOSUPPORT; break;
            case ::NetworkLibrary::Error::MessageSize           : error.NativeCode = WSAEMSGSIZE       ; break;
            case ::NetworkLibrary::Error::NotFound              : error.NativeCode = WSAENOMORE        ; break;
            case ::NetworkLibrary::Error::TimedOut              : error.NativeCode = WSAETIMEDOUT      ; break;
            case ::NetworkLibrary::Error::HostDown              : error.NativeCode = WSAEHOSTDOWN      ; break;
            case ::NetworkLibrary::Error::HostUnreachable       : error.NativeCode = WSAEHOSTUNREACH   ; break;

            case ::NetworkLibrary::Error::WsaNotInitialised     : error.NativeCode = WSANOTINITIALISED ; break;
            case ::NetworkLibrary::Error::WsaNetDown            : error.NativeCode = WSAENETDOWN       ; break;
            case ::NetworkLibrary::Error::WsaSystemNotReady     : error.NativeCode = WSASYSNOTREADY    ; break;
            case ::NetworkLibrary::Error::WsaVersionNotSupported: error.NativeCode = WSAVERNOTSUPPORTED; break;
            case ::NetworkLibrary::Error::WsaProClim            : error.NativeCode = WSAEPROCLIM       ; break;
#elif defined(SOCKET_OS_LINUX) || defined(SOCKET_OS_APPLE)
            case ::NetworkLibrary::Error::OutOfMemory           : error.NativeCode = ENOMEM         ; break;
            case ::NetworkLibrary::Error::Access                : error.NativeCode = EACCES         ; break;
            case ::NetworkLibrary::Error::AddrInUse             : error.NativeCode = EADDRINUSE     ; break;
            case ::NetworkLibrary::Error::ConnectionRefused     : error.NativeCode = ECONNREFUSED   ; break;
            case ::NetworkLibrary::Error::ConnectionAborted     : error.NativeCode = ECONNABORTED   ; break;
            case ::NetworkLibrary::Error::ConnectionReset       : error.NativeCode = ECONNRESET     ; break;
            case ::NetworkLibrary::Error::Fault                 : error.NativeCode = EFAULT         ; break;
            case ::NetworkLibrary::Error::IsConnected           : error.NativeCode = EISCONN        ; break;
            case ::NetworkLibrary::Error::InProgress            : error.NativeCode = EINPROGRESS    ; break;
            case ::NetworkLibrary::Error::InVal                 : error.NativeCode = EINVAL         ; break;
            case ::NetworkLibrary::Error::NotConnected          : error.NativeCode = ENOTCONN       ; break;
            case ::NetworkLibrary::Error::NetworkUnreachable    : error.NativeCode = ENETUNREACH    ; break;
            case ::NetworkLibrary::Error::WouldBlock            : error.NativeCode = EWOULDBLOCK    ; break;
            case ::NetworkLibrary::Error::AfNotSupported        : error.NativeCode = EAFNOSUPPORT   ; break;
            case ::NetworkLibrary::Error::SockTypeNotSupported  : error.NativeCode = ESOCKTNOSUPPORT; break;
            case ::NetworkLibrary::Error::MessageSize           : error.NativeCode = EMSGSIZE       ; break;
            case ::NetworkLibrary::Error::NotFound              : error.NativeCode = ENOENT         ; break;
            case ::NetworkLibrary::Error::TimedOut              : error.NativeCode = ETIMEDOUT      ; break;
            case ::NetworkLibrary::Error::HostDown              : error.NativeCode = EHOSTDOWN      ; break;
            case ::NetworkLibrary::Error::HostUnreachable       : error.NativeCode = EHOSTUNREACH   ; break;
#endif
        }

        return error;
    }

    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) MakeErrorFromNative(int native_error)
    {
        ::NetworkLibrary::Error error;
        error.NativeCode = native_error;
        switch (native_error)
        {
#if defined(SOCKET_OS_WINDOWS)
            case WSAEACCES         : error.ErrorCode = ::NetworkLibrary::Error::Access               ; break;
            case WSAEADDRINUSE     : error.ErrorCode = ::NetworkLibrary::Error::AddrInUse            ; break;
            case WSAEADDRNOTAVAIL  : error.ErrorCode = ::NetworkLibrary::Error::AddrNotAvailable     ; break;
            case WSAECONNREFUSED   : error.ErrorCode = ::NetworkLibrary::Error::ConnectionRefused    ; break;
            case WSAECONNABORTED   : error.ErrorCode = ::NetworkLibrary::Error::ConnectionAborted    ; break;
            case WSAECONNRESET     : error.ErrorCode = ::NetworkLibrary::Error::ConnectionReset      ; break;
            case WSAEFAULT         : error.ErrorCode = ::NetworkLibrary::Error::Fault                ; break;
            case WSAEISCONN        : error.ErrorCode = ::NetworkLibrary::Error::IsConnected          ; break;
            case WSAEINPROGRESS    : error.ErrorCode = ::NetworkLibrary::Error::InProgress           ; break;
            case WSAEINVAL         : error.ErrorCode = ::NetworkLibrary::Error::InVal                ; break;
            case WSAENOTCONN       : error.ErrorCode = ::NetworkLibrary::Error::NotConnected         ; break;
            case WSAENETUNREACH    : error.ErrorCode = ::NetworkLibrary::Error::NetworkUnreachable   ; break;
            case WSAEWOULDBLOCK    : error.ErrorCode = ::NetworkLibrary::Error::WouldBlock           ; break;
            case WSAEAFNOSUPPORT   : error.ErrorCode = ::NetworkLibrary::Error::AfNotSupported       ; break;
            case WSAESOCKTNOSUPPORT: error.ErrorCode = ::NetworkLibrary::Error::SockTypeNotSupported ; break;
            case WSAEMSGSIZE       : error.ErrorCode = ::NetworkLibrary::Error::MessageSize          ; break;
            case WSAENOMORE        : 
            case WSA_E_NO_MORE     : error.ErrorCode = ::NetworkLibrary::Error::NotFound             ; break;
            case WSAETIMEDOUT      : error.ErrorCode = ::NetworkLibrary::Error::TimedOut             ; break;
            case WSAEHOSTDOWN      : error.ErrorCode = ::NetworkLibrary::Error::HostDown             ; break;
            case WSAEHOSTUNREACH   : error.ErrorCode = ::NetworkLibrary::Error::HostUnreachable      ; break;

            case WSANOTINITIALISED : error.ErrorCode = ::NetworkLibrary::Error::WsaNotInitialised     ; break;
            case WSAENETDOWN       : error.ErrorCode = ::NetworkLibrary::Error::WsaNetDown            ; break;
            case WSASYSNOTREADY    : error.ErrorCode = ::NetworkLibrary::Error::WsaSystemNotReady     ; break;
            case WSAVERNOTSUPPORTED: error.ErrorCode = ::NetworkLibrary::Error::WsaVersionNotSupported; break;
            case WSAEPROCLIM       : error.ErrorCode = ::NetworkLibrary::Error::WsaProClim            ; break;
#elif defined(SOCKET_OS_LINUX) || defined(SOCKET_OS_APPLE)
            case ENOMEM         : error.ErrorCode = ::NetworkLibrary::Error::OutOfMemory         ; break;
            case EACCES         : error.ErrorCode = ::NetworkLibrary::Error::Access              ; break;
            case EADDRINUSE     : error.ErrorCode = ::NetworkLibrary::Error::AddrInUse           ; break;
            case ECONNREFUSED   : error.ErrorCode = ::NetworkLibrary::Error::ConnectionRefused   ; break;
            case ECONNABORTED   : error.ErrorCode = ::NetworkLibrary::Error::ConnectionAborted   ; break;
            case ECONNRESET     : error.ErrorCode = ::NetworkLibrary::Error::ConnectionReset     ; break;
            case EFAULT         : error.ErrorCode = ::NetworkLibrary::Error::Fault               ; break;
            case EISCONN        : error.ErrorCode = ::NetworkLibrary::Error::IsConnected         ; break;
            case EINPROGRESS    : error.ErrorCode = ::NetworkLibrary::Error::InProgress          ; break;
            case EINVAL         : error.ErrorCode = ::NetworkLibrary::Error::InVal               ; break;
            case ENOTCONN       : error.ErrorCode = ::NetworkLibrary::Error::NotConnected        ; break;
            case ENETUNREACH    : error.ErrorCode = ::NetworkLibrary::Error::NetworkUnreachable  ; break;
            case EWOULDBLOCK    : error.ErrorCode = ::NetworkLibrary::Error::WouldBlock          ; break;
            case EAFNOSUPPORT   : error.ErrorCode = ::NetworkLibrary::Error::AfNotSupported      ; break;
            case ESOCKTNOSUPPORT: error.ErrorCode = ::NetworkLibrary::Error::SockTypeNotSupported; break;
            case EMSGSIZE       : error.ErrorCode = ::NetworkLibrary::Error::MessageSize         ; break;
            case ENOENT         : error.ErrorCode = ::NetworkLibrary::Error::NotFound            ; break;
            case ETIMEDOUT      : error.ErrorCode = ::NetworkLibrary::Error::TimedOut            ; break;
            case EHOSTDOWN      : error.ErrorCode = ::NetworkLibrary::Error::HostDown            ; break;
            case EHOSTUNREACH   : error.ErrorCode = ::NetworkLibrary::Error::HostUnreachable     ; break;
#endif
            case 0              : error.ErrorCode = ::NetworkLibrary::Error::NoError; break;
            default             : error.ErrorCode = ::NetworkLibrary::Error::UnknownError;
        }

        return error;
    }

    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) LastError()
    {
#if defined(SOCKET_OS_WINDOWS)
        return MakeErrorFromNative(WSAGetLastError());
#elif defined(SOCKET_OS_LINUX) || defined(SOCKET_OS_APPLE)
        return MakeErrorFromNative(errno);
#endif
    }

    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) accept(Internals::NativeSocket const& s, NetworkLibrary::BasicAddr& addr, Internals::NativeSocket& out)
    {
        sockaddr* native_addr = (sockaddr*)addr.GetAddr();
        socklen_t addr_length = static_cast<socklen_t>(addr.GetLength());
        out.Socket = ::accept(s.Socket, native_addr, &addr_length);
        return out.IsValid() ? MakeErrorFromSocketCode(Error::NoError) : LastError();
    }

    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) bind(Internals::NativeSocket const& s, NetworkLibrary::BasicAddr const& addr)
    {
        sockaddr const* native_addr = (sockaddr const*)addr.GetAddr();
        socklen_t addr_length = static_cast<socklen_t>(addr.GetLength());
        return ::bind(s.Socket, native_addr, addr_length) != -1 ? MakeErrorFromSocketCode(Error::NoError) : LastError();
    }

    SOCKET_HIDE_SYMBOLS(void) closeSocket(Internals::NativeSocket& s)
    {
#if defined(SOCKET_OS_WINDOWS)
        ::closesocket(s.Socket);
#elif defined(SOCKET_OS_LINUX) || defined(SOCKET_OS_APPLE)
        ::close(s.Socket);
#endif
    }

    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) connect(Internals::NativeSocket const& s, NetworkLibrary::BasicAddr const& addr)
    {
        sockaddr const* native_addr = (sockaddr const*)addr.GetAddr();
        socklen_t addr_length = static_cast<socklen_t>(addr.GetLength());
        return ::connect(s.Socket, native_addr, addr_length) != -1 ? MakeErrorFromSocketCode(::NetworkLibrary::Error::NoError) : LastError();
    }

    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) ioctlsocket(Internals::NativeSocket const& s, Internals::CmdName cmd, unsigned long* arg)
    {
        ::NetworkLibrary::Error error;

#if defined(SOCKET_OS_WINDOWS)
        error.NativeCode = ::ioctlsocket(s.Socket, static_cast<long>(cmd), arg);
#elif defined(SOCKET_OS_LINUX) || defined(SOCKET_OS_APPLE)
        error.NativeCode = ::ioctl(s.Socket, static_cast<long>(cmd), arg);
#endif

        error.ErrorCode = error.NativeCode == 0 ? ::NetworkLibrary::Error::NoError : ::NetworkLibrary::Error::UnknownError;
        return error;
    }

    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) setsockopt(Internals::NativeSocket const& s, int optname, const void* optval, socklen_t optlen)
    {
        ::NetworkLibrary::Error error;

#if defined(SOCKET_OS_WINDOWS)
        error.NativeCode = ::setsockopt(s.Socket, SOL_SOCKET, static_cast<int>(optname), reinterpret_cast<const char*>(optval), optlen);
#elif defined(SOCKET_OS_LINUX) || defined(SOCKET_OS_APPLE)
        error.NativeCode = ::setsockopt(s.Socket, SOL_SOCKET, static_cast<int>(optname), optval, static_cast<socklen_t>(optlen));
#endif
        error.ErrorCode = error.NativeCode == 0 ? ::NetworkLibrary::Error::NoError : ::NetworkLibrary::Error::UnknownError;
        return error;
    }

    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) getsockopt(Internals::NativeSocket const& s, int optname, void* optval, socklen_t* optlen)
    {
        ::NetworkLibrary::Error error;

#if defined(SOCKET_OS_WINDOWS)
        error.NativeCode = ::getsockopt(s.Socket, SOL_SOCKET, optname, reinterpret_cast<char*>(optval), optlen);
#elif defined(SOCKET_OS_LINUX) || defined(SOCKET_OS_APPLE)
        error.NativeCode = ::getsockopt(s.Socket, SOL_SOCKET, optname, optval, optlen);
#endif
        error.ErrorCode = error.NativeCode == 0 ? ::NetworkLibrary::Error::NoError : ::NetworkLibrary::Error::UnknownError;
        return error;
    }

    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) getsockname(Internals::NativeSocket const& s, NetworkLibrary::BasicAddr& addr)
    {
        socklen_t sock_len = addr.GetLength();
        return MakeErrorFromNative(::getsockname(s.Socket, reinterpret_cast<sockaddr*>(addr.GetAddr()), &sock_len));
    }

    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) listen(Internals::NativeSocket const& s, int waiting_connection)
    {
        return ::listen(s.Socket, waiting_connection) != -1 ? MakeErrorFromSocketCode(::NetworkLibrary::Error::NoError) : LastError();
    }

    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) recv(Internals::NativeSocket const& s, void* buffer, size_t& len, int32_t flags)
    {
        int result = ::recv(s.Socket, reinterpret_cast<char*>(buffer), len, flags);
        if (result == -1)
        {
            len = 0;
            return LastError();
        }

        len = result;
        return MakeErrorFromSocketCode(::NetworkLibrary::Error::NoError);
    }

    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) send(Internals::NativeSocket const& s, const void* buffer, size_t& len, int32_t flags)
    {
        int result = ::send(s.Socket, reinterpret_cast<char const*>(buffer), len, flags);

        if (result == -1)
        {
            len = 0;
            return LastError();
        }

        len = result;
        return MakeErrorFromSocketCode(::NetworkLibrary::Error::NoError);
    }

    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) recvfrom(Internals::NativeSocket const& s, NetworkLibrary::BasicAddr& addr, void* buffer, size_t& len, int32_t flags)
    {
        sockaddr* native_addr = (sockaddr*)addr.GetAddr();
        socklen_t sock_len = (socklen_t)addr.GetLength();
        int result = ::recvfrom(s.Socket, reinterpret_cast<char*>(buffer), len, flags, native_addr, &sock_len);

        if (result == -1)
        {
            len = 0;
            return LastError();
        }

        len = result;
        return MakeErrorFromSocketCode(::NetworkLibrary::Error::NoError);
    }

    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) sendto(Internals::NativeSocket const& s, NetworkLibrary::BasicAddr const& addr, const void* buffer, size_t& len, int32_t flags)
    {
        sockaddr const* native_addr = (sockaddr const*)addr.GetAddr();
        socklen_t sock_len = (socklen_t)addr.GetLength();
        int result = ::sendto(s.Socket, reinterpret_cast<const char*>(buffer), len, flags, native_addr, sock_len);

        if (result == -1)
        {
            len = 0;
            return LastError();
        }

        len = result;
        return MakeErrorFromSocketCode(::NetworkLibrary::Error::NoError);
    }

    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) shutdown(Internals::NativeSocket const& s, Internals::ShutdownFlags how)
    {
        return ::shutdown(s.Socket, static_cast<int32_t>(how)) == -1 ? MakeErrorFromSocketCode(::NetworkLibrary::Error::NoError) : LastError();
    }

    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) socket(Internals::AddressFamily af, Internals::SocketTypes type, Internals::SocketProtocols proto, Internals::NativeSocket& s)
    {
        s.Socket = ::socket(static_cast<int>(af), static_cast<int>(type), static_cast<int>(proto));
        ::NetworkLibrary::Error error = s.IsValid() ? MakeErrorFromSocketCode(::NetworkLibrary::Error::NoError) : LastError();
#if defined(SOCKET_OS_WINDOWS)
        if (error.NativeCode == WSANOTINITIALISED)
        {
            error = WinSockInitializer::Initialize();
            if (error.ErrorCode == ::NetworkLibrary::Error::NoError)
            {// Retry after WinSock initialization.
                s.Socket = ::socket(static_cast<int>(af), static_cast<int>(type), static_cast<int>(proto));
                error = s.IsValid() ? MakeErrorFromSocketCode(::NetworkLibrary::Error::NoError) : LastError();
            }
        }
#endif
        return error;
    }

    SOCKET_HIDE_SYMBOLS(int) getaddrinfo(const char* node, const char* service, const addrinfo* hints, addrinfo** res)
    {
        return ::getaddrinfo(node, service, hints, res);
    }

    SOCKET_HIDE_SYMBOLS(void) freeaddrinfo(addrinfo* infos)
    {
        ::freeaddrinfo(infos);
    }

    SOCKET_HIDE_SYMBOLS(int) getnameinfo(const sockaddr* addr, socklen_t addrlen, char* host, size_t hostlen, char* serv, size_t servlen, int flags)
    {
        return ::getnameinfo(addr, addrlen, host, hostlen, serv, servlen, flags);
    }

    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) inet_pton(std::string const& str_addr, sockaddr* sockaddr)
    {
        int af = static_cast<int>(sockaddr->sa_family);
        void* native_buffer = nullptr;

        if (af == AF_INET)
        {
            native_buffer = &reinterpret_cast<sockaddr_in*>(sockaddr)->sin_addr;
        }
        else if (af == AF_INET6)
        {
            native_buffer = &reinterpret_cast<sockaddr_in6*>(sockaddr)->sin6_addr;
        }
        else
        {
            return MakeErrorFromSocketCode(::NetworkLibrary::Error::AfNotSupported);
        }

        switch (::inet_pton(af, str_addr.c_str(), native_buffer))
        {
            case -1: return MakeErrorFromSocketCode(::NetworkLibrary::Error::AfNotSupported);
            case  0: return MakeErrorFromSocketCode(::NetworkLibrary::Error::InVal);
            case  1: return MakeErrorFromSocketCode(::NetworkLibrary::Error::NoError);
            default: return MakeErrorFromSocketCode(::NetworkLibrary::Error::UnknownError);
        }
    }

    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) inet_ntop(const sockaddr* sockaddr, std::string& str_addr)
    {
        int af = static_cast<int>(sockaddr->sa_family);
        const void* native_buffer = nullptr;
        // 255.255.255.255                         => 15 + 1 Worst IPv4 case
        // 0123:4567:89ab:cdef:0123:4567:89ab:cdef => 39 + 1 Worst IPv6 case
        char buff[40] = {};
        str_addr.clear();

        if (af == AF_INET)
        {
            native_buffer = &reinterpret_cast<const sockaddr_in*>(sockaddr)->sin_addr;
        }
        else if (af == AF_INET6)
        {
            native_buffer = &reinterpret_cast<const sockaddr_in6*>(sockaddr)->sin6_addr;
        }
        else
        {
            return MakeErrorFromSocketCode(::NetworkLibrary::Error::AfNotSupported);
        }

        if (::inet_ntop(af, native_buffer, buff, sizeof(buff) / sizeof(*buff)) == nullptr)
        {
            return MakeErrorFromSocketCode(::NetworkLibrary::Error::InVal);
        }
        str_addr = buff;

        return MakeErrorFromSocketCode(::NetworkLibrary::Error::NoError);
    }

    SOCKET_HIDE_SYMBOLS(int) select(int nfds, fd_set* readfd, fd_set* writefd, fd_set* exceptfd, timeval* timeout)
    {
        return ::select(nfds, readfd, writefd, exceptfd, timeout);
    }

    SOCKET_HIDE_SYMBOLS(int) poll(pollfd* fds, size_t nfds, int timeout)
    {
#if defined(SOCKET_OS_WINDOWS)
        return ::WSAPoll(fds, nfds, timeout);
#else
        return ::poll(fds, nfds, timeout);
#endif
    }
}
}