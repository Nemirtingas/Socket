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

#include <NetworkLibrary/Bluetooth.h>
#include "internals/internal_bluetooth.h"

namespace NetworkLibrary {
namespace Bluetooth {

#if defined(SOCKET_OS_WINDOWS)
    static constexpr int _AddressFamily = (int)AF_BTH;

    static constexpr int _TypeRFCOMM = (int)SOCK_STREAM;
    static constexpr int _ProtoRFCOMM = (int)BTHPROTO_RFCOMM;

    //static constexpr int _TypeL2CAP = (int)SOCK_STREAM;
    static constexpr int _ProtoL2CAP = (int)BTHPROTO_L2CAP;

    #define SSCANF(...) sscanf_s(__VA_ARGS__)

#elif defined(SOCKET_OS_LINUX)
    static constexpr int _AddressFamily = (int)AF_BLUETOOTH;

    static constexpr int _TypeRFCOMM = (int)SOCK_STREAM;
    static constexpr int _ProtoRFCOMM = (int)BTPROTO_RFCOMM;

    //static constexpr int _TypeL2CAP = (int)SOCK_STREAM;
    static constexpr int _ProtoL2CAP = (int)BTPROTO_L2CAP;

    #define SSCANF(...) sscanf(__VA_ARGS__)

#endif

    #define __UUID128_PRINTF_FORMAT__ "%08x-%04hx-%04hx-%04hx-%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx"

    /****************************************
     *
     * UUID implementation
     *
     ****************************************/

    UUID::UUID():
        _UUID{}
    {}

    UUID::UUID(UUID const& other)
    {
        memcpy(&_UUID, &other._UUID, sizeof(_UUID));
    }

    UUID::UUID(UUID&& other) noexcept
    {
        memcpy(&_UUID, &other._UUID, sizeof(_UUID));
    }

    UUID& UUID::operator=(UUID const& other)
    {
        memcpy(&_UUID, &other._UUID, sizeof(_UUID));
        return *this;
    }

    UUID& UUID::operator=(UUID&& other) noexcept
    {
        memcpy(&_UUID, &other._UUID, sizeof(_UUID));
        return *this;
    }

    BluetoothUUID_t UUID::GetUUID() const
    {
        return _UUID;
    }

    NetworkLibrary::Error UUID::SetUUID16(uint16_t uuid)
    {
        return SetUUID32(uuid);
    }

    NetworkLibrary::Error UUID::SetUUID32(uint32_t uuid)
    {
        *this = GetBluetoothBaseUUID();

#if defined(SOCKET_OS_LINUX) // Linux UUID is net ordered.
        uuid = Internals::Endian::NetSwap(uuid);
#endif
        *reinterpret_cast<uint32_t*>(&_UUID.uuid[0]) = uuid;

        return Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NoError);
    }

    NetworkLibrary::Error UUID::SetUUID128(BluetoothUUID_t const& uuid)
    {
        memcpy(&_UUID, &uuid, sizeof(uuid));
        return Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NoError);
    }

    bool UUID::operator ==(UUID const& other) const
    {
        return memcmp(&_UUID, &other._UUID, 16) == 0;
    }
    
    bool UUID::operator !=(UUID const& other) const
    {
        return !(*this == other);
    }

    NetworkLibrary::Error UUID::FromString(std::string const& str_uuid)
    {
        if (!IsValidUUID(str_uuid))
            return Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::InVal);
        
        SSCANF(str_uuid.c_str(), __UUID128_PRINTF_FORMAT__,
            (uint32_t*)&_UUID.uuid[0],
            (uint16_t*)&_UUID.uuid[4],
            (uint16_t*)&_UUID.uuid[6],
            (uint16_t*)&_UUID.uuid[8],
            &_UUID.uuid[10], &_UUID.uuid[11], &_UUID.uuid[12], &_UUID.uuid[13], &_UUID.uuid[14], &_UUID.uuid[15]);

