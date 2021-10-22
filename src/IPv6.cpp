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

#include <NetworkLibrary/IPv6.h>
#include "internals/internal_socket.h"

namespace NetworkLibrary {
namespace IPv6 {
    static constexpr int _AddressFamily = (int)AF_INET6;

    static constexpr int _TypeTCP = (int)SOCK_STREAM;
    static constexpr int _ProtoTCP = (int)IPPROTO_TCP;

    static constexpr int _TypeUDP = (int)SOCK_DGRAM;
    static constexpr int _ProtoUDP = (int)IPPROTO_UDP;

    /****************************************
     *
     * IPv6 implementation
     *
     ****************************************/

    std::pair<NetworkLibrary::Error, std::vector<IfaceInfos>> GetIfacesAddresses()
    {
        std::pair<NetworkLibrary::Error, std::vector<IfaceInfos>> result;
        constexpr in6_addr empty_addr{};

#if defined(UTILS_OS_WINDOWS)
        IP_ADAPTER_ADDRESSES* pAdaptersAddresses = nullptr;
        ULONG err;
        ULONG ulOutBufLen = 0;
        constexpr ULONG AdapterQueryFlags = 0 /* GAA_FLAG_INCLUDE_ALL_INTERFACES */;

        // First call to get buffer size.
        err = GetAdaptersAddresses(_AddressFamily, AdapterQueryFlags, nullptr, pAdaptersAddresses, &ulOutBufLen);
        switch (err)
        {
            case ERROR_ADDRESS_NOT_ASSOCIATED: result.first = Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NotConnected); return result;
            case ERROR_NO_DATA               : result.first = Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NotFound)    ; return result;
            case ERROR_NOT_ENOUGH_MEMORY     : result.first = Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::OutOfMemory) ; return result;
            case ERROR_INVALID_PARAMETER     : result.first = Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::InVal)       ; return result;
            default                          : result.first.NativeCode = err; result.first.ErrorCode = NetworkLibrary::Error::UnknownError; return result;
            case ERROR_BUFFER_OVERFLOW       : break;
        }

        // Mutliply buffer size by 2 so we have some extra space if the needed buffer size has grown since old call.
        if (ulOutBufLen < (std::numeric_limits<decltype(ulOutBufLen)>::max() / 2 - 1))
            ulOutBufLen *= 2;

        pAdaptersAddresses = (IP_ADAPTER_ADDRESSES*)malloc(ulOutBufLen);

        if (pAdaptersAddresses == nullptr)
        {
            result.first = Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::OutOfMemory);
            return result;
        }

        if ((err = GetAdaptersAddresses(_AddressFamily, AdapterQueryFlags, nullptr, pAdaptersAddresses, &ulOutBufLen)) == NO_ERROR)
        {
            for (IP_ADAPTER_ADDRESSES* pAdapterAddress = pAdaptersAddresses; pAdapterAddress != nullptr; pAdapterAddress = pAdapterAddress->Next)
            {
                if (pAdapterAddress->OperStatus == IfOperStatusDown || pAdapterAddress->OperStatus == IfOperStatusUp)
                {
                    IfaceInfos infos;
                    infos.IsUp = pAdapterAddress->OperStatus == IfOperStatusUp;
                    infos.Name = pAdapterAddress->AdapterName;
                    infos.FriendlyName = Internals::WCharToString(pAdapterAddress->FriendlyName);
                    infos.Description = Internals::WCharToString(pAdapterAddress->Description);

                    // Don't list ip address of down interfaces.
                    if (pAdapterAddress->OperStatus == IfOperStatusUp)
                    {
                        for (PIP_ADAPTER_UNICAST_ADDRESS_LH pAddr = pAdapterAddress->FirstUnicastAddress; pAddr != nullptr; pAddr = pAddr->Next)
                        {
                            if (pAddr->Address.lpSockaddr->sa_family == _AddressFamily)
                            {
                                const sockaddr_in6* sock_addr = reinterpret_cast<const sockaddr_in6*>(pAddr->Address.lpSockaddr);

                                if (memcmp(&sock_addr->sin6_addr, &empty_addr, sizeof(empty_addr)) != 0 && pAddr->OnLinkPrefixLength != 0)
                                {
                                    std::string addr;
                                    Internals::inet_ntop(reinterpret_cast<const sockaddr*>(sock_addr), addr);
                                    infos.Addresses.emplace_back(IfaceAddr{ addr, (uint32_t)pAddr->OnLinkPrefixLength });
                                }
                            }
                        }
                    }

                    result.second.emplace_back(std::move(infos));
                }
            }
        }
        else
        {
            switch (err)
            {
                case ERROR_ADDRESS_NOT_ASSOCIATED: result.first = Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NotConnected); return result;
                case ERROR_NO_DATA               : result.first = Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NotFound)    ; return result;
                case ERROR_BUFFER_OVERFLOW       :
                case ERROR_NOT_ENOUGH_MEMORY     : result.first = Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::OutOfMemory) ; return result;
                case ERROR_INVALID_PARAMETER     : result.first = Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::InVal)       ; return result;
                default                          : result.first = Internals::MakeUnknownError(err)                                       ; return result;
            }
        }

        free(pAdaptersAddresses);
