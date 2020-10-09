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
        res += "]:";
        res += std::to_string(utils::Endian::net_swap(_sockaddr->sin6_port));
    }

    return res;
}

bool ipv6_addr::from_string(std::string const& str)
{
    bool success = false;
    addrinfo* info = nullptr;
    addrinfo hints = {};
    hints.ai_family = (int)PortableAPI::Socket::address_family::inet6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;

    size_t sep_pos = 0;
    std::string ip;
    int sep_count = 0;
    for (int i = 0; i < str.length(); ++i)
    {
        if (str[i] == ':')
        {
            sep_pos = i;
            ++sep_count;
        }
    }
    if (sep_count == 8)
    {
        ip = std::move(std::string(str.begin(), str.begin() + sep_pos));
    }
    else
    {
        ip = str;
    }

    if (PortableAPI::Socket::getaddrinfo(ip.c_str(), nullptr, &hints, &info) == 0)
    {
        my_sockaddr* maddr = (my_sockaddr*)info->ai_addr;

        size_t pos = str.find(']');
        std::string str_port("0");

        if (pos != std::string::npos)
        {
            str_port = std::move(std::string(str.begin() + pos + 2, str.end()));
        }
        else if (sep_count == 8)
        {
            str_port = std::move(std::string(str.begin() + sep_pos + 1, str.end()));
        }

        try
        {
            int port = std::stoi(str_port);
            if (port >= 0 && port <= 65535)
            {
                set_addr(maddr->sin6_addr);
                set_port(port);

                success = true;
            }
        }
        catch(...)
        { }
    }

    if (info != nullptr)
        PortableAPI::Socket::freeaddrinfo(info);

    return success;
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

void ipv6_addr::set_ip(in6_addr const& addr)
{
    _sockaddr->sin6_addr = addr;
}

void ipv6_addr::set_addr(in6_addr const& addr)
{
    _sockaddr->sin6_addr = addr;
}

void ipv6_addr::set_port(uint16_t port)
{
    _sockaddr->sin6_port = utils::Endian::net_swap(port);
}

in6_addr ipv6_addr::get_ip() const
{
    return _sockaddr->sin6_addr;
}

in6_addr ipv6_addr::get_addr() const
{
    return _sockaddr->sin6_addr;
}

uint16_t ipv6_addr::get_port() const
{
    return utils::Endian::net_swap(_sockaddr->sin6_port);
}

ipv6_addr::my_sockaddr& ipv6_addr::get_native_addr()
{
    return *_sockaddr;
}
