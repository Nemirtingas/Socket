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

#include <Socket/unix/unix_addr.h>

using namespace PortableAPI;

unix_addr::unix_addr() :_sockaddr(new my_sockaddr)
{
    memset(_sockaddr, 0, len());
    _sockaddr->sun_family = static_cast<uint16_t>(Socket::address_family::unix);
}

unix_addr::unix_addr(unix_addr const &other) : _sockaddr(new my_sockaddr)
{
    memcpy(_sockaddr, other._sockaddr, len());
}

unix_addr::unix_addr(unix_addr &&other) : _sockaddr(nullptr)
{
    std::swap(_sockaddr, other._sockaddr);
}

unix_addr & unix_addr::operator=(unix_addr const &other)
{
    memcpy(_sockaddr, other._sockaddr, len());
    return *this;
}

unix_addr & unix_addr::operator=(unix_addr &&other)
{
    std::swap(_sockaddr, other._sockaddr);
    return *this;
}

unix_addr::~unix_addr()
{
    delete _sockaddr;
}

std::string unix_addr::toString() const
{
    return get_path();
}

void unix_addr::fromString(std::string const & str)
{
    set_path(str);
}

sockaddr & unix_addr::addr()
{
    return *reinterpret_cast<sockaddr*>(_sockaddr);
}

size_t unix_addr::len() const
{
    return sizeof(my_sockaddr);
}

void unix_addr::set_any_addr()
{
}

void unix_addr::set_path(std::string const& path)
{
    strncpy(_sockaddr->sun_path, path.c_str(), sizeof(_sockaddr->sun_path));
}

std::string unix_addr::get_path() const
{
    std::string path(_sockaddr->sun_path);
    return path;
}

unix_addr::my_sockaddr& unix_addr::getAddr()
{
    return *_sockaddr;
}
