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

#include <common/poll.h>
#include <common/basic_socket.h>
#include <algorithm>

using namespace PortableAPI;

void Poll::add_socket(basic_socket const& sock)
{
    Socket::socket_t native_sock = sock.get_native_socket();
    auto it = _sockets.find(native_sock);
    if (it == _sockets.end())
    {
        _sockets[native_sock] = _polls.size();

        pollfd pollinfos = {};
        pollinfos.fd = native_sock;
        _polls.emplace_back(pollinfos);
    }
}

void Poll::remove_socket(basic_socket const& sock)
{
    auto it = _sockets.find(sock.get_native_socket());
    if (it != _sockets.end())
    {
        auto pollit = _polls.begin();
        size_t i = it->second;
        std::advance(pollit, it->second);
        _polls.erase(pollit);
        _sockets.erase(it);
        for (auto& sock : _sockets)
        {
            if (sock.second >= i)
                --sock.second;
        }
    }
}

void Poll::remove_socket(size_t i)
{
    if (i < _polls.size())
    {
        auto it = _polls.begin();
        std::advance(it, i);
        _sockets.erase(std::find_if(_sockets.begin(), _sockets.end(), [i](std::pair<Socket::socket_t const, int> p)
        {
            return p.second == i;
        }));
        _polls.erase(it);
        for (auto& sock : _sockets)
        {
            if (sock.second >= i)
                --sock.second;
        }
    }
}

size_t Poll::get_num_polls() const
{
    return _polls.size();
}

void Poll::set_events(basic_socket const& sock, Socket::poll_flags flags)
{
    _polls[_sockets.at(sock.get_native_socket())].events = static_cast<uint16_t>(flags);
}

void Poll::set_events(size_t i, Socket::poll_flags flags)
{
    _polls[i].events = static_cast<uint16_t>(flags);
}

Socket::poll_flags Poll::get_revents(basic_socket const& sock) const
{
    return static_cast<Socket::poll_flags>(_polls[_sockets.at(sock.get_native_socket())].revents);
}

Socket::poll_flags Poll::get_revents(size_t i) const
{
    return static_cast<Socket::poll_flags>(_polls[i].revents);
}

int Poll::poll(int timeout_ms)
{
    return Socket::poll(_polls.data(), _polls.size(), timeout_ms);
}

void Poll::clear()
{
    _polls.clear();
    _sockets.clear();
}