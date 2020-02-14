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

#include <Socket/bluetooth/rfcomm_socket.h>

using namespace PortableAPI;

rfcomm_socket::rfcomm_socket()
{}

rfcomm_socket::rfcomm_socket(rfcomm_socket const& other):connected_socket(other), _addr(other._addr)
{}

rfcomm_socket::rfcomm_socket(rfcomm_socket &&other):connected_socket(std::move(other)), _addr(std::move(other._addr))
{}

rfcomm_socket::rfcomm_socket(SOCKET s):connected_socket(s)
{}

rfcomm_socket::~rfcomm_socket()
{}

rfcomm_socket & rfcomm_socket::operator=(rfcomm_socket const &other)
{
    _addr = other._addr;
    basic_socket::operator=(other);
    return *this;
}

rfcomm_socket & rfcomm_socket::operator=(rfcomm_socket &&other)
{
    _addr = std::move(other._addr);
    basic_socket::operator=(std::move(other));
    return *this;
}

rfcomm_addr const & rfcomm_socket::get_addr() const
{
    return _addr;
}

rfcomm_socket rfcomm_socket::accept()
{
    rfcomm_addr addr;
    rfcomm_socket res(Socket::accept(*_sock, addr));
    res._addr = addr;
    return res;
}

void rfcomm_socket::server(rfcomm_addr & addr, int waiting_connections)
{
    socket();
    connected_socket::bind(addr);
    connected_socket::listen(waiting_connections);
    _addr = addr;
}

SDPService rfcomm_socket::server(rfcomm_addr& addr, uuid_t uuid, std::string name, std::string prov, std::string desc, int waiting_connections)
{
    socket();
    for (int8_t channel = 1; channel < 32; ++channel)
    {
        try
        {
            addr.set_channel(channel);
            connected_socket::bind(addr);
            break;
        }
        catch (socket_exception &e)
        {
            (void)e;
            if( channel == 31 )
                throw socket_exception("Cannot find free rfcomm channel.");
        }
    }
    connected_socket::listen(waiting_connections);
    SDPService service;

    service.registerService(uuid, addr.get_channel(), name, prov, desc);

    _addr = addr;

    return service;
}

void rfcomm_socket::connect(rfcomm_addr & addr)
{
    socket();
#if defined(__WINDOWS__)
    memset(&addr.getAddr().serviceClassId, 0, sizeof(uuid_t));
#endif
    connected_socket::connect(addr);
    _addr = addr;
}

void rfcomm_socket::connect(rfcomm_addr & addr, uuid_t uuid)
{
    socket();
    addr.set_channel(0);
#if defined(__WINDOWS__)
    addr.getAddr().serviceClassId = uuid;
#elif defined(__LINUX__)
    bdaddr_t bdaddr = addr.get_ip();
    addr.set_channel(BluetoothSocket::scanOpenPortFromUUID(uuid, bdaddr));
#endif
    
    connected_socket::connect(addr);
    _addr = addr;
}

void rfcomm_socket::socket()
{
    basic_socket::socket(static_cast<Socket::address_family>(BluetoothSocket::address_family::bth), Socket::types::stream, static_cast<Socket::protocols>(BluetoothSocket::protocols::rfcomm));
}

void rfcomm_socket::close()
{
    Socket::shutdown(*_sock, Socket::shutdown_flags::both);
    Socket::closeSocket(*_sock);
}
