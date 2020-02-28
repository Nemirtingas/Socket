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

#include <ipv6/ipv6_addr.h>

using namespace PortableAPI;

constexpr in6_addr ipv6_addr::any_addr;
constexpr in6_addr ipv6_addr::loopback_addr;

ipv6_addr::ipv6_addr() :
    _sockaddr(new my_sockaddr)
{
    memset(_sockaddr, 0, len());
    _sockaddr->sin6_family = static_cast<uint16_t>(Socket::address_family::inet6);
}

ipv6_addr::ipv6_addr(ipv6_addr const &other) :
    _sockaddr(new my_sockaddr)
{
    memcpy(_sockaddr, other._sockaddr, len());
}

ipv6_addr::ipv6_addr(ipv6_addr &&other) noexcept :
    _sockaddr(nullptr)
{
    std::swap(_sockaddr, other._sockaddr);
}

ipv6_addr & ipv6_addr::operator=(ipv6_addr const &other)
{
    memcpy(_sockaddr, other._sockaddr, len());
    return *this;
}

ipv6_addr & ipv6_addr::operator=(ipv6_addr &&other) noexcept
{
    std::swap(_sockaddr, other._sockaddr);
    return *this;
}

ipv6_addr::~ipv6_addr()
{
    delete _sockaddr;
}

std::string ipv6_addr::to_string(bool with_port) const
{
    std::string res;
    Socket::inet_ntop(Socket::address_family::inet6, &_sockaddr->sin6_addr, res);
    if (with_port)
    {
        res.insert(res.begin(), '[');
        res.append("]:");
        res += std::to_string(Socket::net_swap(_sockaddr->sin6_port));
    }

    return res;
}

void ipv6_addr::from_string(std::string const & str)
{
    size_t pos = str.find(']');
    size_t pos_port = str.rfind(':');

    if (pos_port > pos)
        pos = pos_port;
    else
        pos = std::string::npos;

    if (pos != std::string::npos)
    {
        std::string ip = str.substr(0, pos);
        std::string port = str.substr(pos + 1);
        Socket::inet_pton(Socket::address_family::inet, ip, &_sockaddr->sin6_addr);
        set_port(stoi(port));
    }
    else
    {
        Socket::inet_pton(Socket::address_family::inet, str, &_sockaddr->sin6_addr);
    }
}

sockaddr & ipv6_addr::addr()
{
    return *reinterpret_cast<sockaddr*>(_sockaddr);
}

sockaddr const& ipv6_addr::addr() const
{
    return *reinterpret_cast<sockaddr*>(_sockaddr);
}

size_t ipv6_addr::len() const
{
    return sizeof(sockaddr_in);
}

void ipv6_addr::set_addr(in6_addr const& addr)
{
    _sockaddr->sin6_addr = Socket::net_swap(addr);
}

void ipv6_addr::set_port(uint16_t port)
{
    _sockaddr->sin6_port = Socket::net_swap(port);
}

in6_addr ipv6_addr::get_addr() const
{
    return Socket::net_swap(_sockaddr->sin6_addr);
}

uint16_t ipv6_addr::get_port() const
{
    return Socket::net_swap(_sockaddr->sin6_port);
}

ipv6_addr::my_sockaddr& ipv6_addr::get_native_addr()
{
    return *_sockaddr;
}
