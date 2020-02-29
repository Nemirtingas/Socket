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

#ifndef _IPV4_ADDR_INCLUDED_
#define _IPV4_ADDR_INCLUDED_

#include <common/basic_socket.h>

namespace PortableAPI
{
    ////////////
    /// @brief An IPV4 sock_addr object
    ////////////
    class EXPORT_SOCKET_API ipv4_addr : public basic_addr
    {
        public:
            using my_sockaddr = sockaddr_in;
#if defined(__WINDOWS__)
            constexpr static in_addr any_addr       = { 0x00, 0x00, 0x00, 0x00 };
            constexpr static in_addr loopback_addr  = { 0x01, 0x00, 0x00, 0x7f };
            constexpr static in_addr broadcast_addr = { 0xff, 0xff, 0xff, 0xff };
#elif defined(__LINUX__) || defined(__APPLE__)
            constexpr static in_addr any_addr       = { INADDR_ANY       };
            constexpr static in_addr loopback_addr  = { INADDR_LOOPBACK  };
            constexpr static in_addr broadcast_addr = { INADDR_BROADCAST };
#endif

        private:
            my_sockaddr *_sockaddr;

        public:
            ipv4_addr();
            ipv4_addr(ipv4_addr const&);
            ipv4_addr(ipv4_addr &&) noexcept;
            ipv4_addr& operator =(ipv4_addr const&);
            ipv4_addr& operator =(ipv4_addr &&) noexcept;

            virtual ~ipv4_addr();
            ////////////
            /// @brief Transforms the address to a human readable string
            /// @param[in] with_port Append the port
            /// @return Address formated like <ip>[:<port>]
            ////////////
            virtual std::string to_string(bool with_port = false) const;
            ////////////
            /// @brief Transforms the human readable string into an address
            /// @param[in] str Pass in a formated string like <ip>[:<port>]
            /// @return 
            ////////////
            virtual void from_string(std::string const& str);
            ////////////
            /// @brief Gets the generic sockaddr ref
            /// @return The sockaddr ref
            ////////////
            virtual sockaddr& addr();
            ////////////
            /// @brief Gets the generic const sockaddr ref
            /// @return The const sockaddr ref
            ////////////
            virtual sockaddr const& addr() const;
            ////////////
            /// @brief Get the sockaddr size
            /// @return sockaddr size
            ////////////
            virtual size_t len() const;
            ////////////
            /// @brief Sets the IPV4 ip
            /// @param[in]  addr The IPV4 ip
            /// @return 
            ////////////
            void set_ip(uint32_t ip);
            ////////////
            /// @brief Sets the IPV4 addr
            /// @param[in]  addr The IPV4 addr
            /// @return 
            ////////////
            void set_addr(in_addr const& addr);
            ////////////
            /// @brief Sets the IPV4 port
            /// @param[in]  port The IPV4 port
            /// @return 
            ////////////
            void set_port(uint16_t port);
            ////////////
            /// @brief Gets the IPV4 ip
            /// @return The IPV4 ip
            ////////////
            uint32_t get_ip() const;
            ////////////
            /// @brief Gets the IPV4 addr
            /// @return The IPV4 addr
            ////////////
            in_addr get_addr() const;
            ////////////
            /// @brief Gets the IPV4 port
            /// @return The IPV4 port
            ////////////
            uint16_t get_port() const;
            ////////////
            /// @brief Gets the native addr structure
            /// @return Native structure
            ////////////
            my_sockaddr& get_native_addr();
    };
}
#endif
