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

#include <unix/unix_addr.h>

using namespace PortableAPI;

unix_addr::unix_addr() :
    _sockaddr(new my_sockaddr)
{
    memset(_sockaddr, 0, len());
    _sockaddr->sun_family = static_cast<uint16_t>(Socket::address_family::unix);
}

unix_addr::unix_addr(unix_addr const &other) :
    _sockaddr(new my_sockaddr)
{
    memcpy(_sockaddr, other._sockaddr, len());
}

unix_addr::unix_addr(unix_addr &&other) noexcept :
    _sockaddr(nullptr)
{
    std::swap(_sockaddr, other._sockaddr);
}

unix_addr & unix_addr::operator=(unix_addr const &other)
{
    memcpy(_sockaddr, other._sockaddr, len());
    return *this;
}

unix_addr & unix_addr::operator=(unix_addr &&other) noexcept
{
    std::swap(_sockaddr, other._sockaddr);
    return *this;
}

unix_addr::~unix_addr()
{
    delete _sockaddr;
}

std::string unix_addr::to_string(bool with_port) const
{
    return get_addr();
}

bool unix_addr::from_string(std::string const & str)
{
    set_addr(str);
    return true;
}

sockaddr & unix_addr::addr()
{
    return *reinterpret_cast<sockaddr*>(_sockaddr);
}

sockaddr const& unix_addr::addr() const
{
    return *reinterpret_cast<sockaddr*>(_sockaddr);
}

size_t unix_addr::len() const
{
    return sizeof(my_sockaddr);
}

void unix_addr::set_addr(std::string const& addr)
{
    addr.copy(_sockaddr->sun_path, UNIX_PATH_MAX);

    auto i = (addr.length() >= UNIX_PATH_MAX ? (UNIX_PATH_MAX - 1) : addr.length());
    _sockaddr->sun_path[i] = '\0';
}

std::string unix_addr::get_addr() const
{
    std::string path(_sockaddr->sun_path);
    return path;
}

unix_addr::my_sockaddr& unix_addr::get_native_addr()
{
    return *_sockaddr;
}