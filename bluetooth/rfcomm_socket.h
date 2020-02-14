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

#ifndef _RFCOMM_SOCKET_INCLUDED_
#define _RFCOMM_SOCKET_INCLUDED_

#include <Socket/common/basic_socket.h>
#include <Socket/bluetooth/rfcomm_addr.h>

namespace PortableAPI
{
	class LOCAL_API rfcomm_socket : public connected_socket
	{
		public:
			typedef rfcomm_addr myaddr;

		private:
			myaddr _addr;

			rfcomm_socket(SOCKET s);

			void socket();
		public:
			rfcomm_socket();
			rfcomm_socket(rfcomm_socket const&);
			rfcomm_socket(rfcomm_socket &&);
			virtual ~rfcomm_socket();

			rfcomm_socket& operator =(rfcomm_socket const&);
			rfcomm_socket& operator =(rfcomm_socket &&);

			rfcomm_addr const& get_addr() const;
			rfcomm_socket accept();
			void server(rfcomm_addr& addr, int waiting_connections = 5);
			SDPService server(rfcomm_addr& addr, uuid_t uuid, std::string name, std::string prov, std::string desc, int waiting_connections = 5);
			void connect(rfcomm_addr& addr);
			void connect(rfcomm_addr& addr, uuid_t uuid);
			virtual void close();
	};
}

#endif
