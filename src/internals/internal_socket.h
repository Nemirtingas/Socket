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

#pragma once

#include <System/SystemExports.h>
#include <System/Endianness.hpp>

#include <NetworkLibrary/details/Socket.h>

#if defined(SYSTEM_OS_WINDOWS)
    #define VC_EXTRALEAN
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <WinSock2.h>
    #include <Ws2tcpip.h>
    #include <iphlpapi.h> // (iphlpapi.lib) Infos about ethernet interfaces

#elif defined(SYSTEM_OS_LINUX)
    #include <unistd.h>
    #include <signal.h>
    #include <netdb.h>

    #include <arpa/inet.h>

    #include <sys/types.h>
    #include <sys/wait.h>
    #include <sys/socket.h>
    #include <sys/un.h>
    #include <sys/ioctl.h>
    #include <sys/poll.h>

    #include <netinet/in.h>
    #include <net/if.h>

    #include <ifaddrs.h>// getifaddrs
#elif defined(SYSTEM_OS_APPLE)
    #include <unistd.h>
    #include <netdb.h>
    #include <errno.h>

    #include <arpa/inet.h>

    #include <sys/types.h>
    #include <sys/wait.h>
    #include <sys/socket.h>
    #include <sys/un.h>
    #include <sys/ioctl.h>
    #include <sys/poll.h>
    #include <sys/select.h>
    #include <sys/filio.h>

    #include <netinet/in.h>
    #include <net/if.h>

    #include <ifaddrs.h>// getifaddrs
#endif

#define SOCKET_HIDE_SYMBOLS(return_type) SYSTEM_HIDE_API(return_type, SYSTEM_CALL_DEFAULT)

#include <cassert>
#include <limits>

namespace NetworkLibrary {
namespace Internals {
    enum class AddressFamily : int
    {
        unknown = 0xffff,
    };

    enum class SocketTypes : int
    {
        unknown = 0xffff,
    };

    enum class SocketProtocols : int
    {
        unknown = 0xffff,
    };

    enum class CmdName : uint32_t
    {
        fionread = FIONREAD,
        fionbio = FIONBIO,
        fioasync = FIOASYNC,
        unknown = 0xffff,
    };

    enum class ShutdownFlags
    {
#if defined(SYSTEM_OS_WINDOWS)
        receive = SD_RECEIVE,
        send = SD_SEND,
        both = SD_BOTH,
#elif defined(SYSTEM_OS_LINUX) || defined(SYSTEM_OS_APPLE)
        receive = SHUT_RD,
        send = SHUT_WR,
        both = SHUT_RDWR,
#endif
    };

#if defined(SYSTEM_OS_WINDOWS)
    static std::string WCharToString(PWCHAR wstr)
    {
        int length = (int)wcslen(wstr);
        if (length == 0)
            return std::string();

        int utf8_size = WideCharToMultiByte(CP_UTF8, 0, wstr, length, nullptr, 0, nullptr, nullptr);
        std::string str(utf8_size, '\0');
        WideCharToMultiByte(CP_UTF8, 0, wstr, length, &str[0], utf8_size, nullptr, nullptr);
        return str;
    }

    static std::wstring StringToWChar(std::string const& str)
    {
        if (str.empty())
            return std::wstring();

        int utf16_size = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), nullptr, 0);
        std::wstring wstr(utf16_size, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstr[0], utf16_size);
        return wstr;
    }
#endif

#if defined(SYSTEM_OS_WINDOWS)
    SYSTEM_HIDE_CLASS(class) WinSockInitializer
    {
        WSAData _Datas;
        
        WinSockInitializer();        
    public:
        ~WinSockInitializer();

        static WinSockInitializer& Inst() noexcept;
        static ::NetworkLibrary::Error Initialize(uint32_t version = 0x0202);
    };
