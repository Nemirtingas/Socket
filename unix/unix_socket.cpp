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

#include <Socket/unix/unix_socket.h>

using namespace PortableAPI;

unix_socket::unix_socket(){}

unix_socket::unix_socket(unix_socket const& other):connected_socket(other), _addr(other._addr)
{}

unix_socket::unix_socket(unix_socket &&other):connected_socket(std::move(other)), _addr(std::move(other._addr))
{}

unix_socket::unix_socket(SOCKET s) : connected_socket(s)
{}

unix_socket::~unix_socket()
{
}

unix_socket & unix_socket::operator=(unix_socket const &other)
{
    _addr = other._addr;
    basic_socket::operator=(other);
    return *this;
}

unix_socket & unix_socket::operator=(unix_socket &&other)
{
    _addr = std::move(other._addr);
    basic_socket::operator=(std::move(other));
    return *this;
}

unix_socket::myaddr const & unix_socket::get_addr() const
{
    return _addr;
}

unix_socket unix_socket::accept()
{
    myaddr addr;

    unix_socket res(Socket::accept(*_sock, addr));
    res._addr = addr;
    return res;
}

void unix_socket::server(myaddr & addr, int waiting_connections)
{
    socket();
    connected_socket::bind(addr);
    connected_socket::listen(waiting_connections);
    _addr = addr;
}

void unix_socket::connect(myaddr & addr)
{
    socket();
    connected_socket::connect(addr);
    _addr = addr;
}

void unix_socket::socket()
{
    basic_socket::socket(Socket::address_family::unix, Socket::types::stream, static_cast<Socket::protocols>(0));
}

void unix_socket::close()
{
    Socket::shutdown(*_sock, Socket::shutdown_flags::both);
    Socket::closeSocket(*_sock);
}