#else
        ifaddrs* ifaces_list = nullptr;
        ifaddrs* pIface = nullptr;

        if (getifaddrs(&ifaces_list) != 0 || ifaces_list == nullptr)
        {
            result.first = Internals::LastError();
            return result;
        }

        for (pIface = ifaces_list; pIface != nullptr; pIface = pIface->ifa_next)
        {
            if (pIface->ifa_addr->sa_family == _AddressFamily)
            {
                IfaceInfos infos;
                infos.IsUp = (pIface->ifa_flags & IFF_UP) == IFF_UP;
                infos.Name = pIface->ifa_name;
                infos.FriendlyName = pIface->ifa_name;
                //infos.Description = ;

                const sockaddr_in6* sock_addr = reinterpret_cast<const sockaddr_in6*>(pIface->ifa_addr);
                const sockaddr_in6* mask_addr = reinterpret_cast<const sockaddr_in6*>(pIface->ifa_netmask);

                if (sock_addr != nullptr && memcmp(&sock_addr->sin6_addr, &empty_addr, sizeof(empty_addr)) != 0 && mask_addr != nullptr)
                {
                    std::string addr;
                    Internals::inet_ntop(reinterpret_cast<const sockaddr*>(sock_addr), addr);

                    uint8_t* mask = (uint8_t*)&mask_addr->sin6_addr;
                    int index = 0;
                    uint32_t mask_cidr = 0;
                    for (int i = 0; i < (16 * 8);)
                    {
                        if (((mask[index] >> (i++ % 8)) & 1) == 0)
                            break;
                        if ((i % 8) == 0)
                            ++index;

                        ++mask_cidr;
                    }
                    infos.Addresses.emplace_back(IfaceAddr{ addr, mask_cidr });
                }

                result.second.emplace_back(std::move(infos));
            }
        }
        freeifaddrs(ifaces_list);

