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

#ifndef _IPV6_ADDR_INCLUDED_
#define _IPV6_ADDR_INCLUDED_

#include "../common/basic_socket.h"

namespace PortableAPI
{
    ////////////
    /// @brief An IPV6 sock_addr object
    ////////////
    class EXPORT_SOCKET_API ipv6_addr : public basic_addr
    {
        public:
            using my_sockaddr = sockaddr_in6;
            constexpr static in6_addr any_addr      = IN6ADDR_ANY_INIT;
            constexpr static in6_addr loopback_addr = IN6ADDR_LOOPBACK_INIT;

        private:
            my_sockaddr *_sockaddr;

        public:
            ipv6_addr();
            ipv6_addr(ipv6_addr const&);
            ipv6_addr(ipv6_addr &&) noexcept;
            ipv6_addr& operator =(ipv6_addr const&);
            ipv6_addr& operator =(ipv6_addr &&) noexcept;

            virtual ~ipv6_addr();
            ////////////
            /// @brief Transforms the address to a human readable string
            /// @param[in] with_port Append the port
            /// @return Address formated like <ip> or [<ip>]:port
            ////////////
            virtual std::string to_string(bool with_port = false) const;
            ////////////
            /// @brief Transforms the human readable string into an address
            /// @param[in] str <ip> or [<ip>]:<port> (brackets needed)
            /// @return false failed to parse, true succeeded to parse
            ////////////
            virtual bool from_string(std::string const& str);
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
            /// @brief Sets the IPV6 addr
            /// @param[in]  addr The IPV6 addr
            /// @return 
            ////////////
            void set_addr(in6_addr const& addr);
            ////////////
            /// @brief Sets the IPV6 port
            /// @param[in]  port The IPV6 port
            /// @return 
            ////////////
            void set_port(uint16_t);
            ////////////
            /// @brief Gets the IPV6 addr
            /// @return The IPV6 addr
            ////////////
            in6_addr get_addr() const;
            ////////////
            /// @brief Gets the IPV6 port
            /// @return The IPV6 port
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
