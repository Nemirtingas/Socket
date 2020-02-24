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

#pragma once

#include <Socket/common/socket.h>

#include <memory>

namespace PortableAPI
{
    class LOCAL_API basic_socket
    {
        public:
            virtual ~basic_socket() = default;

            ////////////////////////////////////////////////////////////////////////////////
            // M�thode : ioctlsocket
            // Usage   : Permet de modifier les param�tres d'un socket (non bloquant etc...)
            // param�tres entrants : Socket::cmd_name cmd, unsigned long* arg
            // param�tres sortants : void
            ////////////////////////////////////////////////////////////////////////////////
            void ioctlsocket(Socket::cmd_name cmd, unsigned long* arg);
            ////////////////////////////////////////////////////////////////////////////////
            // M�thode : setsockopt
            // Usage   : Permet de modifier les options d'un socket (broadcast etc...)
            // param�tres entrants : Socket::level level, Socket::option_name optname, const void* optval, socklen_t optlen
            // param�tres sortants : void
            ////////////////////////////////////////////////////////////////////////////////
            void setsockopt(Socket::level level, Socket::option_name optname, const void* optval, socklen_t optlen);
            ////////////////////////////////////////////////////////////////////////////////
            // M�thode : getsockopt
            // Usage   : Permet de lire les options d'un socket (broadcast etc...)
            // param�tres entrants : Socket::level level, Socket::option_name optname, void* optval, socklen_t* optlen
            // param�tres sortants : void
            ////////////////////////////////////////////////////////////////////////////////
            void getsockopt(Socket::level level, Socket::option_name optname, void* optval, socklen_t* optlen);
            ////////////////////////////////////////////////////////////////////////////////
            // M�thode : shutdown
            // Usage   : Ferme la lecture, l'�criture ou les 2 de _Sock
            // param�tres entrants : int _How = socketshutdown::both
            // param�tres sortants : aucun
            ////////////////////////////////////////////////////////////////////////////////
            void shutdown(Socket::shutdown_flags _How = Socket::shutdown_flags::both);
            ////////////////////////////////////////////////////////////////////////////////
            // M�thode : close
            // Usage   : Lib�re les ressources du socket
            // param�tres entrants : aucun
            // param�tres sortants : aucun
            ////////////////////////////////////////////////////////////////////////////////
            void close();
            ////////////////////////////////////////////////////////////////////////////////
            // M�thode : set_nonblocking
            // Usage   : Passe en mode non bloquant
            // param�tres entrants : bool non_blocking = true
            // param�tres sortants : aucun
            ////////////////////////////////////////////////////////////////////////////////
            void set_nonblocking(bool non_blocking = true);

            Socket::socket_t get_sock() const;
        protected:
            std::shared_ptr<Socket::socket_t> _sock;

            bool isvalid() const;

            basic_socket()                    = default;
            basic_socket(basic_socket const&) = default;
            basic_socket(basic_socket &&)     = default;
            basic_socket(Socket::socket_t s);

            basic_socket& operator =(basic_socket const&) = default;
            basic_socket& operator =(basic_socket &&)     = default;

            ////////////////////////////////////////////////////////////////////////////////
            // M�thode : socket
            // Usage   : Cr�e le socket avec les informations pass�es
            // param�tres entrants : Socket::address_family af, Socket::types type, Socket::protocols proto
            // param�tres sortants : aucun
            ////////////////////////////////////////////////////////////////////////////////
            void socket(Socket::address_family af, Socket::types type, Socket::protocols proto);
            ////////////////////////////////////////////////////////////////////////////////
            // M�thode : reset_socket
            // Usage   : Modifie la valeur de l'attribut _sock 
            // param�tres entrants : Socket::socket_t s
            // param�tres sortants : aucun
            ////////////////////////////////////////////////////////////////////////////////
            void reset_socket(Socket::socket_t s);
    };

    template<typename Addr, Socket::address_family family, Socket::types type, Socket::protocols proto>
    class LOCAL_API connected_socket : public basic_socket
    {
        public:
            using myaddr_t = Addr;
            using mytype_t = connected_socket<Addr, family, type, proto>;

        protected:
            connected_socket(Socket::socket_t s);
            myaddr_t _addr;

        public:
            connected_socket();
            connected_socket(connected_socket const&)      = default;
            connected_socket(connected_socket &&) noexcept = default;

