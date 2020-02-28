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

#include <ipv4/ipv4_addr.h>

#ifdef __WINDOWS__
  #ifndef s_addr
    #define s_addr S_un.S_addr
  #endif
#endif

using namespace PortableAPI;

constexpr in_addr ipv4_addr::any_addr;
constexpr in_addr ipv4_addr::loopback_addr;
constexpr in_addr ipv4_addr::broadcast_addr;

ipv4_addr::ipv4_addr() :
    _sockaddr(new my_sockaddr)
{
    memset(_sockaddr, 0, len());
    _sockaddr->sin_family = static_cast<uint16_t>(Socket::address_family::inet);
}

ipv4_addr::ipv4_addr(ipv4_addr const &other) :
    _sockaddr(new my_sockaddr)
{
    memcpy(_sockaddr, other._sockaddr, len());
}

ipv4_addr::ipv4_addr(ipv4_addr &&other) noexcept :
    _sockaddr(nullptr)
{
    std::swap(_sockaddr, other._sockaddr);
}

ipv4_addr & ipv4_addr::operator=(ipv4_addr const &other)
{
    memcpy(_sockaddr, other._sockaddr, len());
    return *this;
}

ipv4_addr & ipv4_addr::operator=(ipv4_addr &&other) noexcept
{
    std::swap(_sockaddr, other._sockaddr);
    return *this;
}

ipv4_addr::~ipv4_addr()
{
    delete _sockaddr;
}

std::string ipv4_addr::to_string(bool with_port) const
{
    std::string res;
    Socket::inet_ntop(Socket::address_family::inet, &_sockaddr->sin_addr, res);
    if (with_port)
    {
        res.push_back(':');
        res += std::to_string(Socket::net_swap(_sockaddr->sin_port));
    }

    return res;
}

void ipv4_addr::from_string(std::string const & str)
{
    size_t pos = str.find(':');

    if (pos != std::string::npos)
    {
        std::string ip = str.substr(0, pos);
        std::string port = str.substr(pos + 1);
        Socket::inet_pton(Socket::address_family::inet, ip, &_sockaddr->sin_addr);
        set_port(stoi(port));
    }
    else
    {
        Socket::inet_pton(Socket::address_family::inet, str, &_sockaddr->sin_addr);
    }
}

sockaddr & ipv4_addr::addr()
{
    return *reinterpret_cast<sockaddr*>(_sockaddr);
}

sockaddr const& ipv4_addr::addr() const
{
    return *reinterpret_cast<sockaddr*>(_sockaddr);
}

size_t ipv4_addr::len() const
{
    return sizeof(sockaddr_in);
}

void ipv4_addr::set_ip(uint32_t ip)
{
    _sockaddr->sin_addr.s_addr = Socket::net_swap(ip);
}

void ipv4_addr::set_addr(in_addr const& addr)
{
    set_ip(addr.s_addr);
}

void ipv4_addr::set_port(uint16_t port)
{
    _sockaddr->sin_port = Socket::net_swap(port);
}

uint32_t ipv4_addr::get_ip() const
{
    return Socket::net_swap(_sockaddr->sin_addr.s_addr);
}

in_addr ipv4_addr::get_addr() const
{
    in_addr res;
    res.s_addr = Socket::net_swap(_sockaddr->sin_addr.s_addr);
    return res;
}

uint16_t ipv4_addr::get_port() const
{
    return Socket::net_swap(_sockaddr->sin_port);
}

ipv4_addr::my_sockaddr& ipv4_addr::get_native_addr()
{
    return *_sockaddr;
}
