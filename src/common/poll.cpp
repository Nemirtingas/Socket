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

using namespace PortableAPI;

void Poll::add_socket(basic_socket* sock)
{
    _sockets[sock] = _polls.size();

    pollfd pollinfos = {};
    pollinfos.fd = sock->get_native_socket();
    _polls.emplace_back(pollinfos);
}

void Poll::remove_socket(basic_socket* sock)
{
    auto it = _sockets.find(sock);
    auto pollit = _polls.begin();
    std::advance(pollit, it->second);
    _polls.erase(pollit);
    _sockets.erase(it);
}

void Poll::remove_socket(int i)
{
    auto it = _polls.begin();
    std::advance(it, i);
    _sockets.erase(std::find_if(_sockets.begin(), _sockets.end(), [i](std::pair<basic_socket const*, int> p)
    {
        return p.second == i;
    }));
    _polls.erase(it);
}

size_t Poll::get_num_polls() const
{
    return _polls.size();
}

void Poll::set_events(basic_socket* sock, Socket::poll_flags flags)
{
    _polls[_sockets.at(sock)].revents = static_cast<uint16_t>(flags);
}

void Poll::set_events(int i, Socket::poll_flags flags)
{
    _polls[i].revents = static_cast<uint16_t>(flags);
}

Socket::poll_flags Poll::get_revents(basic_socket* sock) const
{
    return static_cast<Socket::poll_flags>(_polls[_sockets.at(sock)].revents);
}

Socket::poll_flags Poll::get_revents(int i) const
{
    return static_cast<Socket::poll_flags>(_polls[i].revents);
}

int Poll::poll(int timeout_ms)
{
    return Socket::poll(_polls.data(), _polls.size(), timeout_ms);
}