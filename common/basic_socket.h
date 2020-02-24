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
            // Méthode : ioctlsocket
            // Usage   : Permet de modifier les paramètres d'un socket (non bloquant etc...)
            // paramètres entrants : Socket::cmd_name cmd, unsigned long* arg
            // paramètres sortants : void
            ////////////////////////////////////////////////////////////////////////////////
            void ioctlsocket(Socket::cmd_name cmd, unsigned long* arg);
            ////////////////////////////////////////////////////////////////////////////////
            // Méthode : setsockopt
            // Usage   : Permet de modifier les options d'un socket (broadcast etc...)
            // paramètres entrants : Socket::level level, Socket::option_name optname, const void* optval, socklen_t optlen
            // paramètres sortants : void
            ////////////////////////////////////////////////////////////////////////////////
            void setsockopt(Socket::level level, Socket::option_name optname, const void* optval, socklen_t optlen);
            ////////////////////////////////////////////////////////////////////////////////
            // Méthode : getsockopt
            // Usage   : Permet de lire les options d'un socket (broadcast etc...)
            // paramètres entrants : Socket::level level, Socket::option_name optname, void* optval, socklen_t* optlen
            // paramètres sortants : void
            ////////////////////////////////////////////////////////////////////////////////
            void getsockopt(Socket::level level, Socket::option_name optname, void* optval, socklen_t* optlen);
            ////////////////////////////////////////////////////////////////////////////////
            // Méthode : shutdown
            // Usage   : Ferme la lecture, l'écriture ou les 2 de _Sock
            // paramètres entrants : int _How = socketshutdown::both
            // paramètres sortants : aucun
            ////////////////////////////////////////////////////////////////////////////////
            void shutdown(Socket::shutdown_flags _How = Socket::shutdown_flags::both);
            ////////////////////////////////////////////////////////////////////////////////
            // Méthode : close
            // Usage   : Libère les ressources du socket
            // paramètres entrants : aucun
            // paramètres sortants : aucun
            ////////////////////////////////////////////////////////////////////////////////
            void close();
            ////////////////////////////////////////////////////////////////////////////////
            // Méthode : set_nonblocking
            // Usage   : Passe en mode non bloquant
            // paramètres entrants : bool non_blocking = true
            // paramètres sortants : aucun
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
            // Méthode : socket
            // Usage   : Crée le socket avec les informations passées
            // paramètres entrants : Socket::address_family af, Socket::types type, Socket::protocols proto
            // paramètres sortants : aucun
            ////////////////////////////////////////////////////////////////////////////////
            void socket(Socket::address_family af, Socket::types type, Socket::protocols proto);
            ////////////////////////////////////////////////////////////////////////////////
            // Méthode : reset_socket
            // Usage   : Modifie la valeur de l'attribut _sock 
            // paramètres entrants : Socket::socket_t s
            // paramètres sortants : aucun
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
            // Méthode : get_addr
            // Usage   : Get the internal addr object
            // paramètres entrants : none
            // paramètres sortants : myaddr_t const&
            ////////////////////////////////////////////////////////////////////////////////
            inline myaddr_t const& get_addr() const;
            ////////////////////////////////////////////////////////////////////////////////
            // Méthode : listen
            // Usage   : Permet au client d'être en écoute et d'accepter les clients
            // paramètres entrants : int waiting_socks = 5
            // paramètres sortants : aucun
            ////////////////////////////////////////////////////////////////////////////////
            inline void listen(int waiting_socks = 5);
            ////////////////////////////////////////////////////////////////////////////////
            // Méthode : connect
            // Usage   : Connect le client à l'adresse addr
            // paramètres entrants : myaddr_t const &addr
            // paramètres sortants : aucun
            ////////////////////////////////////////////////////////////////////////////////
            inline void connect(myaddr_t const& addr);
            ////////////////////////////////////////////////////////////////////////////////
            // Méthode : accept
            // Usage   : Accepte la connexion un client
            // paramètres entrants : mytype_t
            // paramètres sortants : aucun
            ////////////////////////////////////////////////////////////////////////////////
            inline mytype_t accept();
            ////////////////////////////////////////////////////////////////////////////////
            // Méthode : socket
            // Usage   : Créé un nouveau socket en libérant les anciennes ressources
            // paramètres entrants : aucun
            // paramètres sortants : aucun
            // Throw   : Peut lancer une exception si la création du socket échoue
            ////////////////////////////////////////////////////////////////////////////////
            inline void socket();
            ////////////////////////////////////////////////////////////////////////////////
            // Méthode : bind
            // Usage   : Bind une adresse
            // paramètres entrants : myaddr_t const& addr
            // paramètres sortants : aucun
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
            // Méthode : get_addr
            // Usage   : Get the internal addr object
            // paramètres entrants : none
            // paramètres sortants : myaddr_t const&
            ////////////////////////////////////////////////////////////////////////////////
            inline myaddr_t const& get_addr() const;
            ////////////////////////////////////////////////////////////////////////////////
            // Méthode : socket
            // Usage   : Créé un nouveau socket en libérant les anciennes ressources
            // paramètres entrants : aucun
            // paramètres sortants : aucun
            // Throw   : Peut lancer une exception si la création du socket échoue
            ////////////////////////////////////////////////////////////////////////////////
            inline void socket();
            ////////////////////////////////////////////////////////////////////////////////
            // Méthode : bind
            // Usage   : Bind une adresse
            // paramètres entrants : myaddr_t const& addr
            // paramètres sortants : aucun
            ////////////////////////////////////////////////////////////////////////////////
            inline void bind(myaddr_t const& addr);

            size_t recvfrom(basic_addr& addr, void* buffer, size_t len, Socket::socket_flags flags = Socket::socket_flags::normal);
            size_t sendto(basic_addr & addr, const void* buffer, size_t len, Socket::socket_flags flags = Socket::socket_flags::normal);
    };

#include "basic_socket.inl"
}