#endif

    SYSTEM_HIDE_CLASS(class) NativeSocket
    {
    public:
#if defined(SYSTEM_OS_WINDOWS)
        using socket_t = SOCKET;
#elif defined(SYSTEM_OS_LINUX) || defined(SYSTEM_OS_APPLE)
        using socket_t = int32_t;
#endif
        static constexpr socket_t invalid_socket = ((socket_t)(-1));

        socket_t Socket;

        constexpr bool IsValid() { return Socket != invalid_socket; }

        NativeSocket();
        NativeSocket(NativeSocket const&) = delete;
        NativeSocket(NativeSocket&& other);
        NativeSocket& operator=(NativeSocket const& other) = delete;
        NativeSocket& operator=(NativeSocket&& other);
        ~NativeSocket();

        NetworkLibrary::Error CreateSocket(Internals::AddressFamily af, Internals::SocketTypes type, Internals::SocketProtocols proto);
        NetworkLibrary::Error SetSockOption(int32_t option_name, const void* value, socklen_t optlen);
        NetworkLibrary::Error SetNonBlocking(bool non_blocking);
        int32_t GetWaitingSize() const;
        void Close();
    };

    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) MakeUnknownError(int native_error);
    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) MakeErrorFromSocketCode(int socket_error);
    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) MakeErrorFromNative(int native_error);
    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) LastError();

    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) accept(Internals::NativeSocket const& s, NetworkLibrary::BasicAddr& addr, Internals::NativeSocket& out);
    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) bind(Internals::NativeSocket const& s, NetworkLibrary::BasicAddr const& addr);
    SOCKET_HIDE_SYMBOLS(void) closeSocket(Internals::NativeSocket& s);
    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) connect(Internals::NativeSocket const& s, NetworkLibrary::BasicAddr const& addr);
    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) ioctlsocket(Internals::NativeSocket const& s, Internals::CmdName cmd, unsigned long* arg);
    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) setsockopt(Internals::NativeSocket const& s, int optname, const void* optval, socklen_t optlen);
    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) getsockopt(Internals::NativeSocket const& s, int optname, void* optval, socklen_t* optlen);
    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) getsockname(Internals::NativeSocket const& s, NetworkLibrary::BasicAddr& addr);
    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) listen(Internals::NativeSocket const& s, int waiting_connection = 5);
    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) recv(Internals::NativeSocket const& s, void* buffer, size_t& len, int32_t flags);
    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) send(Internals::NativeSocket const& s, const void* buffer, size_t& len, int32_t flags);
    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) recvfrom(Internals::NativeSocket const& s, NetworkLibrary::BasicAddr& addr, void* buffer, size_t& len, int32_t flags);
    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) sendto(Internals::NativeSocket const& s, NetworkLibrary::BasicAddr const& addr, const void* buffer, size_t& len, int32_t flags);
    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) shutdown(Internals::NativeSocket const& s, Internals::ShutdownFlags how);
    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) socket(Internals::AddressFamily af, Internals::SocketTypes type, Internals::SocketProtocols proto, Internals::NativeSocket& s);
    SOCKET_HIDE_SYMBOLS(int) getaddrinfo(const char* node, const char* service, const addrinfo* hints, addrinfo** res);
    SOCKET_HIDE_SYMBOLS(void) freeaddrinfo(addrinfo* infos);
    SOCKET_HIDE_SYMBOLS(int) getnameinfo(const sockaddr* addr, socklen_t addrlen, char* host, size_t hostlen, char* serv, size_t servlen, int flags);
    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) inet_pton(std::string const& str_addr, sockaddr* sockaddr);
    SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) inet_ntop(const sockaddr* sockaddr, std::string& str_addr);
    SOCKET_HIDE_SYMBOLS(int) select(int nfds, fd_set* readfd, fd_set* writefd, fd_set* exceptfd, timeval* timeout);
    SOCKET_HIDE_SYMBOLS(int) poll(pollfd* fds, size_t nfds, int timeout);
}
}