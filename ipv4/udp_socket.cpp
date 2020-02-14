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

#include <Socket/ipv4/udp_socket.h>

using namespace PortableAPI;

udp_socket::udp_socket()
{
    socket();
}

udp_socket::udp_socket(udp_socket const& other):unconnected_socket(other), _addr(other._addr)
{}

udp_socket::udp_socket(udp_socket &&other):unconnected_socket(std::move(other)), _addr(std::move(other._addr))
{}

udp_socket::udp_socket(Socket::socket_t s) : unconnected_socket(s)
{}

udp_socket::~udp_socket()
{
}

udp_socket & udp_socket::operator=(udp_socket const &other)
{
    _addr = other._addr;
    basic_socket::operator=(other);
    return *this;
}

udp_socket & udp_socket::operator=(udp_socket &&other)
{
    _addr = std::move(other._addr);
    basic_socket::operator=(std::move(other));
    return *this;
}

udp_socket::myaddr const & udp_socket::get_addr() const
{
    return _addr;
}

void udp_socket::socket()
{
    basic_socket::socket(Socket::address_family::inet, Socket::types::dgram, Socket::protocols::udp);
}

void udp_socket::bind(udp_socket::myaddr &addr)
{
    unconnected_socket::bind(addr);
}

void udp_socket::close()
{
    Socket::closeSocket(*_sock);
    socket();
}
