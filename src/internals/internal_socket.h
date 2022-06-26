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

#include "internal_os_stuff.h"
#include <NetworkLibrary/details/Socket.h>

#if defined(SOCKET_OS_WINDOWS)
    #define VC_EXTRALEAN
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <WinSock2.h>
    #include <Ws2tcpip.h>
    #include <iphlpapi.h> // (iphlpapi.lib) Infos about ethernet interfaces

#elif defined(SOCKET_OS_LINUX)
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
#elif defined(SOCKET_OS_APPLE)
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

#define SOCKET_HIDE_SYMBOLS(return_type) SOCKET_HIDE_API(return_type, SOCKET_CALL_DEFAULT)

#include <cassert>
#include <limits>

namespace NetworkLibrary {
namespace Internals {
    static int32_t SocketFlagsToNative(int32_t flags)
    {
        int32_t native = 0;

        if (flags & SocketFlags::oob)
            native |= MSG_OOB;

        if (flags & SocketFlags::peek)
            native |= MSG_PEEK;

        if (flags & SocketFlags::dontroute)
            native |= MSG_DONTROUTE;

        return native;
    }

    static int16_t PollFlagsToNative(int16_t flags)
    {
        int16_t native = 0;

        if(flags & PollFlags::in)
            native |= POLLIN;

        if (flags & PollFlags::pri)
            native |= POLLPRI;

        if (flags & PollFlags::out)
            native |= POLLOUT;

        if (flags & PollFlags::err)
            native |= POLLERR;

        if (flags & PollFlags::hup)
            native |= POLLHUP;

        if (flags & PollFlags::nval)
            native |= POLLNVAL;

        if (flags & PollFlags::rdnorm)
            native |= POLLRDNORM;

        if (flags & PollFlags::rdband)
            native |= POLLRDBAND;

        if (flags & PollFlags::wrnorm)
            native |= POLLWRNORM;

        if (flags & PollFlags::wrband)
            native |= POLLWRBAND;

        return native;
    }

    static int16_t NativeToPollFlags(int16_t native)
    {
        int16_t flags = 0;

        if (native & POLLIN)
            flags |= PollFlags::in;

        if (native & POLLPRI)
            flags |= PollFlags::pri;

        if (native & POLLOUT)
            flags |= PollFlags::out;

        if (native & POLLERR)
            flags |= PollFlags::err;

        if (native & POLLHUP)
            flags |= PollFlags::hup;

        if (native & POLLNVAL)
            flags |= PollFlags::nval;

        if (native & POLLRDNORM)
            flags |= PollFlags::rdnorm;

        if (native & POLLRDBAND)
            flags |= PollFlags::rdband;

        if (native & POLLWRNORM)
            flags |= PollFlags::wrnorm;

        if (native & POLLWRBAND)
            flags |= PollFlags::wrband;

        return flags;
    }

    static int32_t OptionNameToNative(int32_t option)
    {
        switch (option)
        {
            case OptionName::so_debug    : return SO_DEBUG;
            case OptionName::so_reuseaddr: return SO_REUSEADDR;
            case OptionName::so_keepalive: return SO_KEEPALIVE;
            case OptionName::so_dontroute: return SO_DONTROUTE;
            case OptionName::so_broadcast: return SO_BROADCAST;
            case OptionName::so_linger   : return SO_LINGER;
            case OptionName::so_oobinline: return SO_OOBINLINE;
            case OptionName::so_sndbuf   : return SO_SNDBUF;
            case OptionName::so_rcvbuf   : return SO_RCVBUF;
            case OptionName::so_sndlowat : return SO_SNDLOWAT;
            case OptionName::so_rcvlowat : return SO_RCVLOWAT;
            case OptionName::so_sndtimeo : return SO_SNDTIMEO;
            case OptionName::so_rcvtimeo : return SO_RCVTIMEO;
            case OptionName::so_error    : return SO_ERROR;
            case OptionName::so_type     : return SO_TYPE;
        }

        return 0;
    }

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
#if defined(SOCKET_OS_WINDOWS)
        receive = SD_RECEIVE,
        send = SD_SEND,
        both = SD_BOTH,
#elif defined(SOCKET_OS_LINUX) || defined(SOCKET_OS_APPLE)
        receive = SHUT_RD,
        send = SHUT_WR,
        both = SHUT_RDWR,
#endif
    };

#if defined(SOCKET_OS_WINDOWS)
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

#if defined(SOCKET_OS_WINDOWS)
    SOCKET_HIDE_CLASS(class) WinSockInitializer
    {
        WSAData _Datas;
        
        WinSockInitializer();        
    public:
        ~WinSockInitializer();

        static WinSockInitializer& Inst() noexcept;
        static ::NetworkLibrary::Error Initialize(uint32_t version = 0x0202);
    };
#endif