#if defined(SOCKET_OS_LINUX) // Linux UUID is net ordered.
        *(uint32_t*)&_UUID.uuid[0] = Internals::Endian::NetSwap(*(uint32_t*)&_UUID.uuid[0]);
        *(uint16_t*)&_UUID.uuid[4] = Internals::Endian::NetSwap(*(uint16_t*)&_UUID.uuid[4]);
        *(uint16_t*)&_UUID.uuid[6] = Internals::Endian::NetSwap(*(uint16_t*)&_UUID.uuid[6]);
#endif
        *(uint16_t*)&_UUID.uuid[8] = Internals::Endian::NetSwap(*(uint16_t*)&_UUID.uuid[8]);

        return Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NoError);
    }

    std::string UUID::ToString() const
    {
        std::string res(36, '\0');
        
#if defined(SOCKET_OS_WINDOWS)
        snprintf(&res[0], 37, __UUID128_PRINTF_FORMAT__,
            *(uint32_t*)&_UUID.uuid[0],
            *(uint16_t*)&_UUID.uuid[4],
            *(uint16_t*)&_UUID.uuid[6],
            Internals::Endian::NetSwap(*(uint16_t*)&_UUID.uuid[8]),
            _UUID.uuid[10], _UUID.uuid[11], _UUID.uuid[12], _UUID.uuid[13], _UUID.uuid[14], _UUID.uuid[15]);
#elif defined(SOCKET_OS_LINUX)
        snprintf(&res[0], 37, __UUID128_PRINTF_FORMAT__,
            Internals::Endian::NetSwap(*(uint32_t*)&_UUID.uuid[0]),
            Internals::Endian::NetSwap(*(uint16_t*)&_UUID.uuid[4]),
            Internals::Endian::NetSwap(*(uint16_t*)&_UUID.uuid[6]),
            Internals::Endian::NetSwap(*(uint16_t*)&_UUID.uuid[8]),
            _UUID.uuid[10], _UUID.uuid[11], _UUID.uuid[12], _UUID.uuid[13], _UUID.uuid[14], _UUID.uuid[15]);
#endif

        return res;
    }

    bool UUID::IsValidUUID(std::string const& struuid)
    {
        if (struuid.length() != 36
            || struuid[8] != '-' || struuid[13] != '-'
            || struuid[18] != '-' || struuid[23] != '-')
        {
            return false;
        }
        const char* ptr = struuid.c_str();

        for (int i = 0; i < 36; ++i, ++ptr)
        {
            if (i != 8 && i != 13 && i != 18 && i != 23)
            {
                char c = *ptr;
                if (((c < '0') || (c > '9')) &&
                    ((c < 'A') || (c > 'F')) &&
                    ((c < 'a') || (c > 'f')))
                {
                    return false;
                }
            }
        }
        return true;
    }

    UUID UUID::GetBluetoothBaseUUID()
    {
#if defined(SOCKET_OS_WINDOWS)
        static constexpr BluetoothUUID_t bluetooth_base = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB };
#else
        static constexpr BluetoothUUID_t bluetooth_base = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB };
