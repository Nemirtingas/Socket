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

#ifndef _TCP_SOCKET_INCLUDED_
#define _TCP_SOCKET_INCLUDED_

#include <Socket/ipv4/ipv4_addr.h>

namespace PortableAPI
{
	class LOCAL_API tcp_socket : public connected_socket
	{
		public:
			typedef ipv4_addr myaddr;

		private:
			myaddr _addr;

			tcp_socket(SOCKET s);

			void socket();
		public:
			tcp_socket();
			tcp_socket(tcp_socket const&);
			tcp_socket(tcp_socket &&);
			virtual ~tcp_socket();

			tcp_socket& operator =(tcp_socket const&);
			tcp_socket& operator =(tcp_socket &&);

			myaddr const& get_addr() const;
			tcp_socket accept();
			void server(myaddr& addr, int waiting_connections = 5);
			void connect(myaddr& addr);
			virtual void close();
	};
}
#endif