    SOCKET_HIDE_CLASS(class) NativeSocket
    {
    public:
#if defined(SOCKET_OS_WINDOWS)
        using socket_t = SOCKET;
#elif defined(SOCKET_OS_LINUX) || defined(SOCKET_OS_APPLE)
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

    namespace Endian
    {
        static constexpr uint32_t _EndianMagic = 0x01020304;

        template<typename T, size_t byte_count>
        SOCKET_HIDE_CLASS(struct) ByteSwapImpl
        {
            static inline T Swap(T v)
            {
                for (int i = 0; i < (byte_count / 2); ++i)
                {
                    uint8_t tmp = reinterpret_cast<uint8_t*>(&v)[i];
                    reinterpret_cast<uint8_t*>(&v)[i] = reinterpret_cast<uint8_t*>(&v)[byte_count - i - 1];
                    reinterpret_cast<uint8_t*>(&v)[byte_count - i - 1] = tmp;
                }

                return v;
            }
        };

        template<typename T>
        SOCKET_HIDE_CLASS(struct) ByteSwapImpl<T, 1>
        {
            static inline T Swap(T v) { return v; }
        };

        template<typename T>
        SOCKET_HIDE_CLASS(struct) ByteSwapImpl<T, 2>
        {
            static inline T Swap(T v)
            {
                uint16_t& tmp = *reinterpret_cast<uint16_t*>(&v);
                tmp = ((tmp & 0x00ffu) << 8)
                    | ((tmp & 0xff00u) >> 8);
                return v;
            }
        };

        template<typename T>
        SOCKET_HIDE_CLASS(struct) ByteSwapImpl<T, 4>
        {
            static inline T Swap(T v)
            {
                uint32_t& tmp = *reinterpret_cast<uint32_t*>(&v);
                tmp = ((tmp & 0x000000fful) << 24)
                    | ((tmp & 0x0000ff00ul) << 8)
                    | ((tmp & 0x00ff0000ul) >> 8)
                    | ((tmp & 0xff000000ul) >> 24);
                return v;
            }
        };

        template<typename T>
        SOCKET_HIDE_CLASS(struct) ByteSwapImpl<T, 8>
        {
            static inline T Swap(T v)
            {
                uint64_t& tmp = *reinterpret_cast<uint64_t*>(&v);
                tmp = ((tmp & 0x00000000000000ffull) << 56)
                    | ((tmp & 0x000000000000ff00ull) << 40)
                    | ((tmp & 0x0000000000ff0000ull) << 24)
                    | ((tmp & 0x00000000ff000000ull) << 8)
                    | ((tmp & 0x000000ff00000000ull) >> 8)
                    | ((tmp & 0x0000ff0000000000ull) >> 24)
                    | ((tmp & 0x00ff000000000000ull) >> 40)
                    | ((tmp & 0xff00000000000000ull) >> 56);
                return v;
            }
        };

        inline bool Big()
        {
            return reinterpret_cast<const uint8_t*>(&_EndianMagic)[0] == 0x01;
        }

        template<typename T, size_t Size = sizeof(T)>
        static T NetSwap(T v)
        {
            if (!Endian::Big())
            {
                return ByteSwapImpl<T, Size>::Swap(v);
            }

            return v;
        }
    }

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