            connected_socket& operator=(connected_socket const&)     = default;
            connected_socket& operator=(connected_socket&&) noexcept = default;

            virtual ~connected_socket() = default;

            ////////////////////////////////////////////////////////////////////////////////
            // M�thode : get_addr
            // Usage   : Get the internal addr object
            // param�tres entrants : none
            // param�tres sortants : myaddr_t const&
            ////////////////////////////////////////////////////////////////////////////////
            inline myaddr_t const& get_addr() const;
            ////////////////////////////////////////////////////////////////////////////////
            // M�thode : listen
            // Usage   : Permet au client d'�tre en �coute et d'accepter les clients
            // param�tres entrants : int waiting_socks = 5
            // param�tres sortants : aucun
            ////////////////////////////////////////////////////////////////////////////////
            inline void listen(int waiting_socks = 5);
            ////////////////////////////////////////////////////////////////////////////////
            // M�thode : connect
            // Usage   : Connect le client � l'adresse addr
            // param�tres entrants : myaddr_t const &addr
            // param�tres sortants : aucun
            ////////////////////////////////////////////////////////////////////////////////
            inline void connect(myaddr_t const& addr);
            ////////////////////////////////////////////////////////////////////////////////
            // M�thode : accept
            // Usage   : Accepte la connexion un client
            // param�tres entrants : mytype_t
            // param�tres sortants : aucun
            ////////////////////////////////////////////////////////////////////////////////
            inline mytype_t accept();
            ////////////////////////////////////////////////////////////////////////////////
            // M�thode : socket
            // Usage   : Cr�� un nouveau socket en lib�rant les anciennes ressources
            // param�tres entrants : aucun
            // param�tres sortants : aucun
            // Throw   : Peut lancer une exception si la cr�ation du socket �choue
            ////////////////////////////////////////////////////////////////////////////////
            inline void socket();
            ////////////////////////////////////////////////////////////////////////////////
            // M�thode : bind
            // Usage   : Bind une adresse
            // param�tres entrants : myaddr_t const& addr
            // param�tres sortants : aucun
            ////////////////////////////////////////////////////////////////////////////////
            inline void bind(myaddr_t const& addr);

            size_t recv(void* buffer, size_t len, Socket::socket_flags flags = Socket::socket_flags::normal);
            size_t send(const void* buffer, size_t len, Socket::socket_flags flags = Socket::socket_flags::normal);
    };

    template<typename Addr, Socket::address_family family, Socket::types type, Socket::protocols proto>
    class LOCAL_API unconnected_socket : public basic_socket
    {
        public:
            using myaddr_t = Addr;
            using mytype_t = unconnected_socket<Addr, family, type, proto>;

        protected:
            myaddr_t _addr;

        public:
            unconnected_socket();
            unconnected_socket(unconnected_socket const&)      = default;
            unconnected_socket(unconnected_socket &&) noexcept = default;

            unconnected_socket& operator=(unconnected_socket const&)      = default;
            unconnected_socket& operator=(unconnected_socket &&) noexcept = default;

            virtual ~unconnected_socket() = default;

            ////////////////////////////////////////////////////////////////////////////////
            // M�thode : get_addr
            // Usage   : Get the internal addr object
            // param�tres entrants : none
            // param�tres sortants : myaddr_t const&
            ////////////////////////////////////////////////////////////////////////////////
            inline myaddr_t const& get_addr() const;
            ////////////////////////////////////////////////////////////////////////////////
            // M�thode : socket
            // Usage   : Cr�� un nouveau socket en lib�rant les anciennes ressources
            // param�tres entrants : aucun
            // param�tres sortants : aucun
            // Throw   : Peut lancer une exception si la cr�ation du socket �choue
            ////////////////////////////////////////////////////////////////////////////////
            inline void socket();
            ////////////////////////////////////////////////////////////////////////////////
            // M�thode : bind
            // Usage   : Bind une adresse
            // param�tres entrants : myaddr_t const& addr
            // param�tres sortants : aucun
            ////////////////////////////////////////////////////////////////////////////////
            inline void bind(myaddr_t const& addr);

            size_t recvfrom(basic_addr& addr, void* buffer, size_t len, Socket::socket_flags flags = Socket::socket_flags::normal);
            size_t sendto(basic_addr & addr, const void* buffer, size_t len, Socket::socket_flags flags = Socket::socket_flags::normal);
    };

#include "basic_socket.inl"
}
