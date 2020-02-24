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

#include <Socket/bluetooth/rfcomm_addr.h>

using namespace PortableAPI;

rfcomm_addr::rfcomm_addr() :
    _sockaddr(new my_sockaddr)
{
    memset(_sockaddr, 0, len());
#if defined(__WINDOWS__)
    _sockaddr->addressFamily = static_cast<uint16_t>(BluetoothSocket::address_family::bth);
#elif defined(__LINUX__)
    _sockaddr->rc_family = static_cast<uint8_t>(BluetoothSocket::address_family::bth);
#endif
}

rfcomm_addr::rfcomm_addr(rfcomm_addr const &other) :
    _sockaddr(new my_sockaddr)
{
    memcpy(_sockaddr, other._sockaddr, len());
}

rfcomm_addr::rfcomm_addr(rfcomm_addr &&other) noexcept:
    _sockaddr(nullptr)
{
    std::swap(_sockaddr, other._sockaddr);
}

rfcomm_addr & rfcomm_addr::operator=(rfcomm_addr const &other)
{
    memcpy(_sockaddr, other._sockaddr, len());
    return *this;
}

rfcomm_addr & rfcomm_addr::operator=(rfcomm_addr &&other) noexcept
{
    std::swap(_sockaddr, other._sockaddr);
    return *this;
}

rfcomm_addr::~rfcomm_addr()
{
    delete _sockaddr;
}

std::string rfcomm_addr::to_string() const
{
    std::string ip, port;
#if defined(__WINDOWS__)
    ip = BluetoothSocket::inet_ntoa(_sockaddr->btAddr);
    port = std::to_string(_sockaddr->port);
#elif defined(__LINUX__)
    ip = BluetoothSocket::inet_ntoa(_sockaddr->rc_bdaddr);
    port = std::to_string(_sockaddr->rc_channel);
#endif

    return ip + '@' + port;
}

void rfcomm_addr::from_string(std::string const & str)
{
    size_t pos;

    if ((pos = str.find('@')) != std::string::npos)
    {
        std::string ip = str.substr(0, pos);
        std::string channel = str.substr(pos + 1);
#if defined(__WINDOWS__)
        _sockaddr->btAddr = BluetoothSocket::inet_addr(ip);
#elif defined(__LINUX__)
        _sockaddr->rc_bdaddr = BluetoothSocket::inet_addr(ip);
#endif
        set_channel(stoi(channel));
    }
    else
    {
#if defined(__WINDOWS__)
        _sockaddr->btAddr = BluetoothSocket::inet_addr(str);
#elif defined(__LINUX__)
        _sockaddr->rc_bdaddr = BluetoothSocket::inet_addr(str);
#endif
    }
}

sockaddr & rfcomm_addr::addr()
{
    return *reinterpret_cast<sockaddr*>(_sockaddr);
}

sockaddr const& rfcomm_addr::addr() const
{
    return *reinterpret_cast<sockaddr*>(_sockaddr);
}

size_t rfcomm_addr::len() const
{
    return sizeof(my_sockaddr);
}

void rfcomm_addr::set_any_addr()
{
#if defined(__WINDOWS__)
    memset(&_sockaddr->btAddr, 0, sizeof(bdaddr_t));
#elif defined(__LINUX__)
    memset(&_sockaddr->rc_bdaddr, 0, sizeof(bdaddr_t));
#endif
}

void rfcomm_addr::set_ip(bdaddr_t const& ip)
{
#if defined(__WINDOWS__)
    _sockaddr->btAddr = ip;
#elif defined(__LINUX__)
    _sockaddr->rc_bdaddr = ip;
#endif
}

void rfcomm_addr::set_channel(uint8_t channel)
{
#if defined(__WINDOWS__)
    _sockaddr->port = channel;
#elif defined(__LINUX__)
    _sockaddr->rc_channel = channel;
#endif
}

bdaddr_t rfcomm_addr::get_ip() const
{
#if defined(__WINDOWS__)
    return _sockaddr->btAddr;
#elif defined(__LINUX__)
    return _sockaddr->rc_bdaddr;
#endif
}

uint8_t rfcomm_addr::get_channel() const
{
#if defined(__WINDOWS__)
    return static_cast<uint8_t>(_sockaddr->port);
#elif defined(__LINUX__)
    return _sockaddr->rc_channel;
#endif
}

rfcomm_addr::my_sockaddr& rfcomm_addr::get_native_addr()
{
    return *_sockaddr;
}
