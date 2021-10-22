/* Copyright (C) 2019-2020 Nemirtingas
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

#include "internal_bluetooth.h"

#if defined(UTILS_OS_LINUX) && !defined(USE_BLUEZ_COMPAT)
#include <thread>
#include <iostream>
#include <iomanip>      // for setfill and setw
#include <dbus/dbus.h> // for bluez

namespace NetworkLibrary {
namespace BluetoothInternals {

SOCKET_HIDE_SYMBOLS(class) BluezDBusSystem
{
private:
    DBusConnection* _Conn;
    DBusError _Err;

    BluezDBusSystem():
        _Conn(nullptr),
        _Err(DBUS_ERROR_INIT)
    {}

    BluezDBusSystem(BluezDBusSystem const&) = delete;
    BluezDBusSystem(BluezDBusSystem&&) = delete;
    BluezDBusSystem& operator=(BluezDBusSystem const&) = delete;
    BluezDBusSystem& operator=(BluezDBusSystem&&) = delete;

public:
    static BluezDBusSystem& Inst()
    {
        static BluezDBusSystem* _inst = new BluezDBusSystem;
        return *_inst;
    }

    ~BluezDBusSystem()
    {
        if (_Conn != nullptr)
        {
            dbus_connection_unref(_Conn);
            _Conn = nullptr;
        }
    }

    NetworkLibrary::Error Initialize()
    {
        if (_Conn == nullptr)
        {
            dbus_error_init(&_Err);
            _Conn = dbus_bus_get(DBUS_BUS_SYSTEM, &_Err);
            if (_Conn == nullptr)
            {
                return Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::Fault);
            }
        }

        return Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NoError);
    }

    operator DBusConnection* () { return _Conn; }
    operator DBusError* () { return &_Err; }
    const char* message() const { return _Err.message; }
};
}
}

#endif

namespace NetworkLibrary {
namespace BluetoothInternals {

#if defined(UTILS_OS_WINDOWS)
    static constexpr int _AddressFamily = (int)AF_BTH;

#elif defined(UTILS_OS_LINUX)
    static constexpr int _AddressFamily = (int)AF_BLUETOOTH;

#endif

static bool is_hex_char(int c)
{
    return ((c >= '0' && c <= '9') ||
            (c >= 'A' && c <= 'F') ||
            (c >= 'a' && c <= 'f'));
}

static std::string bluetoothmac_to_string(NetworkLibrary::Bluetooth::BluetoothMac addr)
{
    std::string str_addr(17, '\0');
    char* s = &str_addr[0];

    addr = utils::Endian::net_swap(addr);

    for (int i = 0; i < 6; ++i)
    {
        uint8_t p = addr.addr[i];
        uint8_t v = (p >> 4);
        *s++ = v + (v > 9 ? 55 : 48);
        v = (p & 0x0f);
        *s++ = v + (v > 9 ? 55 : 48);
        *s++ = (i == 5 ? '\0' : ':');
    }

    return str_addr;
}

SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) bth_pton(std::string const& str_addr, bluetooth_sockaddr* addr)
{
    if (addr->BTH_FAMILY != _AddressFamily)
        return NetworkLibrary::Internals::MakeErrorFromSocketCode(::NetworkLibrary::Error::AfNotSupported);

    if(str_addr.length() != 17)
        return NetworkLibrary::Internals::MakeErrorFromSocketCode(::NetworkLibrary::Error::InVal);
    
    for (int i = 0; i < 17; i += 3)
    {
        if (!is_hex_char(str_addr[i]) || !is_hex_char(str_addr[i+1]))
            return NetworkLibrary::Internals::MakeErrorFromSocketCode(::NetworkLibrary::Error::InVal);

        if(i != 15 && str_addr[i+2] != ':')
            return NetworkLibrary::Internals::MakeErrorFromSocketCode(::NetworkLibrary::Error::InVal);
    }
    
    NetworkLibrary::Bluetooth::BluetoothMac* bluetooth_addr = reinterpret_cast<NetworkLibrary::Bluetooth::BluetoothMac*>(&addr->BTH_ADDR);
    
    // hex string to byte array
    for (int i = 0; i < 6; ++i)
    {
        uint32_t index = i * 3;
        uint8_t c = ((unsigned char)str_addr[index] - 16) & 0x1F;
        uint8_t v = 0;
        if (c >= 0x10)
            c -= 7;

        v = c << 4u;

        c = ((unsigned char)str_addr[index + 1] - 16) & 0x1F;
        if (c >= 0x10)
            c -= 7;

        v |= c;

        bluetooth_addr->addr[i] = v;
    }

    // Net Swap 6 bytes.
    *bluetooth_addr = utils::Endian::net_swap(*bluetooth_addr);

    return NetworkLibrary::Internals::MakeErrorFromSocketCode(::NetworkLibrary::Error::NoError);
}

SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) bth_ntop(bluetooth_sockaddr const* addr, std::string& str_addr)
{
    if (addr->BTH_FAMILY != _AddressFamily)
        return NetworkLibrary::Internals::MakeErrorFromSocketCode(::NetworkLibrary::Error::AfNotSupported);

    str_addr = bluetoothmac_to_string(*reinterpret_cast<const NetworkLibrary::Bluetooth::BluetoothMac*>(&addr->BTH_ADDR));

    return NetworkLibrary::Internals::MakeErrorFromSocketCode(::NetworkLibrary::Error::NoError);
}

#if defined(UTILS_OS_WINDOWS)

static uint8_t sdp_get_proto_port(SDP_ELEMENT_DATA& protocol_container, WORD proto_uuid)
{
    uint8_t port = 0xffu;
    // HBLUETOOTH_CONTAINER_ELEMENT is actually just a PBYTE with offset to the next SDP type
    HBLUETOOTH_CONTAINER_ELEMENT hProtocolContainer = NULL;
    SDP_ELEMENT_DATA protocol_sequence;
    SDP_ELEMENT_DATA elem;

    while (BluetoothSdpGetContainerElementData(protocol_container.data.sequence.value, protocol_container.data.sequence.length, &hProtocolContainer, &protocol_sequence) == ERROR_SUCCESS)
    {
        HBLUETOOTH_CONTAINER_ELEMENT hProtocolSequence = NULL;
        int proto_port = -1;
        GUID uuid = { 0 };
        while (BluetoothSdpGetContainerElementData(protocol_sequence.data.sequence.value, protocol_sequence.data.sequence.length, &hProtocolSequence, &elem) == ERROR_SUCCESS)
        {
            switch (elem.type)
            {
                case SDP_TYPE_UUID:
                    switch (elem.specificType)
                    {
                        case SDP_ST_UUID16 : uuid.Data1 = elem.data.uuid16; break;
                        case SDP_ST_UUID32 : uuid.Data1 = elem.data.uuid32; break;
                        case SDP_ST_UUID128: uuid       = elem.data.uuid128; break;
                    }
                    break;

                case SDP_TYPE_UINT:
                    switch (elem.specificType)
                    {
                        case SDP_ST_UINT8 : proto_port = elem.data.uint8; break;
                        case SDP_ST_UINT16: proto_port = elem.data.uint16; break;
                    }
                    break;

                case SDP_TYPE_INT:
                    switch (elem.specificType)
                    {
                        case SDP_ST_INT8 : proto_port = elem.data.int8; break;
                        case SDP_ST_INT16: proto_port = elem.data.int16; break;
                    }
                    break;
            }
        }
        if (uuid.Data1 == proto_uuid)
        {
            port = proto_port;
            break;
        }
    }

    return port;
}

SOCKET_HIDE_SYMBOLS(std::vector<NetworkLibrary::Bluetooth::BluetoothDevice>) ScanDevices(bool flushCache)
{
    std::vector<NetworkLibrary::Bluetooth::BluetoothDevice> devices;
    NetworkLibrary::Bluetooth::BluetoothDevice device;

    HANDLE hLookup;
    constexpr size_t buffer_size = 8192;
    uint8_t* buffer = (uint8_t*)malloc(buffer_size);
    if (buffer == nullptr)
        return devices;

    WSAQUERYSETW wsaq = { 0 };
    wsaq.dwSize = sizeof(WSAQUERYSETW);
    wsaq.dwNameSpace = NS_BTH;
    wsaq.lpcsaBuffer = 0;
    int flags = LUP_CONTAINERS;
    if (flushCache)
        flags |= LUP_FLUSHCACHE;

    // Starting scan.
    while (WSALookupServiceBeginW(&wsaq, flags, &hLookup) != 0)
    {
        int error = WSAGetLastError();
        switch (error)
        {
            case WSANOTINITIALISED   : Internals::WinSockInitializer::Initialize(); break;
            case WSASERVICE_NOT_FOUND:
            default:
                return devices;
        }
    }

    LPWSAQUERYSETW pwsaResults = (LPWSAQUERYSETW)buffer;
    DWORD dwSize = buffer_size;
    memset(pwsaResults, 0, sizeof(WSAQUERYSETW));
    pwsaResults->dwSize = sizeof(WSAQUERYSETW);
    pwsaResults->dwNameSpace = NS_BTH;
    pwsaResults->lpBlob = NULL;

    while (!WSALookupServiceNextW(hLookup, LUP_RETURN_NAME | LUP_RETURN_ADDR, &dwSize, pwsaResults))
    {
        if (pwsaResults->dwNumberOfCsAddrs != 1)
            break;

        BluetoothInternals::bth_ntop((SOCKADDR_BTH*)pwsaResults->lpcsaBuffer->RemoteAddr.lpSockaddr, device.Address);
        if (pwsaResults->lpszServiceInstanceName != nullptr && wcslen(pwsaResults->lpszServiceInstanceName) > 0)
        {
            device.Name = Internals::WCharToString(pwsaResults->lpszServiceInstanceName);
        }
        else
        {
            device.Name = "[unknown]";
        }

        devices.emplace_back(device);
    }

    WSALookupServiceEnd(hLookup);

    return devices;
}

SOCKET_HIDE_SYMBOLS(NetworkLibrary::Error) GetChannelFromServiceUUID(NetworkLibrary::Bluetooth::UUID const& uuid, BasicAddr const& remote_addr, uint8_t& out_channel)
{
    if(remote_addr.GetFamily() != _AddressFamily)
        Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::AfNotSupported);

    NetworkLibrary::Error error = Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NotFound);
    HANDLE hLookup;
    GUID protocol = RFCOMM_PROTOCOL_UUID;
    DWORD flags = LUP_FLUSHCACHE | LUP_RETURN_BLOB;
    int result;

    WSAQUERYSETW* querySet = reinterpret_cast<WSAQUERYSETW*>(malloc(sizeof(WSAQUERYSETW)));
    if (querySet == nullptr)
        return Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::OutOfMemory);

    std::wstring addressAsString;
    {
        std::string str_addr;
        BluetoothInternals::bth_ntop(reinterpret_cast<const bluetooth_sockaddr*>(remote_addr.GetAddr()), str_addr);
        addressAsString = Internals::StringToWChar(str_addr);
    }

    memset(querySet, 0, sizeof(*querySet));
    querySet->dwSize = sizeof(*querySet);
    querySet->lpServiceClassId = &protocol;
    querySet->dwNameSpace = NS_BTH;
    querySet->lpszContext = &addressAsString[0];

    result = WSALookupServiceBeginW(querySet, flags, &hLookup);
    if (result != ERROR_SUCCESS)
    {
        error = Internals::LastError();
        if (error.ErrorCode == NetworkLibrary::Error::WsaNotInitialised)
        {
            error = Internals::WinSockInitializer::Initialize(0x0202);
            result = WSALookupServiceBeginW(querySet, flags, &hLookup);
        }
    }

    if (result == ERROR_SUCCESS)
    {
        DWORD bufferLength = sizeof(WSAQUERYSETW);
        WSAQUERYSETW* pResults = reinterpret_cast<WSAQUERYSETW*>(malloc(bufferLength));
        if (pResults != nullptr)
        {
            while (1)
            {
                result = WSALookupServiceNextW(hLookup, flags, &bufferLength, pResults);
                // Buffer too small
                if (result != ERROR_SUCCESS && WSAGetLastError() == WSAEFAULT && bufferLength > 0)
                {
                    free(pResults);
                    bufferLength <<= 1; // Multiply by 2
                    pResults = reinterpret_cast<WSAQUERYSETW*>(malloc(bufferLength));
                    if (pResults != nullptr)
                    {
                        result = WSALookupServiceNextW(hLookup, flags, &bufferLength, pResults);
                    }
                }

                if (result != ERROR_SUCCESS)
                {
                    error = Internals::LastError();
                    break;
                }

                if (pResults->lpBlob)
                {
                    const BLOB* pBlob = (BLOB*)pResults->lpBlob;

                    SDP_ELEMENT_DATA elem;

                    if (BluetoothSdpGetAttributeValue(pBlob->pBlobData, pBlob->cbSize, SDP_ATTRIB_SERVICE_ID, &elem) == ERROR_SUCCESS)
                    {
                        if (elem.type == SDP_TYPE_UUID)
                        {
                            NetworkLibrary::Bluetooth::UUID sdp_uuid;

                            switch (elem.specificType)
                            {
                                case SDP_ST_UUID16 : sdp_uuid.SetUUID16(elem.data.uuid16); break;
                                case SDP_ST_UUID32 : sdp_uuid.SetUUID32(elem.data.uuid32); break;
                                case SDP_ST_UUID128: sdp_uuid.SetUUID128(*(NetworkLibrary::Bluetooth::BluetoothUUID_t*)&elem.data.uuid128); break;
                            }

                            if (sdp_uuid == uuid)
                            {
                                SDP_ELEMENT_DATA protocol_container;

                                // Open the decriptor list sequence
                                if (BluetoothSdpGetAttributeValue(pBlob->pBlobData, pBlob->cbSize, SDP_ATTRIB_PROTOCOL_DESCRIPTOR_LIST, &protocol_container) == ERROR_SUCCESS)
                                {
                                    out_channel = sdp_get_proto_port(protocol_container, RFCOMM_PROTOCOL_UUID16);
                                    if (out_channel != 0xff)
                                        error = Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NoError);

                                    break;
                                }

                                // If you need theses, just uncomment these lines & use the values
                                //if (BluetoothSdpGetAttributeValue(pBlob->pBlobData, pBlob->cbSize, 0x0100, &elem) == ERROR_SUCCESS)
                                //{
                                //    if (elem.type == SDP_TYPE_STRING)// Service Name
                                //    {
                                //        (char*)elem.data.string.value;
                                //        (size_t)elem.data.string.length);
                                //    }
                                //}
                                //if (BluetoothSdpGetAttributeValue(pBlob->pBlobData, pBlob->cbSize, 0x0101, &elem) == ERROR_SUCCESS)
                                //{
                                //    if (elem.type == SDP_TYPE_STRING)// Service Provider
                                //    {
                                //        (char*)elem.data.string.value;
                                //        (size_t)elem.data.string.length);
                                //    }
                                //}
                                //if (BluetoothSdpGetAttributeValue(pBlob->pBlobData, pBlob->cbSize, 0x0102, &elem) == ERROR_SUCCESS)
                                //{
                                //    if (elem.type == SDP_TYPE_STRING)// Service Description
                                //    {
                                //        (char*)elem.data.string.value;
                                //        (size_t)elem.data.string.length);
                                //    }
                                //}
                            }
                        }
                    }
                }
            }
        }
        if (pResults == nullptr)
        {
            error = Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::OutOfMemory);
            free(pResults);
        }

        WSALookupServiceEnd(hLookup);
    }

    free(querySet);
    return error;
}

SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) RegisterSDPServiceRFCOMM(SDPServiceHandle* out_handle, NetworkLibrary::Bluetooth::UUID const& uuid, uint8_t port, std::string const& srv_name, std::string const& srv_prov, std::string const& srv_desc)
{
    if(out_handle == nullptr)
        return Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::InVal);

    NetworkLibrary::Error error;

    WSAQUERYSETW* result     = (WSAQUERYSETW*)malloc(sizeof(WSAQUERYSETW));
    GUID* private_uuid       = nullptr;
    bluetooth_sockaddr* addr = nullptr;
    CSADDR_INFO* csAddr      = nullptr;

    if (result != nullptr)
    {
        private_uuid = (GUID*)malloc(sizeof(GUID));
        addr = (bluetooth_sockaddr*)malloc(sizeof(bluetooth_sockaddr));
        csAddr = (CSADDR_INFO*)malloc(sizeof(CSADDR_INFO));

        memset(result, 0, sizeof(*result));
        std::wstring wstr;
        {
            wstr = Internals::StringToWChar(srv_name);
            size_t str_size = sizeof(WCHAR) * (wstr.length() + 1);
            WCHAR* lpwstr = (WCHAR*)malloc(str_size);
            result->lpszServiceInstanceName = lpwstr;
            if (lpwstr != nullptr)
            {
                memcpy(lpwstr, wstr.c_str(), str_size);
            }
        }
        {
            wstr = Internals::StringToWChar(srv_desc);
            size_t str_size = sizeof(WCHAR) * (wstr.length() + 1);
            WCHAR* lpwstr = (WCHAR*)malloc(str_size);
            result->lpszComment = lpwstr;
            if (lpwstr != nullptr)
            {
                memcpy(lpwstr, wstr.c_str(), str_size);
            }
        }
    }

    if (result == nullptr ||
        addr == nullptr ||
        csAddr == nullptr ||
        private_uuid == nullptr ||
        result->lpszServiceInstanceName == nullptr ||
        result->lpszComment == nullptr)
    {
        goto ONERROR;
    }

    memset(addr, 0, sizeof(*addr));
    memset(csAddr, 0, sizeof(CSADDR_INFO));
    *private_uuid = UUIDToNative(uuid);

    result->dwSize = sizeof(*result);

    result->lpServiceClassId = private_uuid;
    result->dwNumberOfCsAddrs = 1;
    result->dwNameSpace = NS_BTH;
    
    addr->addressFamily = _AddressFamily;
    addr->port = port;
    
    csAddr->LocalAddr.iSockaddrLength = sizeof(*addr);
    csAddr->LocalAddr.lpSockaddr = reinterpret_cast<sockaddr*>(addr);
    csAddr->iSocketType = SOCK_STREAM;
    csAddr->iProtocol = BTHPROTO_RFCOMM;

    result->lpcsaBuffer = csAddr;
    
    if (WSASetServiceW(result, RNRSERVICE_REGISTER, 0) == 0)
    {
        *out_handle = result;
        return Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NoError);
    }

    error = Internals::LastError();
    if (error.ErrorCode == NetworkLibrary::Error::WsaNotInitialised)
    {
        Internals::WinSockInitializer::Initialize(0x0202);
        if (WSASetServiceW(result, RNRSERVICE_REGISTER, 0) == 0)
        {
            *out_handle = result;
            return Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NoError);
        }
        error = Internals::LastError();
    }

    return error;

ONERROR:
    if (result != nullptr)
    {
        if (result->lpszServiceInstanceName != nullptr)
            free(result->lpszServiceInstanceName);

        if (result->lpszComment != nullptr)
            free(result->lpszComment);

        free(result);
    }
    if (private_uuid != nullptr)
        free(private_uuid);

    if (addr != nullptr)
        free(addr);

    if (csAddr != nullptr)
        free(csAddr);

    return Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::OutOfMemory);
}

SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) UnregisterSDPServiceRFCOMM(SDPServiceHandle handle)
{
    if(handle == nullptr)
        return Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::InVal);

    NetworkLibrary::Error error;

    if (WSASetServiceW(handle, RNRSERVICE_DELETE, 0) == 0)
    {
        goto EXIT_NOERROR;
    }

    error = Internals::LastError();
    if (error.ErrorCode == NetworkLibrary::Error::WsaNotInitialised)
    {
        error = Internals::WinSockInitializer::Initialize(0x0202);
        if (error.ErrorCode != NetworkLibrary::Error::NoError)
            return error;

        if (WSASetServiceW(handle, RNRSERVICE_DELETE, 0) == 0)
        {
            goto EXIT_NOERROR;
        }
        error = Internals::LastError();
    }
    return error;

EXIT_NOERROR:
    free(handle->lpcsaBuffer->LocalAddr.lpSockaddr);
    free(handle->lpcsaBuffer);
    free(handle->lpServiceClassId);
    free(handle->lpszServiceInstanceName);
    free(handle->lpszComment);
    free(handle);
    return Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NoError);
}

#elif defined(UTILS_OS_LINUX)
SOCKET_HIDE_SYMBOLS(NetworkLibrary::Error) GetChannelFromServiceUUID(NetworkLibrary::Bluetooth::UUID const& uuid, BasicAddr const& remote_addr, uint8_t& out_channel)
{
    bdaddr_t any_bth_addr{};
    bdaddr_t remote_sdp_addr;
    sdp_session_t* session = 0;
    sdp_list_t* response_list, * search_list, * attrid_list;
    uint32_t range = 0x0000FFFF;
    uuid_t native_uuid = UUIDToNative(uuid);
    NetworkLibrary::Error error = Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NotFound);

    remote_sdp_addr = reinterpret_cast<bluetooth_sockaddr const*>(remote_addr.GetAddr())->BTH_ADDR;

    session = sdp_connect(&any_bth_addr, &remote_sdp_addr, 0);
    if (session == nullptr)
    {
        return Internals::LastError();
    }

    search_list = sdp_list_append(0, &native_uuid);
    attrid_list = sdp_list_append(0, &range);

    if (sdp_service_search_attr_req(session, search_list, SDP_ATTR_REQ_RANGE, attrid_list, &response_list) == 0)
    {
        sdp_list_t* proto_list;
        sdp_list_t* r = response_list;

        for (; r; r = r->next)
        {
            sdp_record_t* rec = (sdp_record_t*)r->data;
            if (sdp_get_access_protos(rec, &proto_list) == 0)
            {
                out_channel = sdp_get_proto_port(proto_list, RFCOMM_UUID);
                sdp_list_free(proto_list, 0);
                error = Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NoError);
            }

            sdp_record_free(rec);
        }
    }

    sdp_list_free(response_list, 0);
    sdp_list_free(search_list, 0);
    sdp_list_free(attrid_list, 0);

    sdp_close(session);

    return error;
}

#if defined(USE_BLUEZ_COMPAT)

SOCKET_HIDE_SYMBOLS(std::vector<NetworkLibrary::Bluetooth::BluetoothDevice>) ScanDevices(bool flushCache)
{
    std::vector<NetworkLibrary::Bluetooth::BluetoothDevice> devices;
    NetworkLibrary::Bluetooth::BluetoothDevice device;

    inquiry_info* iinfo;
    int dev_id, sock;
    int devices_count;
    char name[255];

    // Get the first available adapter id.
    dev_id = hci_get_route(nullptr);
    if (dev_id < 0)
        return devices;

    // Open adapter
    sock = hci_open_dev(dev_id);
    if (sock < 0)
        return devices;

    // allocation d'autant inquiry_info que d'appareils potentiellement trouvable, ici 255 appareils maxi
    iinfo = (inquiry_info*)malloc(sizeof(inquiry_info) * 255);
    if (iinfo == nullptr)
    {
        hci_close_dev(sock);
        return devices;
    }

    // hci_inquiry
    // Usage : Start bluetooth scan

    // int hci_inquiry(int dev_id, int len, int max_rsp, const uint8_t *lap, inquiry_info **ii, long flags);
    // dev_id : adapter id
    // len     : 1.28 * len = seconds for inquiry. Should be between 5 and 15 secs.
    //             1.28 * 8 = 10.24s.
    // max_rsp: max answers (iinfo size)
    // lap     : restart count (?)
    // ii      : structure de requ�te
    // flags  : IREQ_CACHE_FLUSH = flush cache
    // Returns the number of devices found.
    devices_count = hci_inquiry(dev_id, 8, 255, nullptr, &iinfo, flushCache ? IREQ_CACHE_FLUSH : 0);
    if (devices_count > 0)
    {
        for (int i = 0; i < devices_count; ++i)
        {
            if (hci_read_remote_name(sock, &iinfo[i].bdaddr, 255, name, 0) < 0)
                strcpy(name, "[unknown]");

            device.Name = name;
            device.Address = bluetoothmac_to_string(*reinterpret_cast<const NetworkLibrary::Bluetooth::BluetoothMac*>(&iinfo[i].bdaddr));

            devices.emplace_back(device);
        }
    }

    free(iinfo);
    hci_close_dev(sock);
    return devices;
}

SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) RegisterSDPServiceRFCOMM(SDPServiceHandle* out_handle, NetworkLibrary::Bluetooth::UUID const& uuid, uint8_t port, std::string const& srv_name, std::string const& srv_prov, std::string const& srv_desc)
{
    NetworkLibrary::Error error;
    bdaddr_t addr_any{};
    bdaddr_t addr_local{ { 0, 0, 0, 0xFF, 0xFF, 0xFF } };
    uuid_t native_uuid = UUIDToNative(uuid);

    sdp_session_t* sdp_session;

    // Start SDP session
    sdp_session = sdp_connect(&addr_any, &addr_local, SDP_RETRY_IF_BUSY);
    if (sdp_session == nullptr)
        return Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::Fault);
    
    // UUIDS
    uuid_t root_uuid, l2cap_uuid, rfcomm_uuid;
    
    sdp_list_t* l2cap_list = 0,
        *rfcomm_list = 0,
        *root_list = 0,
        *proto_list = 0,
        *access_proto_list = 0;
    
    sdp_data_t* channel = 0;
    // Create record
    sdp_record_t* record = sdp_record_alloc();
    
    // Set UUID
    sdp_set_service_id(record, native_uuid);
    
    sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
    root_list = sdp_list_append(0, &root_uuid);
    sdp_set_browse_groups(record, root_list);
    
    sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
    l2cap_list = sdp_list_append(0, &l2cap_uuid);
    proto_list = sdp_list_append(0, l2cap_list);
    
    sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
    channel = sdp_data_alloc(SDP_UINT8, &port);
    rfcomm_list = sdp_list_append(0, &rfcomm_uuid);
    sdp_list_append(rfcomm_list, channel);
    sdp_list_append(proto_list, rfcomm_list);
    
    access_proto_list = sdp_list_append(0, proto_list);
    sdp_set_access_protos(record, access_proto_list);
    sdp_set_info_attr(record, srv_name.c_str(), srv_prov.c_str(), srv_desc.c_str());
    
    if (sdp_record_register(sdp_session, record, 0))
    {
        *out_handle = nullptr;
        sdp_close(sdp_session);
        error = Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::Fault);
    }
    else
    {
        *out_handle = sdp_session;
        error = Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NoError);
    }
    
    sdp_data_free(channel);
    sdp_list_free(l2cap_list, 0);
    sdp_list_free(rfcomm_list, 0);
    sdp_list_free(root_list, 0);
    sdp_list_free(access_proto_list, 0);
    sdp_record_free(record);

    return error;
}

SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) UnregisterSDPServiceRFCOMM(SDPServiceHandle handle)
{
    if (handle == nullptr)
        return Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::InVal);

    sdp_close(handle);
    return Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NoError);
}

#else

SOCKET_HIDE_SYMBOLS(std::vector<NetworkLibrary::Bluetooth::BluetoothDevice>) ScanDevices(bool flushCache)
{
    (void)flushCache;

    std::string adapter_path;
    std::vector<NetworkLibrary::Bluetooth::BluetoothDevice> devices;
    NetworkLibrary::Bluetooth::BluetoothDevice device;
    bool found_device;

    DBusMessageIter managed_objects_iterator;

    if (BluezDBusSystem::Inst().Initialize().ErrorCode != NetworkLibrary::Error::NoError)
        return devices;

    found_device = [&managed_objects_iterator = managed_objects_iterator, &adapter_path = adapter_path]() -> bool {
        DBusMessage* msg, *ret;
        char* string_value;
        char* type_value;

        DBusMessageIter Dict1;
        DBusMessageIter Dict2;
        DBusMessageIter DictEntry1;
        DBusMessageIter DictEntry2;

        if ((msg = dbus_message_new_method_call("org.bluez", "/", "org.freedesktop.DBus.ObjectManager", "GetManagedObjects")) == nullptr)
        {
            return false;
        }

        if ((ret = dbus_connection_send_with_reply_and_block(BluezDBusSystem::Inst(), msg, DBUS_TIMEOUT_INFINITE, BluezDBusSystem::Inst())) == nullptr)
        {
            return false;
        }

        dbus_message_iter_init(ret, &managed_objects_iterator);
        // a{oa{sa{sv}}}
        // Foreach array
        for (int type; (type = dbus_message_iter_get_arg_type(&managed_objects_iterator)) != DBUS_TYPE_INVALID; dbus_message_iter_next(&managed_objects_iterator))
        {
            // Enter Array
            dbus_message_iter_recurse(&managed_objects_iterator, &Dict1);

            // Foreach Dictionnary Object Array
            for (int type; (type = dbus_message_iter_get_arg_type(&Dict1)) != DBUS_TYPE_INVALID; dbus_message_iter_next(&Dict1))
            {
                // Enter Dictionnary
                dbus_message_iter_recurse(&Dict1, &DictEntry1);
                // Foreach array
                for (int type; (type = dbus_message_iter_get_arg_type(&DictEntry1)) != DBUS_TYPE_INVALID; dbus_message_iter_next(&DictEntry1))
                {
                    // Get Dictionnary entry key
                    dbus_message_iter_get_basic(&DictEntry1, &string_value);
                    // Next value
                    dbus_message_iter_next(&DictEntry1);
                    // Enter ditionnary value
                    dbus_message_iter_recurse(&DictEntry1, &Dict2);
                    // Foreach Array
                    for (int type; (type = dbus_message_iter_get_arg_type(&Dict2)) != DBUS_TYPE_INVALID; dbus_message_iter_next(&Dict2))
                    {
                        // Enter ditionnary
                        dbus_message_iter_recurse(&Dict2, &DictEntry2);
                        // Foreach Dictionnary String Value
                        for (int type; (type = dbus_message_iter_get_arg_type(&DictEntry2)) != DBUS_TYPE_INVALID; dbus_message_iter_next(&DictEntry2))
                        {
                            // Get Dictionnary entry key
                            dbus_message_iter_get_basic(&DictEntry2, &type_value);
                            // Next value
                            dbus_message_iter_next(&DictEntry2);
                            if (!strcmp(type_value, "org.bluez.Adapter1"))
                            {
                                adapter_path = string_value;
                                return true;
                            }
                        }
                    }
                }
            }
        }

        return false;
    }();

    if (!found_device)
        return devices;

    [&managed_objects_iterator = managed_objects_iterator, &adapter_path = adapter_path, &devices = devices]() -> bool {
        char* string_value;
        DBusMessage* msg, *ret;
        bool found_device;
        NetworkLibrary::Bluetooth::BluetoothDevice device;
        DBusMessageIter Dict1;
        DBusMessageIter DictEntry1;
        DBusMessageIter Dict2;
        DBusMessageIter DictEntry2;
        DBusMessageIter Dict3;
        DBusMessageIter DictEntry3;

        if ((msg = dbus_message_new_method_call("org.bluez", adapter_path.c_str(), "org.bluez.Adapter1", "StartDiscovery")) == nullptr)
        {
            return false;
        }

        if ((ret = dbus_connection_send_with_reply_and_block(BluezDBusSystem::Inst(), msg, DBUS_TIMEOUT_INFINITE, BluezDBusSystem::Inst())) == nullptr)
        {
            return false;
        }

        // Wait for 10 seconds for the scan to detect something.
        std::this_thread::sleep_for(std::chrono::seconds(10));

        // Pour chaque valeur du tableau de dictionnaire
        for (int type; (type = dbus_message_iter_get_arg_type(&managed_objects_iterator)) != DBUS_TYPE_INVALID; dbus_message_iter_next(&managed_objects_iterator))
        {
            // On r�cup�re la valeur du tableau (par exemple a[0], a[1], etc...)
            dbus_message_iter_recurse(&managed_objects_iterator, &Dict1);
            // Pour chaque valeur du tableau
            for (int type; (type = dbus_message_iter_get_arg_type(&Dict1)) != DBUS_TYPE_INVALID; dbus_message_iter_next(&Dict1))
            {
                dbus_message_iter_recurse(&Dict1, &DictEntry1);

                // Adapter path
                for (int type; (type = dbus_message_iter_get_arg_type(&DictEntry1)) != DBUS_TYPE_INVALID; dbus_message_iter_next(&DictEntry1))
                {
                    // Get Dictionnary key
                    dbus_message_iter_get_basic(&DictEntry1, &string_value);
                    dbus_message_iter_next(&DictEntry1);
                    
                    dbus_message_iter_recurse(&DictEntry1, &Dict2);
                    for (int type; (type = dbus_message_iter_get_arg_type(&Dict2)) != DBUS_TYPE_INVALID; dbus_message_iter_next(&Dict2))
                    {
                        // On rentre dans le dictionnaire
                        dbus_message_iter_recurse(&Dict2, &DictEntry2);
                        // Pour chaque entr�e du dictionnaire (� une string on associe un tableau)
                        for (int type; (type = dbus_message_iter_get_arg_type(&DictEntry2)) != DBUS_TYPE_INVALID; dbus_message_iter_next(&DictEntry2))
                        {
                            // Get Dictionnary key
                            dbus_message_iter_get_basic(&DictEntry2, &string_value);
                            dbus_message_iter_next(&DictEntry2);
                            
                            dbus_message_iter_recurse(&DictEntry2, &Dict3);
                            if (!strcmp(string_value, "org.bluez.Device1"))
                            {
                                device.Address.clear();
                                device.Name.clear();
                                found_device = false;

                                dbus_message_iter_recurse(&DictEntry2, &Dict3);
                                for (int type; (type = dbus_message_iter_get_arg_type(&Dict3)) != DBUS_TYPE_INVALID; dbus_message_iter_next(&Dict3))
                                {
                                    dbus_message_iter_recurse(&Dict3, &DictEntry3);
                                    dbus_message_iter_get_basic(&DictEntry3, &string_value);
                                    dbus_message_iter_next(&DictEntry3);
                                    if (!strcmp(string_value, "Address"))
                                    {
                                        DBusMessageIter variant;
                                        dbus_message_iter_recurse(&DictEntry3, &variant);
                                        dbus_message_iter_get_basic(&variant, &string_value);
                                        device.Address = string_value;
                                        found_device = true;

                                    }
                                    else if (!strcmp(string_value, "Name"))
                                    {
                                        DBusMessageIter variant;
                                        dbus_message_iter_recurse(&DictEntry3, &variant);
                                        dbus_message_iter_get_basic(&variant, &string_value);
                                        device.Name = string_value;
                                        found_device = true;
                                    }
                                }
                                if (found_device)
                                {
                                    if (device.Name.empty())
                                    {
                                        device.Name = "[unknown]";
                                    }
                                    devices.emplace_back(std::move(device));
                                }
                            }
                        }
                    }
                }
            }
        }

        // If this fail, I don't know what to do about this.
        if ((msg = dbus_message_new_method_call("org.bluez", adapter_path.c_str(), "org.bluez.Adapter1", "StopDiscovery")) != nullptr)
        {
            dbus_connection_send_with_reply_and_block(BluezDBusSystem::Inst(), msg, DBUS_TIMEOUT_INFINITE, BluezDBusSystem::Inst());
        }
    }();

    return devices;
}

constexpr uint32_t hash(const char* datas, size_t len)
{
    uint32_t h = 0xffffffff;

    while (len--)
    {
        h ^= *datas++;
        for (int i = 7; i >= 0; --i)
        {
            h = (h >> 1) ^ (0x82F63B78ul & -static_cast<int32_t>(h & 1));
        }
    }

    return ~h;
}

SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) RegisterSDPServiceRFCOMM(SDPServiceHandle* out_handle, NetworkLibrary::Bluetooth::UUID const& uuid, uint8_t port, std::string const& srv_name, std::string const& srv_prov, std::string const& srv_desc)
{
    if (out_handle == nullptr)
        return Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::InVal);

    BluezDBusSystem& bus = BluezDBusSystem::Inst();
    DBusMessage* msg, * ret;
    DBusMessageIter iter, dict, entry, variant;
    std::string sdp_session;
    std::string sdp_record;
    sdp_session += srv_name;
    std::string str_uuid;
    const char* record_key;
    const char* string_value;
    std::stringstream sstr;

    if (BluezDBusSystem::Inst().Initialize().ErrorCode != NetworkLibrary::Error::NoError)
    {
        return Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::Fault);
    }

    if ((msg = dbus_message_new_method_call("org.bluez", "/org/bluez", "org.bluez.ProfileManager1", "RegisterProfile")) == nullptr)
    {
        return Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::Fault);
    }

    sdp_session = "/NetworkLibrary/" + std::to_string(hash(srv_name.c_str(), srv_name.length()));

    str_uuid = uuid.ToString();
    record_key = "ServiceRecord";

    sstr << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << std::endl
         << "<record>" << std::endl
         << "    <attribute id=\"0x0003\">" << std::endl // SDP_ATTRIB_SERVICE_ID
         << "        <uuid value=\"" << str_uuid << "\" />" << std::endl
         << "    </attribute>" << std::endl
         << "    <attribute id=\"0x0004\">" << std::endl // SDP_ATTRIB_PROTOCOL_DESCRIPTOR_LIST
         << "        <sequence>" << std::endl
         << "            <sequence>" << std::endl
         << "                <uuid value=\"0x0100\" />" << std::endl
         << "            </sequence>" << std::endl
         << "            <sequence>" << std::endl
         << "                <uuid value=\"0x0003\" />" << std::endl // RFCOMM_PROTOCOL_UUID16 == 0x0003
         << "                <uint8 value=\"0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint16_t>(port) << std::dec << "\" />" << std::endl
         << "            </sequence>" << std::endl
         << "        </sequence>" << std::endl
         << "    </attribute>" << std::endl
         << "    <attribute id=\"0x0005\">" << std::endl // SDP_ATTRIB_BROWSE_GROUP_LIST
         << "        <sequence>" << std::endl
         << "            <uuid value=\"0x1002\" />" << std::endl
         << "        </sequence>" << std::endl
         << "    </attribute>" << std::endl
         << "    <attribute id=\"0x0100\">" << std::endl
         << "        <text value=\"" << srv_name << "\" />" << std::endl
         << "    </attribute>" << std::endl
         << "    <attribute id=\"0x0101\">" << std::endl
         << "        <text value=\"" << srv_prov << "\" />" << std::endl
         << "    </attribute>" << std::endl
         << "    <attribute id=\"0x0102\">" << std::endl
         << "        <text value=\"" << srv_desc << "\" />" << std::endl
         << "    </attribute>" << std::endl
         << "</record>";
    
    sdp_record = sstr.str();

    dbus_message_iter_init_append(msg, &iter);    
    string_value = sdp_session.c_str();
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_OBJECT_PATH, &string_value);
    string_value = str_uuid.c_str();
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &string_value);

    dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY,
        DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
        DBUS_TYPE_STRING_AS_STRING
        DBUS_TYPE_VARIANT_AS_STRING
        DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
        &dict);
    
    
    dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, 0, &entry);
    
    dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &record_key);
    
    dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, DBUS_TYPE_STRING_AS_STRING, &variant);
    
    string_value = sdp_record.c_str();
    dbus_message_iter_append_basic(&variant, DBUS_TYPE_STRING, &string_value);
    
    dbus_message_iter_close_container(&entry, &variant);
    
    dbus_message_iter_close_container(&dict, &entry);
    
    dbus_message_iter_close_container(&iter, &dict);

    if ((ret = dbus_connection_send_with_reply_and_block(bus, msg, DBUS_TIMEOUT_INFINITE, bus)) == nullptr)
    {
        return Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::Fault);
    }

    *out_handle = new std::string(std::move(sdp_session));

    return Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NoError);
}

SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) UnregisterSDPServiceRFCOMM(SDPServiceHandle handle)
{
    if(handle == nullptr)
        return Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::InVal);

    BluezDBusSystem& bus = BluezDBusSystem::Inst();
    DBusMessage* msg, * ret;
    DBusMessageIter iter;
    const char* string_value = handle->c_str();

    if (BluezDBusSystem::Inst().Initialize().ErrorCode != NetworkLibrary::Error::NoError)
    {
        return Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::Fault);
    }

    if ((msg = dbus_message_new_method_call("org.bluez", "/org/bluez", "org.bluez.ProfileManager1", "UnregisterProfile")) == nullptr)
    {
        return Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::Fault);
    }
    
    dbus_message_iter_init_append(msg, &iter);
    
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_OBJECT_PATH, &string_value);
    if ((ret = dbus_connection_send_with_reply_and_block(bus, msg, DBUS_TIMEOUT_INFINITE, bus)) == nullptr)
    {
        return Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::Fault);
    }

    return Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NoError);
}

#endif

#endif

}
}