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

#include "internal_socket.h"
#include <NetworkLibrary/Bluetooth.h>

#if defined(SOCKET_OS_WINDOWS)

#define INITGUID
#include <ws2bth.h>
#include <bluetoothapis.h>
#include <initguid.h>
#include <devguid.h>

#pragma comment(lib, "Bthprops.lib")

using bluetooth_sockaddr = SOCKADDR_BTH;
#define BTH_ADDR    btAddr
#define BTH_FAMILY  addressFamily
#define BTH_CHANNEL port

using SDPService_t = WSAQUERYSETW;

static GUID UUIDToNative(NetworkLibrary::Bluetooth::UUID const& uuid)
{
    GUID res;
    auto nuuid = uuid.GetUUID();
    memcpy(&res, &nuuid, sizeof(GUID));
    return res;
}

#elif defined(SOCKET_OS_LINUX)
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

using bluetooth_sockaddr = sockaddr_rc;
#define BTH_ADDR    rc_bdaddr
#define BTH_FAMILY  rc_family
#define BTH_CHANNEL rc_channel

#if defined(USE_BLUEZ_COMPAT)
using SDPService_t = sdp_session_t;
#else
using SDPService_t = std::string;
#endif

static uuid_t UUIDToNative(NetworkLibrary::Bluetooth::UUID const& uuid)
{
    uuid_t res;
    auto nuuid = uuid.GetUUID();
    memcpy(&res.value, &nuuid, sizeof(uuid_t::value));
    res.type = SDP_UUID128;
    return res;
}

#endif

using SDPServiceHandle = SDPService_t*;

namespace NetworkLibrary {
namespace BluetoothInternals {

SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) bth_pton(std::string const& str_addr, bluetooth_sockaddr* addr);
SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) bth_ntop(bluetooth_sockaddr const* addr, std::string& str_addr);
SOCKET_HIDE_SYMBOLS(std::vector<NetworkLibrary::Bluetooth::BluetoothDevice>) ScanDevices(bool flushCache);
SOCKET_HIDE_SYMBOLS(NetworkLibrary::Error) GetChannelFromServiceUUID(NetworkLibrary::Bluetooth::UUID const& uuid, NetworkLibrary::BasicAddr const& remote_addr, uint8_t& out_channel);
SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) RegisterSDPServiceRFCOMM(SDPServiceHandle* out_handle, NetworkLibrary::Bluetooth::UUID const& uuid, uint8_t port, std::string const& srv_name, std::string const& srv_prov, std::string const& srv_desc);
SOCKET_HIDE_SYMBOLS(::NetworkLibrary::Error) UnregisterSDPServiceRFCOMM(SDPServiceHandle handle);

}
}