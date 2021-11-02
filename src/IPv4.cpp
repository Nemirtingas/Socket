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

#include <NetworkLibrary/IPv4.h>
#include "internals/internal_socket.h"

namespace NetworkLibrary {
namespace IPv4 {
    static constexpr int _AddressFamily = (int)AF_INET;

    static constexpr int _TypeTCP = (int)SOCK_STREAM;
    static constexpr int _ProtoTCP = (int)IPPROTO_TCP;

    static constexpr int _TypeUDP = (int)SOCK_DGRAM;
    static constexpr int _ProtoUDP = (int)IPPROTO_UDP;

    /****************************************
     *
     * IPv4 implementation
     *
     ****************************************/

    std::pair<NetworkLibrary::Error, std::vector<IfaceInfos>> GetIfacesAddresses()
    {
        std::pair<NetworkLibrary::Error, std::vector<IfaceInfos>> result;

#if defined(SYSTEM_OS_WINDOWS)
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
            default                          : result.first = Internals::MakeUnknownError(err)                                       ; return result;
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
                                const sockaddr_in* sock_addr = reinterpret_cast<const sockaddr_in*>(pAddr->Address.lpSockaddr);

                                if (sock_addr->sin_addr.s_addr != 0 && pAddr->OnLinkPrefixLength != 0)
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

                const sockaddr_in* sock_addr = reinterpret_cast<const sockaddr_in*>(pIface->ifa_addr);
                const sockaddr_in* mask_addr = reinterpret_cast<const sockaddr_in*>(pIface->ifa_netmask);

                if (sock_addr != nullptr && sock_addr->sin_addr.s_addr != 0 && mask_addr != nullptr)
                {
                    std::string addr;
                    Internals::inet_ntop(reinterpret_cast<const sockaddr*>(sock_addr), addr);

                    uint32_t mask = System::Endian::net_swap(mask_addr->sin_addr.s_addr);
                    uint32_t mask_cidr = 0;
                    for (int i = 0; i < 32;)
                    {
                        if (((mask << i++) & 0x80000000) == 0)
                            break;

                        ++mask_cidr;
                    }
                    infos.Addresses.emplace_back(IfaceAddr{ addr, mask_cidr });
                }

                result.second.emplace_back(std::move(infos));
            }
        }
        freeifaddrs(ifaces_list);

#endif