#endif

        UUID res;
        res._UUID = bluetooth_base;
        return res;
    }

    /****************************************
     *
     * SDPService implementation
     *
     ****************************************/
    SDPService::SDPService():
        _SDPHandle(nullptr)
    {}
    
    SDPService::SDPService(SDPService&& other) noexcept:
        _SDPHandle(other._SDPHandle)
    {
        _SDPHandle = nullptr;
    }
    
    SDPService& SDPService::operator=(SDPService&& other) noexcept
    {
        void* tmp = other._SDPHandle;
        other._SDPHandle = _SDPHandle;
        _SDPHandle = other._SDPHandle;
        return *this;
    }

    SDPService::~SDPService()
    {
        BluetoothInternals::UnregisterSDPServiceRFCOMM(reinterpret_cast<SDPServiceHandle>(_SDPHandle));
        _SDPHandle = nullptr;
    }

    NetworkLibrary::Error SDPService::RegisterSDPServiceRFCOMM(NetworkLibrary::Bluetooth::UUID const& uuid, uint8_t port, std::string const& srv_name, std::string const& srv_prov, std::string const& srv_desc)
    {
        return BluetoothInternals::RegisterSDPServiceRFCOMM(reinterpret_cast<SDPServiceHandle*>(&_SDPHandle), uuid, port, srv_name, srv_prov, srv_desc);
    }

    NetworkLibrary::Error SDPService::UnregisterSDPServiceRFCOMM()
    {
        return BluetoothInternals::UnregisterSDPServiceRFCOMM(reinterpret_cast<SDPServiceHandle>(_SDPHandle));
    }

    /****************************************
     *
     * Bluetooth implementation
     *
     ****************************************/
    std::pair<NetworkLibrary::Error, std::vector<IfaceInfos>> GetIfacesAddresses()
    {
        std::pair<NetworkLibrary::Error, std::vector<IfaceInfos>> result;

#if defined(SOCKET_OS_WINDOWS)
        HANDLE radio_handle = nullptr;
        BLUETOOTH_FIND_RADIO_PARAMS find_params{};
        BLUETOOTH_RADIO_INFO radio_infos{};
        DWORD error;

        find_params.dwSize = sizeof(find_params);
        auto enumeration_handle = BluetoothFindFirstRadio(&find_params, &radio_handle);
        if (enumeration_handle == nullptr)
        {
            error = GetLastError();
            result.first.NativeCode = error;
            switch (error)
            {
                case ERROR_NO_MORE_ITEMS    : result.first.ErrorCode = NetworkLibrary::Error::NoError; break;
                case ERROR_INVALID_PARAMETER: result.first.ErrorCode = NetworkLibrary::Error::InVal; break;
                case ERROR_OUTOFMEMORY      : result.first.ErrorCode = NetworkLibrary::Error::OutOfMemory; break;
                default                     : result.first.ErrorCode = NetworkLibrary::Error::UnknownError; break;
            }    

            return result;
        }

        for(int i = 0; ; ++i)
        {
            radio_infos.dwSize = sizeof(radio_infos);
            if (BluetoothGetRadioInfo(radio_handle, &radio_infos) == ERROR_SUCCESS)
            {
                bluetooth_sockaddr bth_addr;
                IfaceInfos iface_infos;
                IfaceAddr iface_addr;

                bth_addr.BTH_FAMILY = _AddressFamily;
                memcpy(&bth_addr.BTH_ADDR, &radio_infos.address.ullLong, sizeof(bth_addr.BTH_ADDR));

                BluetoothInternals::bth_ntop(&bth_addr, iface_addr.Addr);
                iface_addr.MaskCIDR = 0;

                iface_infos.Name = "hci" + std::to_string(i);
                iface_infos.FriendlyName = Internals::WCharToString(radio_infos.szName);
                iface_infos.Addresses.emplace_back(std::move(iface_addr));
                iface_infos.IsUp = true;

                result.second.emplace_back(std::move(iface_infos));
            }

            if (radio_handle != nullptr)
            {
                CloseHandle(radio_handle);
            }

            if (BluetoothFindNextRadio(enumeration_handle, &radio_handle) == FALSE)
            {
                error = GetLastError();
                result.first.NativeCode = error;
                switch (error)
                {
                    case ERROR_NO_MORE_ITEMS    : result.first.ErrorCode = NetworkLibrary::Error::NoError; break;
                    case ERROR_INVALID_PARAMETER: result.first.ErrorCode = NetworkLibrary::Error::InVal; break;
                    case ERROR_OUTOFMEMORY      : result.first.ErrorCode = NetworkLibrary::Error::OutOfMemory; break;
                    default                     : result.first.ErrorCode = NetworkLibrary::Error::UnknownError; break;
                }

                break;
            }
        }

        BluetoothFindRadioClose(enumeration_handle);
        result.first = NetworkLibrary::Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NoError);
