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

#include <Socket/common/basic_socket.h>

namespace PortableAPI
{
    class LOCAL_API ipv4_addr : public basic_addr
    {
        public:
            typedef sockaddr_in my_sockaddr;
            constexpr static uint32_t any_addr       = INADDR_ANY;
            constexpr static uint32_t loopback_addr  = INADDR_LOOPBACK;
            constexpr static uint32_t broadcast_addr = INADDR_BROADCAST;

        private:
            my_sockaddr *_sockaddr;

        public:
            ipv4_addr();
            ipv4_addr(ipv4_addr const&);
            ipv4_addr(ipv4_addr &&) noexcept;
            ipv4_addr& operator =(ipv4_addr const&);
            ipv4_addr& operator =(ipv4_addr &&) noexcept;

            virtual ~ipv4_addr();
            // Returns addr formated like <ip>:<port>
            virtual std::string to_string() const;
            // Pass in a formated std::string like <ip>[:<port>]
            virtual void from_string(std::string const& str);
            virtual sockaddr& addr();
            virtual sockaddr const& addr() const;
            virtual size_t len() const;
            virtual void set_any_addr();
            void set_ip(uint32_t);
            void set_port(uint16_t);
            uint32_t get_ip() const;
            uint16_t get_port() const;
            my_sockaddr& get_native_addr();
    };
}
#endif