        result.first = Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NoError);
        return result;
    }

    /****************************************
     * 
     * IPv4Addr implementation
     * 
     ****************************************/

    SYSTEM_HIDE_CLASS(class) IPv4AddrImpl
    {
        using my_sockaddr_t = sockaddr_in;
        my_sockaddr_t _SockAddr;

    public:
        IPv4AddrImpl() :
            _SockAddr()
        {
            _SockAddr.sin_family = _AddressFamily;
        }

        IPv4AddrImpl(IPv4AddrImpl const& other) :
            _SockAddr()
        {
            memcpy(&_SockAddr, &other._SockAddr, sizeof(my_sockaddr_t));
        }

        IPv4AddrImpl& operator=(IPv4AddrImpl const& other)
        {
            memcpy(&_SockAddr, &other._SockAddr, sizeof(my_sockaddr_t));
            return *this;
        }

        IPv4AddrImpl(IPv4AddrImpl&& other) noexcept = delete;
        IPv4AddrImpl& operator=(IPv4AddrImpl&& other) noexcept = delete;

        std::string ToString(bool with_port) const
        {
            std::string res;
            NetworkLibrary::Internals::inet_ntop(reinterpret_cast<const sockaddr*>(&_SockAddr), res);
            if (with_port)
            {
                res.push_back(':');
                res += std::to_string(System::Endian::net_swap(_SockAddr.sin_port));
            }

            return res;
        }

        int GetFamily()
        {
            return _AddressFamily;
        }

        void* GetAddr()
        {
            return &_SockAddr;
        }

        const void* GetAddr() const
        {
            return &_SockAddr;
        }

        size_t GetLength() const
        {
            return sizeof(my_sockaddr_t);
        }

        NetworkLibrary::Error FromString(std::string str)
        {
            if (str.empty())
                return Internals::MakeErrorFromSocketCode(Error::InVal);

            size_t pos = str.find(':');
            uint32_t port;

            if (pos != std::string::npos)
            {
                str[pos] = 0;
                port = static_cast<uint32_t>(strtoul(&str[pos + 1], nullptr, 10));
                if (port == 0 || port > 65535u)
                {
                    return Internals::MakeErrorFromSocketCode(Error::InVal);
                }
            }

            NetworkLibrary::Error error = NetworkLibrary::Internals::inet_pton(&str[0], reinterpret_cast<sockaddr*>(&_SockAddr));
            if (error.ErrorCode != NetworkLibrary::Error::NoError)
                return error;

            if (pos != std::string::npos)
            {
                SetPort(static_cast<uint16_t>(port));
            }

            return error;
        }

        void SetIPv4(uint32_t ip)
        {
            _SockAddr.sin_addr.s_addr = System::Endian::net_swap(ip);
        }

        void SetPort(uint16_t port)
        {
            _SockAddr.sin_port = System::Endian::net_swap(port);
        }

        uint32_t GetIPv4() const
        {
            return System::Endian::net_swap(_SockAddr.sin_addr.s_addr);
        }

        uint16_t GetPort() const
        {
            return System::Endian::net_swap(_SockAddr.sin_port);
        }

        void SetAnyAddr()
        {
            _SockAddr.sin_addr.s_addr = INADDR_ANY;
        }

        void SetLoopbackAddr()
        {
            _SockAddr.sin_addr.s_addr = System::Endian::net_swap(INADDR_LOOPBACK);
        }

        void SetBroadcastAddr()
        {
            _SockAddr.sin_addr.s_addr = INADDR_BROADCAST;
        }
    };

    IPv4Addr::IPv4Addr(): _Impl(new IPv4AddrImpl)
    {}

    IPv4Addr::IPv4Addr(IPv4Addr const& other):
        _Impl(new IPv4AddrImpl(*other._Impl))
    {}

    IPv4Addr::IPv4Addr(IPv4Addr&& other) noexcept:
        _Impl(other._Impl)
    {
        other._Impl = nullptr;
    }

    IPv4Addr& IPv4Addr::operator=(IPv4Addr const& other)
    {
        IPv4AddrImpl* tmp(new IPv4AddrImpl(*other._Impl));
        delete _Impl;
        _Impl = tmp;
        return *this;
    }

    IPv4Addr& IPv4Addr::operator=(IPv4Addr&& other) noexcept
    {
        IPv4AddrImpl* tmp = other._Impl;
        other._Impl = _Impl;
        _Impl = tmp;
        return *this;
    }

    IPv4Addr::~IPv4Addr()
    {
        delete _Impl;
    }

    std::string IPv4Addr::ToString(bool with_port) const
    {
        return _Impl->ToString(with_port);
    }

    int IPv4Addr::GetFamily() const
    {
        return _Impl->GetFamily();
    }

    void* IPv4Addr::GetAddr()
    {
        return _Impl->GetAddr();
    }

    const void* IPv4Addr::GetAddr() const
    {
        return _Impl->GetAddr();
    }

    size_t IPv4Addr::GetLength() const
    {
        return _Impl->GetLength();
    }

    NetworkLibrary::Error IPv4Addr::FromString(std::string str)
    {
        return _Impl->FromString(str);
    }

    void IPv4Addr::SetIPv4(uint32_t ip)
    {
        _Impl->SetIPv4(ip);
    }

    void IPv4Addr::SetPort(uint16_t port)
    {
        _Impl->SetPort(port);
    }

    uint32_t IPv4Addr::GetIPv4() const
    {
        return _Impl->GetIPv4();
    }

    uint16_t IPv4Addr::GetPort() const
    {
        return _Impl->GetPort();
    }

    void IPv4Addr::SetAnyAddr()
    {
        _Impl->SetAnyAddr();
    }

    void IPv4Addr::SetLoopbackAddr()
    {
        _Impl->SetLoopbackAddr();
    }

    void IPv4Addr::SetBroadcastAddr()
    {
        _Impl->SetBroadcastAddr();
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

    NetworkLibrary::Error TCP::GetSockName(IPv4Addr& out_addr)
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

    NetworkLibrary::Error UDP::GetSockName(IPv4Addr& out_addr)
    {
        return Internals::getsockname(*_Impl, out_addr);
    }

    int UDP::GetFamily() const { return _AddressFamily; }
    int UDP::GetType  () const { return _TypeUDP; }
    int UDP::GetProto () const { return _ProtoUDP; }
}

}