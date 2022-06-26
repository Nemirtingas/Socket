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
namespace Unix {

class UnixAddr :
    public BasicAddr
{
    class UnixAddrImpl* _Impl;

public:
    ////////////
    /// @brief 
    ////////////
    UnixAddr();
    ////////////
    /// @brief 
    ////////////
    UnixAddr(UnixAddr const& other);
    ////////////
    /// @brief 
    ////////////
    UnixAddr(UnixAddr&& other) noexcept;
    ////////////
    /// @brief 
    ////////////
    UnixAddr& operator=(UnixAddr const& other);
    ////////////
    /// @brief 
    ////////////
    UnixAddr& operator=(UnixAddr&& other) noexcept;

    ////////////
    /// @brief 
    ////////////
    virtual ~UnixAddr();
    ////////////
    /// @brief Transforms the address to a human readable string
    /// @param[in] with_port Append the port
    /// @return The string representation of the address
    ////////////
    virtual std::string ToString(bool with_port = false) const;
    ////////////
    /// @brief Get this Addr family type.
    /// @return 
    ////////////
    virtual int GetFamily() const;
    ////////////
    /// @brief Get this Addr sockaddr.
    /// @return Reference
    ////////////
    virtual void* GetAddr();
    ////////////
    /// @brief Get this Addr sockaddr.
    /// @return Const reference
    ////////////
    virtual const void* GetAddr() const;
    ////////////
    /// @brief Get this Addr length.
    /// @return Addr length
    ////////////
    virtual size_t GetLength() const;

    ////////////
    /// @brief Fill this UnixAddr from string representation.
    /// @param[in] str The string path representation.
    /// @return Error code.
    ////////////
    NetworkLibrary::Error FromString(std::string str);
};

class UnixDgram:
    public UnconnectedSocket
{
    std::string* _BoundAddress; // Used for filesystem cleanup.

    void UnixCleanup();

public:
    UnixDgram();
    UnixDgram(UnixDgram const& other) = delete;
    UnixDgram(UnixDgram&& other) noexcept;
    UnixDgram& operator=(UnixDgram const& other) = delete;
    UnixDgram& operator=(UnixDgram&& other) noexcept;
    virtual ~UnixDgram();

    ////////////
    /// @brief Allocates resources to use network functions.
    /// @return Error
    ////////////
    NetworkLibrary::Error CreateSocket();
    ////////////
    /// @brief Gets this socket addr (if any).
    /// @param[out] out_addr Socket address
    /// @return Error
    ////////////
    NetworkLibrary::Error GetSockName(UnixAddr& out_addr);

    virtual int GetFamily() const;
    virtual int GetType  () const;
    virtual int GetProto () const;

    ////////////
    /// @brief Bind the socket on an Unix Address. It will try to remove the socket file on destructor or CreateSocket.
    /// @param[in] addr The address to bind on.
    /// @return Error code
    ////////////
    virtual NetworkLibrary::Error Bind(BasicAddr const& addr);
};

class UnixStream :
    public ConnectedSocket
{
    std::string* _BoundAddress; // Used for filesystem cleanup.

    void UnixCleanup();

public:
    UnixStream();
    UnixStream(UnixStream const& other) = delete;
    UnixStream(UnixStream&& other) noexcept;
    UnixStream& operator=(UnixStream const& other) = delete;
    UnixStream& operator=(UnixStream&& other) noexcept;
    virtual ~UnixStream();

    ////////////
    /// @brief Allocates resources to use network functions.
    /// @return Error
    ////////////
    NetworkLibrary::Error CreateSocket();
    ////////////
    /// @brief Gets this socket addr (if any).
    /// @param[out] out_addr Socket address
    /// @return Error
    ////////////
    NetworkLibrary::Error GetSockName(UnixAddr& out_addr);

    virtual int GetFamily() const;
    virtual int GetType() const;
    virtual int GetProto() const;

    ////////////
    /// @brief Bind the socket on an Unix Address. It will try to remove the socket file on destructor or CreateSocket.
    /// @param[in] addr The address to bind on.
    /// @return Error code
    ////////////
    virtual NetworkLibrary::Error Bind(BasicAddr const& addr);
};

}
}
