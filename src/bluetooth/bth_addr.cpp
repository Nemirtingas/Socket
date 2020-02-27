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

#include <bluetooth/bth_addr.h>

using namespace PortableAPI;

constexpr bdaddr_t bth_addr::any_addr;

bth_addr::bth_addr() :
    _sockaddr(new my_sockaddr)
{
    memset(_sockaddr, 0, len());
#if defined(__WINDOWS__)
    _sockaddr->addressFamily = static_cast<uint16_t>(BluetoothSocket::address_family::bth);
#elif defined(__LINUX__)
    _sockaddr->rc_family = static_cast<uint8_t>(BluetoothSocket::address_family::bth);
#endif
}

bth_addr::bth_addr(bth_addr const &other) :
    _sockaddr(new my_sockaddr)
{
    memcpy(_sockaddr, other._sockaddr, len());
}

bth_addr::bth_addr(bth_addr &&other) noexcept:
    _sockaddr(nullptr)
{
    std::swap(_sockaddr, other._sockaddr);
}

bth_addr & bth_addr::operator=(bth_addr const &other)
{
    memcpy(_sockaddr, other._sockaddr, len());
    return *this;
}

bth_addr & bth_addr::operator=(bth_addr &&other) noexcept
{
    std::swap(_sockaddr, other._sockaddr);
    return *this;
}

bth_addr::~bth_addr()
{
    delete _sockaddr;
}

std::string bth_addr::to_string(bool with_port) const
{
    std::string res;

#if defined(__WINDOWS__)
    BluetoothSocket::inet_ntop(BluetoothSocket::address_family::bth, &_sockaddr->btAddr, res);
#elif defined(__LINUX__)
    BluetoothSocket::inet_ntop(BluetoothSocket::address_family::bth, &_sockaddr->rc_bdaddr, res);
#endif

    if (with_port)
    {
        res.push_back('@');
#if defined(__WINDOWS__)
        res += std::to_string(_sockaddr->port);
#elif defined(__LINUX__)
        res += std::to_string(_sockaddr->rc_channel);
#endif
    }

    return res;
}

void bth_addr::from_string(std::string const & str)
{
    size_t pos = str.find('@');

    if (pos != std::string::npos)
    {
        std::string ip = str.substr(0, pos);
        std::string channel = str.substr(pos + 1);
#if defined(__WINDOWS__)
        BluetoothSocket::inet_pton(BluetoothSocket::address_family::bth, ip, &_sockaddr->btAddr);
#elif defined(__LINUX__)
        BluetoothSocket::inet_pton(BluetoothSocket::address_family::bth, ip, &_sockaddr->rc_bdaddr);
#endif
        set_channel(stoi(channel));
    }
    else
    {
#if defined(__WINDOWS__)
        BluetoothSocket::inet_pton(BluetoothSocket::address_family::bth, str, &_sockaddr->btAddr);
#elif defined(__LINUX__)
        BluetoothSocket::inet_pton(BluetoothSocket::address_family::bth, str, &_sockaddr->rc_bdaddr);
#endif
    }
}

sockaddr & bth_addr::addr()
{
    return *reinterpret_cast<sockaddr*>(_sockaddr);
}

sockaddr const& bth_addr::addr() const
{
    return *reinterpret_cast<sockaddr*>(_sockaddr);
}

size_t bth_addr::len() const
{
    return sizeof(my_sockaddr);
}

void bth_addr::set_addr(bdaddr_t const& addr)
{
#if defined(__WINDOWS__)
    _sockaddr->btAddr = addr;
#elif defined(__LINUX__)
    _sockaddr->rc_bdaddr = addr;
#endif
}

void bth_addr::set_channel(uint8_t channel)
{
#if defined(__WINDOWS__)
    _sockaddr->port = channel;
#elif defined(__LINUX__)
    _sockaddr->rc_channel = channel;
#endif
}

bdaddr_t bth_addr::get_addr() const
{
#if defined(__WINDOWS__)
    return _sockaddr->btAddr;
#elif defined(__LINUX__)
    return _sockaddr->rc_bdaddr;
#endif
}

uint8_t bth_addr::get_channel() const
{
#if defined(__WINDOWS__)
    return static_cast<uint8_t>(_sockaddr->port);
#elif defined(__LINUX__)
    return _sockaddr->rc_channel;
#endif
}

bth_addr::my_sockaddr& bth_addr::get_native_addr()
{
    return *_sockaddr;
}
