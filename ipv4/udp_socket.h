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

#ifndef _UDP_SOCKET_INCLUDED_
#define _UDP_SOCKET_INCLUDED_

#include <Socket/ipv4/ipv4_addr.h>

namespace PortableAPI
{
    class LOCAL_API udp_socket : public unconnected_socket
    {
        public:
            typedef ipv4_addr myaddr;

        private:
            myaddr _addr;

            udp_socket(Socket::socket_t s);

            void socket();
        public:
            udp_socket();
            udp_socket(udp_socket const&);
            udp_socket(udp_socket &&);
            virtual ~udp_socket();

            udp_socket& operator =(udp_socket const&);
            udp_socket& operator =(udp_socket &&);

            myaddr const& get_addr() const;
            void bind(myaddr& addr);
            virtual void close();
    };
}
#endif