#else
        char name_buffer[255];
        int hci_fd;
        hci_dev_info hci_infos;
        for (int i = 0; hci_devinfo(i, &hci_infos) == 0; ++i)
        {
            bluetooth_sockaddr bth_addr;
            IfaceInfos iface_infos;
            IfaceAddr iface_addr;

            hci_fd = hci_open_dev(hci_infos.dev_id);
            if (hci_fd >= 0)
            {
                hci_read_local_name(hci_fd, sizeof(name_buffer) / sizeof(*name_buffer), name_buffer, 0);
                hci_close_dev(hci_fd);

                bth_addr.BTH_FAMILY = _AddressFamily;
                memcpy(&bth_addr.BTH_ADDR, &hci_infos.bdaddr, sizeof(bth_addr.BTH_ADDR));
                BluetoothInternals::bth_ntop(&bth_addr, iface_addr.Addr);
                iface_addr.MaskCIDR = 0;

                iface_infos.Name = hci_infos.name;
                iface_infos.FriendlyName = name_buffer;
                iface_infos.Addresses.emplace_back(std::move(iface_addr));
                iface_infos.IsUp = true;

                result.second.emplace_back(std::move(iface_infos));
            }
        }

        result.first = NetworkLibrary::Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NoError);
#endif
        
        return result;
    }

    std::vector<BluetoothDevice> ScanDevices(bool flushCache)
    {
        return BluetoothInternals::ScanDevices(flushCache);
    }

    NetworkLibrary::Error GetChannelFromServiceUUID(UUID const& uuid, BasicAddr const& remote_addr, uint8_t& out_channel)
    {
        return BluetoothInternals::GetChannelFromServiceUUID(uuid, remote_addr, out_channel);
    }

    /****************************************
     * 
     * BluetoothAddr implementation
     * 
     ****************************************/

    SOCKET_HIDE_CLASS(class) BluetoothAddrImpl
    {
        using my_sockaddr_t = bluetooth_sockaddr;
        my_sockaddr_t _SockAddr;

        static_assert(sizeof(bluetooth_sockaddr::BTH_ADDR) >= sizeof(BluetoothMac), "BluetoothMac can't hold a bluetooth_sockaddr::BTH_ADDR");

    public:
        BluetoothAddrImpl() :
            _SockAddr()
        {
            _SockAddr.BTH_FAMILY = _AddressFamily;
        }

        BluetoothAddrImpl(BluetoothAddrImpl const& other) :
            _SockAddr()
        {
            memcpy(&_SockAddr, &other._SockAddr, sizeof(my_sockaddr_t));
        }

        BluetoothAddrImpl& operator=(BluetoothAddrImpl const& other)
        {
            memcpy(&_SockAddr, &other._SockAddr, sizeof(my_sockaddr_t));
            return *this;
        }

        BluetoothAddrImpl(BluetoothAddrImpl&& other) noexcept = delete;
        BluetoothAddrImpl& operator=(BluetoothAddrImpl&& other) noexcept = delete;

        std::string ToString(bool with_port) const
        {
            std::string res;
            NetworkLibrary::BluetoothInternals::bth_ntop(&_SockAddr, res);
            if (with_port)
            {
                res.push_back('@');
                res += std::to_string(_SockAddr.BTH_CHANNEL);
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

            size_t pos = str.find('@');
            uint32_t port;

            if (pos != std::string::npos)
            {
                str[pos] = 0;
                port = static_cast<uint32_t>(strtoul(&str[pos + 1], nullptr, 10));
                if (port == 0 || port > 255u)
                {
                    return Internals::MakeErrorFromSocketCode(Error::InVal);
                }
            }

            NetworkLibrary::Error error = NetworkLibrary::BluetoothInternals::bth_pton(&str[0], &_SockAddr);
            if (error.ErrorCode != NetworkLibrary::Error::NoError)
                return error;

            if (pos != std::string::npos)
            {
                SetChannel(static_cast<uint8_t>(port));
            }

            return error;
        }

        void SetAddress(BluetoothMac addr)
        {
            memcpy(&_SockAddr.BTH_ADDR, &addr, sizeof(BluetoothMac));
        }

        void SetChannel(uint8_t channel)
        {
            _SockAddr.BTH_CHANNEL = channel;
        }

        BluetoothMac GetAddress() const
        {
            BluetoothMac mac;
            memcpy(&mac, &_SockAddr.BTH_ADDR, sizeof(BluetoothMac));
            return mac;
        }

        uint8_t GetChannel() const
        {
            return static_cast<uint8_t>(_SockAddr.BTH_CHANNEL);
        }

        void SetAnyAddr()
        {
            memset(&_SockAddr.BTH_ADDR, 0, sizeof(bluetooth_sockaddr::BTH_ADDR));
        }

        void SetLocalAddr()
        {
            
        }
    };

    BluetoothAddr::BluetoothAddr():
        _Impl(new BluetoothAddrImpl())
    {}

    BluetoothAddr::BluetoothAddr(BluetoothAddr const& other):
        _Impl(new BluetoothAddrImpl(*other._Impl))
    {}

    BluetoothAddr::BluetoothAddr(BluetoothAddr&& other) noexcept :
        _Impl(other._Impl)
    {
        other._Impl = nullptr;
    }

    BluetoothAddr& BluetoothAddr::operator=(BluetoothAddr const& other)
    {
        BluetoothAddrImpl* tmp = new BluetoothAddrImpl(*other._Impl);
        delete _Impl;
        _Impl = tmp;
        return *this;
    }

    BluetoothAddr& BluetoothAddr::operator=(BluetoothAddr&& other) noexcept
    {
        BluetoothAddrImpl* tmp = other._Impl;
        other._Impl = _Impl;
        _Impl = tmp;
        return *this;
    }

    BluetoothAddr::~BluetoothAddr()
    {
        delete _Impl;
    }

    std::string BluetoothAddr::ToString(bool with_port) const
    {
        return _Impl->ToString(with_port);
    }

    void* BluetoothAddr::GetAddr()
    {
        return _Impl->GetAddr();
    }

    const void* BluetoothAddr::GetAddr() const
    {
        return _Impl->GetAddr();
    }

    int BluetoothAddr::GetFamily() const
    {
        return _Impl->GetFamily();
    }

    size_t BluetoothAddr::GetLength() const
    {
        return _Impl->GetLength();
    }

    NetworkLibrary::Error BluetoothAddr::FromString(std::string str)
    {
        return _Impl->FromString(str);
    }

    void BluetoothAddr::SetAddress(BluetoothMac addr)
    {
        return _Impl->SetAddress(addr);
    }

    void BluetoothAddr::SetChannel(uint8_t channel)
    {
        return _Impl->SetChannel(channel);
    }

    BluetoothMac BluetoothAddr::GetAddress() const
    {
        return _Impl->GetAddress();
    }

    uint8_t BluetoothAddr::GetChannel() const
    {
        return _Impl->GetChannel();
    }

    void BluetoothAddr::SetAnyAddr()
    {
        return _Impl->SetAnyAddr();
    }

    void BluetoothAddr::SetLocalAddr()
    {
        return _Impl->SetLocalAddr();
    }

    /****************************************
     *
     * RFCOMM implementation
     *
     ****************************************/
    RFCOMM::RFCOMM()
    {}
    
    RFCOMM::RFCOMM(RFCOMM&& other) noexcept
    {
        _Impl = other._Impl;
    
        other._Impl = nullptr;
    }
    
    RFCOMM& RFCOMM::operator=(RFCOMM&& other) noexcept
    {
        auto tmp = other._Impl;
        other._Impl = _Impl;
        _Impl = tmp;
    
        return *this;
    }
    
    NetworkLibrary::Error RFCOMM::CreateSocket()
    {
        return _Impl->CreateSocket(
            (Internals::AddressFamily)_AddressFamily,
            (Internals::SocketTypes)_TypeRFCOMM,
            (Internals::SocketProtocols)_ProtoRFCOMM);
    }

    NetworkLibrary::Error RFCOMM::GetSockName(BluetoothAddr& out_addr)
    {
        return Internals::getsockname(*_Impl, out_addr);
    }
    
    int RFCOMM::GetFamily() const { return _AddressFamily; }
    int RFCOMM::GetType  () const { return _TypeRFCOMM; }
    int RFCOMM::GetProto () const { return _ProtoRFCOMM; }
}

}