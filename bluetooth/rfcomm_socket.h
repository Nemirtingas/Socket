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

#ifndef _RFCOMM_SOCKET_INCLUDED_
#define _RFCOMM_SOCKET_INCLUDED_

#include <Socket/bluetooth/rfcomm_addr.h>
#include <Socket/common/basic_socket.h>

namespace PortableAPI
{
    // Connect by uuid:
    //#if defined(__WINDOWS__)
    //    server_addr.from_string("00:11:22:33:44:55");
    //    server_addr.set_channel(0);
    //    server_addr.get_native_addr().serviceClassId = uuid;
    //    clisock.connect(server_addr);
    //#elif defined(__LINUX__)
    //    server_addr.from_string("00:11:22:33:44:55");
    //    server_addr.set_channel(BluetoothSocket::scanOpenPortFromUUID(uuid, server_addr.get_ip()));
    //#endif

    // Register SDP service:
    //PortableAPI::Socket::InitSocket();
    //PortableAPI::rfcomm_socket rfsock;
    //PortableAPI::rfcomm_addr addr;
    //PortableAPI::SDPService service;
    //
    //addr.from_string("00:11:22:33:44:55");
    //int port; // Port 0 does bind but is invalid, start at 1
    //for (port = 1; port < 32; ++port)
    //{
    //    addr.set_channel(port);
    //    try
    //    {
    //        rfsock.bind(addr);
    //        break;
    //    }
    //    catch (...)
    //    {
    //    }
    //}
    //if (port != 32)
    //{
    //    auto uuid = PortableAPI::BluetoothSocket::str2uuid(_BTHUUID_);
    //    service.registerService(uuid, port, "test", "toto", "titi");
    //}

    using rfcomm_socket = connected_socket<rfcomm_addr,
        static_cast<Socket::address_family>(BluetoothSocket::address_family::bth),
        Socket::types::stream,
        static_cast<Socket::protocols>(BluetoothSocket::protocols::rfcomm)
    >;
}

#endif
