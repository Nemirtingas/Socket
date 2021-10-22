/* Copyright (C) Nemirtingas
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

#include "details/Socket.h"

namespace NetworkLibrary {
    ////////////
    /// @brief A class to manage a socket poll. Can handle a poll with index or with socket address
    ////////////
    class Poll
    {
        class PollImpl* _Impl;

    public:
        Poll();
        Poll(Poll const& other);
        Poll(Poll && other) noexcept;

        Poll& operator=(Poll const& other);
        Poll& operator=(Poll && other) noexcept;

        ////////////
        /// @brief Get the number of sockets currently in the poll
        /// @return Number of sockets in the poll
        ////////////
        size_t GetSocketCount() const;
        ////////////
        /// @brief Adds a socket into the poll
        /// @param[in] sock The socket to add
        /// @param[in] flags new event flags
        /// @return Error
        ////////////
        NetworkLibrary::Error AddSocket(BasicSocket const& sock, /* PollFlags */ int16_t flags);
        ////////////
        /// @brief Removes a socket from the poll
        /// @param[in] sock The socket to remove
        /// @return Error
        ////////////
        NetworkLibrary::Error RemoveSocket(BasicSocket const& sock);
        ////////////
        /// @brief Removes a socket from the poll
        /// @param[in] index The socket to remove
        /// @return Error
        ////////////
        NetworkLibrary::Error RemoveSocket(size_t index);
        ////////////
        /// @brief Set a poll event for the socket
        /// @param[in] sock The socket to change the flags to
        /// @param[in] flags new event flags
        /// @return Error
        ////////////
        NetworkLibrary::Error SetEvents(BasicSocket const& sock, /* PollFlags */ int16_t flags);
        ////////////
        /// @brief Set a poll event for the socket
        /// @param[in] index The socket to change the flags to
        /// @param[in] flags new event flags
        /// @return Error
        ////////////
        NetworkLibrary::Error SetEvents(size_t index, /* PollFlags */ int16_t flags);
        ////////////
        /// @brief Get a poll revents for the socket
        /// @param[in] sock The socket to get the flags from
        /// @return The poll revents flags, returns also none is sock is not in the poll
        ////////////
        /* PollFlags */ int16_t GetRevents(BasicSocket const& sock);
        ////////////
        /// @brief Get a poll revents for the socket
        /// @param[in] index The socket to get the flags from
        /// @return The poll revents flags, returns also none is sock is not in the poll
        ////////////
        /* PollFlags */ int16_t GetRevents(size_t index);
        ////////////
        /// @brief Start the socket poll
        /// @param[in] timeout <0 = block, 0 = returns now, >0 = The time in milliseconds to wait.
        /// @return The number of sockets that have revents
        ////////////
        int32_t DoPoll(std::chrono::milliseconds timeout);
        ////////////
        /// @brief Clear the poll of its sockets
        /// @return 
        ////////////
        void Clear();
    };
}