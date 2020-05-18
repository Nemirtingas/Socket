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

#pragma once

#include "socket.h"
#include <map>

namespace PortableAPI
{
    class EXPORT_SOCKET_API basic_socket;
    ////////////
    /// @brief A class to manage a socket poll. Can handle a poll with index or with socket address
    ////////////
    class Poll
    {
        std::map<Socket::socket_t, size_t> _sockets;
        std::vector<pollfd> _polls;

    public:
        ////////////
        /// @brief Adds a socket into the poll
        /// @param[in] sock The socket to add
        /// @return 
        ////////////
        void add_socket(basic_socket const& sock);
        ////////////
        /// @brief Removes a socket from the poll
        /// @param[in] sock The socket to remove
        /// @return 
        ////////////
        void remove_socket(basic_socket const& ock);
        ////////////
        /// @brief Removes a socket from the poll
        /// @param[in] i The socket index to remove, better check with get_num_polls before.
        /// @return 
        ////////////
        void remove_socket(size_t i);
        ////////////
        /// @brief Get the number of sockets currently in the poll
        /// @return Number of sockets in the poll
        ////////////
        size_t get_num_polls() const;
        ////////////
        /// @brief Set a poll event for the socket
        /// @param[in] sock The socket to change the flags to
        /// @param[in] flags new event flags
        /// @return 
        ////////////
        void set_events(basic_socket const& sock, Socket::poll_flags flags);
        ////////////
        /// @brief Set a poll event for the socket
        /// @param[in] i The socket index to change the flags to, better check with get_num_polls before.
        /// @param[in] flags new event flags
        /// @return 
        ////////////
        void set_events(size_t i, Socket::poll_flags flags);
        ////////////
        /// @brief Get a poll revents for the socket
        /// @param[in] sock The socket to get the flags from
        /// @return The poll revents flags
        ////////////
        Socket::poll_flags get_revents(basic_socket const& sock) const;
        ////////////
        /// @brief Get a poll revents for the socket
        /// @param[in] sock The socket index to get the flags from, better check with get_num_polls before.
        /// @return The poll revents flags
        ////////////
        Socket::poll_flags get_revents(size_t i) const;
        ////////////
        /// @brief Start the socket poll
        /// @param[in] timeout_ms <0, block, 0 returns now, >0 The time in milliseconds to wait.
        /// @return The number of sockets that have revents
        ////////////
        int poll(int timeout_ms);
    };
}