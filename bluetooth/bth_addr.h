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

#ifndef _BTH_ADDR_INCLUDED_
#define _BTH_ADDR_INCLUDED_

#include <Socket/bluetooth/bluetooth_socket.h>

namespace PortableAPI
{
    class LOCAL_API bth_addr : public basic_addr
    {
        public:
            typedef sockaddr_rc my_sockaddr;

        private:
            my_sockaddr *_sockaddr;

        public:
            bth_addr();
            bth_addr(bth_addr const&);
            bth_addr(bth_addr &&) noexcept;
            bth_addr& operator =(bth_addr const&);
            bth_addr& operator =(bth_addr &&) noexcept;

            virtual ~bth_addr();
            // Returns addr formated like <addr>[@<channel>]
            virtual std::string to_string(bool with_port = false) const;
            // Pass in a formated std::string like <addr>[@<channel>]
            virtual void from_string(std::string const& str);
            virtual sockaddr& addr();
            virtual sockaddr const& addr() const;
            virtual size_t len() const;
            virtual void set_any_addr();
            void set_ip(bdaddr_t const&);
            void set_channel(uint8_t channel);
            bdaddr_t get_ip() const;
            uint8_t get_channel() const;
            my_sockaddr& get_native_addr();
    };
}
#endif