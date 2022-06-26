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
namespace IPv6 {

////////////
/// @brief Gets all IPv6 interfaces.
/// @return A list of IfaceInfos.
////////////
std::pair<NetworkLibrary::Error, std::vector<IfaceInfos>> GetIfacesAddresses();

struct InAddr6
{
    uint8_t bytes[16];
};

class IPv6Addr :
    public BasicAddr
{
    class IPv6AddrImpl* _Impl;

public:
    ////////////
    /// @brief 
    ////////////
    IPv6Addr();
    ////////////
    /// @brief 
    ////////////
    IPv6Addr(IPv6Addr const& other);
    ////////////
    /// @brief 
    ////////////
    IPv6Addr(IPv6Addr&& other) noexcept;
    ////////////
    /// @brief 
    ////////////
    IPv6Addr& operator=(IPv6Addr const& other);
    ////////////
    /// @brief 
    ////////////
    IPv6Addr& operator=(IPv6Addr&& other) noexcept;

    ////////////
    /// @brief 
    ////////////
    virtual ~IPv6Addr();
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
    /// @brief Fill this IPv6Addr from string representation.
    /// @param[in] str The string IPv6 representation.
    /// @return Error code.
    ////////////
    NetworkLibrary::Error FromString(std::string str);
    ////////////
    /// @brief Set this IPv6Addr ip.
    /// @param[in] ip The IPv6 ip.
    /// @return
    ////////////
    void SetIPv6(InAddr6 ip);
    ////////////
    /// @brief Set this IPv6Addr port.
    /// @param[in] port The host ordered IPv6 port.
    /// @return
    ////////////
    void SetPort(uint16_t port);
    ////////////
    /// @brief Get this IPv6Addr host ordered ip.
    /// @return Host ordered ip.
    ////////////
    InAddr6 GetIPv6() const;
    ////////////
    /// @brief Get this IPv6Addr host ordered port.
    /// @return Host ordered port.
    ////////////
    uint16_t GetPort() const;
    ////////////
    /// @brief Set this IPv6Addr anyaddr ip.
    /// @return
    ////////////
    void SetAnyAddr();
    ////////////
    /// @brief Set this IPv6Addr loopback ip.
    /// @return
    ////////////
    void SetLoopbackAddr();
};

class TCP :
    public ConnectedSocket
{
public:
    TCP();
    TCP(TCP const& other) = delete;
    TCP(TCP&& other) noexcept;
    TCP& operator=(TCP const& other) = delete;
    TCP& operator=(TCP&& other) noexcept;

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
    NetworkLibrary::Error GetSockName(IPv6Addr& out_addr);

    virtual int GetFamily() const;
    virtual int GetType() const;
    virtual int GetProto() const;
};

class UDP :
    public UnconnectedSocket
{
public:
    UDP();
    UDP(UDP const& other) = delete;
    UDP(UDP&& other) noexcept;
    UDP& operator=(UDP const& other) = delete;
    UDP& operator=(UDP&& other) noexcept;

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
    NetworkLibrary::Error GetSockName(IPv6Addr& out_addr);

    virtual int GetFamily() const;
    virtual int GetType() const;
    virtual int GetProto() const;
};

}
}