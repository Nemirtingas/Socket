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

#include <Socket/ipv4/tcp_socket.h>

using namespace PortableAPI;

tcp_socket::tcp_socket(){}

tcp_socket::tcp_socket(tcp_socket const& other):connected_socket(other), _addr(other._addr)
{}

tcp_socket::tcp_socket(tcp_socket &&other):connected_socket(std::move(other)), _addr(std::move(other._addr))
{}

tcp_socket::tcp_socket(SOCKET s) : connected_socket(s)
{}

tcp_socket::~tcp_socket()
{
}

tcp_socket & tcp_socket::operator=(tcp_socket const &other)
{
    _addr = other._addr;
    basic_socket::operator=(other);
    return *this;
}

tcp_socket & tcp_socket::operator=(tcp_socket &&other)
{
    _addr = std::move(other._addr);
    basic_socket::operator=(std::move(other));
    return *this;
}

tcp_socket::myaddr const & tcp_socket::get_addr() const
{
    return _addr;
}

tcp_socket tcp_socket::accept()
{
    ipv4_addr addr;

    tcp_socket res(Socket::accept(*_sock, addr));
    res._addr = addr;
    return res;
}

void tcp_socket::server(myaddr & addr, int waiting_connections)
{
    socket();
    connected_socket::bind(addr);
    connected_socket::listen(waiting_connections);
    _addr = addr;
}

void tcp_socket::connect(myaddr & addr)
{
    socket();
    connected_socket::connect(addr);
    _addr = addr;
}

void tcp_socket::socket()
{
    basic_socket::socket(Socket::address_family::inet, Socket::types::stream, Socket::protocols::tcp);
}

void tcp_socket::close()
{
    Socket::shutdown(*_sock, Socket::shutdown_flags::both);
    Socket::closeSocket(*_sock);
}