#endif

        return result;
    }

    /****************************************
     * 
     * IPv6Addr implementation
     * 
     ****************************************/

    UTILS_HIDE_CLASS(class) IPv6AddrImpl
    {
        using my_sockaddr_t = sockaddr_in6;
        my_sockaddr_t _SockAddr;

    public:
        IPv6AddrImpl() :
            _SockAddr()
        {
            _SockAddr.sin6_family = _AddressFamily;
        }

        IPv6AddrImpl(IPv6AddrImpl const& other) :
            _SockAddr()
        {
            memcpy(&_SockAddr, &other._SockAddr, sizeof(my_sockaddr_t));
        }

        IPv6AddrImpl& operator=(IPv6AddrImpl const& other)
        {
            memcpy(&_SockAddr, &other._SockAddr, sizeof(my_sockaddr_t));
            return *this;
        }

        IPv6AddrImpl(IPv6AddrImpl&& other) noexcept = delete;
        IPv6AddrImpl& operator=(IPv6AddrImpl&& other) noexcept = delete;

        std::string ToString(bool with_port) const
        {
            std::string res;
            NetworkLibrary::Internals::inet_ntop(reinterpret_cast<const sockaddr*>(&_SockAddr), res);
            if (with_port)
            {
                res.insert(res.begin(), '[');
                res += "]:" + std::to_string(utils::Endian::net_swap(_SockAddr.sin6_port));
            }

            return res;
        }

        void* GetAddr()
        {
            return &_SockAddr;
        }

        const void* GetAddr() const
        {
            return &_SockAddr;
        }

        int GetFamily() const
        {
            return _AddressFamily;
        }

        size_t GetLength() const
        {
            return sizeof(my_sockaddr_t);
        }

        NetworkLibrary::Error FromString(std::string str)
        {
            if (str.empty())
                return Internals::MakeErrorFromSocketCode(Error::InVal);

            size_t pos = std::string::npos;
            uint32_t port;
            char* ip = &str[0];

            if (str[0] == '[' && (pos = str.rfind(':')) && str[pos - 1] == ']')
            {
                ++ip;
                str[pos - 1] = 0;
                port = static_cast<uint32_t>(strtoul(&str[pos + 1], nullptr, 10));
                if (port == 0 || port > 65535u)
                {
                    return Internals::MakeErrorFromSocketCode(Error::InVal);
                }
            }

            NetworkLibrary::Error error = NetworkLibrary::Internals::inet_pton(ip, reinterpret_cast<sockaddr*>(&_SockAddr));
            if (error.ErrorCode != NetworkLibrary::Error::NoError)
                return error;

            if (pos != std::string::npos)
            {
                SetPort(static_cast<uint16_t>(port));
            }

            return error;
        }

        void SetIPv6(InAddr6 ip)
        {
            memcpy(&_SockAddr.sin6_addr, &ip, sizeof(ip));
        }

        void SetPort(uint16_t port)
        {
            _SockAddr.sin6_port = utils::Endian::net_swap(port);
        }

        InAddr6 GetIPv6() const
        {
            InAddr6 res;
            memcpy(&res, &_SockAddr.sin6_addr, sizeof(res));
            return res;
        }

        uint16_t GetPort() const
        {
            return utils::Endian::net_swap(_SockAddr.sin6_port);
        }

        void SetAnyAddr()
        {
            _SockAddr.sin6_addr = IN6ADDR_ANY_INIT;
        }

        void SetLoopbackAddr()
        {
            _SockAddr.sin6_addr = IN6ADDR_LOOPBACK_INIT;
        }

        int AddressFamily()
        {
            return _AddressFamily;
        }
    };

    IPv6Addr::IPv6Addr():
        _Impl(new IPv6AddrImpl)
    {}

    IPv6Addr::IPv6Addr(IPv6Addr const& other):
        _Impl(new IPv6AddrImpl(*other._Impl))
    {}

    IPv6Addr::IPv6Addr(IPv6Addr&& other) noexcept :
        _Impl(nullptr)
    {
        _Impl = other._Impl;
        other._Impl = nullptr;
    }

    IPv6Addr& IPv6Addr::operator=(IPv6Addr const& other)
    {
        IPv6AddrImpl* tmp = new IPv6AddrImpl(*other._Impl);
        delete _Impl;
        _Impl = tmp;
        return *this;
    }

    IPv6Addr& IPv6Addr::operator=(IPv6Addr&& other) noexcept
    {
        IPv6AddrImpl* tmp = other._Impl;
        other._Impl = _Impl;
        _Impl = tmp;
        return *this;
    }

    IPv6Addr::~IPv6Addr()
    {
        delete _Impl;
    }

    std::string IPv6Addr::ToString(bool with_port) const
    {
        return _Impl->ToString(with_port);
    }

    void* IPv6Addr::GetAddr()
    {
        return _Impl->GetAddr();
    }

    const void* IPv6Addr::GetAddr() const
    {
        return _Impl->GetAddr();
    }

    int IPv6Addr::GetFamily() const
    {
        return _Impl->GetFamily();
    }

    size_t IPv6Addr::GetLength() const
    {
        return _Impl->GetLength();
    }

    NetworkLibrary::Error IPv6Addr::FromString(std::string str)
    {
        return _Impl->FromString(str);
    }

    void IPv6Addr::SetIPv6(InAddr6 ip)
    {
        _Impl->SetIPv6(ip);
    }

    void IPv6Addr::SetPort(uint16_t port)
    {
        _Impl->SetPort(port);
    }

    InAddr6 IPv6Addr::GetIPv6() const
    {
        return _Impl->GetIPv6();
    }

    uint16_t IPv6Addr::GetPort() const
    {
        return _Impl->GetPort();
    }

    void IPv6Addr::SetAnyAddr()
    {
        _Impl->SetAnyAddr();
    }

    void IPv6Addr::SetLoopbackAddr()
    {
        _Impl->SetLoopbackAddr();
    }

    int AddressFamily()
    {
        return _AddressFamily;
    }

    /****************************************
     *
     * TCP implementation
     *
     ****************************************/
    TCP::TCP()
    {}

    TCP::TCP(TCP&& other) noexcept
    {
        _Impl = other._Impl;

        other._Impl = nullptr;
    }

    TCP& TCP::operator=(TCP&& other) noexcept
    {
        auto tmp = other._Impl;
        other._Impl = _Impl;
        _Impl = tmp;

        return *this;
    }

    NetworkLibrary::Error TCP::CreateSocket()
    {
        return _Impl->CreateSocket(
            (Internals::AddressFamily)_AddressFamily,
            (Internals::SocketTypes)_TypeTCP,
            (Internals::SocketProtocols)_ProtoTCP);
    }

    NetworkLibrary::Error TCP::GetSockName(IPv6Addr& out_addr)
    {
        return Internals::getsockname(*_Impl, out_addr);
    }

    int TCP::GetFamily() const { return _AddressFamily; }
    int TCP::GetType  () const { return _TypeTCP; }
    int TCP::GetProto () const { return _ProtoTCP; }

    /****************************************
     *
     * UDP implementation
     *
     ****************************************/
    UDP::UDP()
    {}

    UDP::UDP(UDP&& other) noexcept
    {
        _Impl = other._Impl;

        other._Impl = nullptr;
    }

    UDP& UDP::operator=(UDP&& other) noexcept
    {
        auto tmp = other._Impl;
        other._Impl = _Impl;
        _Impl = tmp;

        return *this;
    }

    NetworkLibrary::Error UDP::CreateSocket()
    {
        return _Impl->CreateSocket(
            (Internals::AddressFamily)_AddressFamily,
            (Internals::SocketTypes)_TypeUDP,
            (Internals::SocketProtocols)_ProtoUDP);
    }

    NetworkLibrary::Error UDP::GetSockName(IPv6Addr& out_addr)
    {
        return Internals::getsockname(*_Impl, out_addr);
    }

    int UDP::GetFamily() const { return _AddressFamily; }
    int UDP::GetType  () const { return _TypeUDP; }
    int UDP::GetProto () const { return _ProtoUDP; }
}

}