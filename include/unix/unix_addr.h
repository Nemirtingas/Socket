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

#ifndef _UNIX_ADDR_INCLUDED_
#define _UNIX_ADDR_INCLUDED_

#include "../common/basic_socket.h"

#if defined(__WINDOWS__)
    #include <afunix.h>
#endif

#ifndef UNIX_PATH_MAX
  #define UNIX_PATH_MAX 108
#endif

namespace PortableAPI
{
    class EXPORT_SOCKET_API unix_addr : public basic_addr
    {
        public:
            using my_sockaddr = sockaddr_un;

        private:
            my_sockaddr *_sockaddr;

        public:
            unix_addr();
            unix_addr(unix_addr const&);
            unix_addr(unix_addr &&) noexcept;
            unix_addr& operator =(unix_addr const&);
            unix_addr& operator =(unix_addr &&) noexcept;

            virtual ~unix_addr();
            ////////////
            /// @brief Transforms the address to a human readable string
            /// @param[in] with_port Append the port
            /// @return Address formated like <path>
            ////////////
            virtual std::string to_string(bool with_port = false) const;
            ////////////
            /// @brief Transforms the human readable string into an address
            /// @param[in] str Pass in a formated string like <path>
            /// @return false if failed to parse, true if succeeded to parse
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
            /// @brief Sets the path
            /// @param[in]  addr The path
            /// @return 
            ////////////
            void set_addr(std::string const& path);
            ////////////
            /// @brief Gets the path
            /// @return The path
            ////////////
            std::string get_addr() const;
            ////////////
            /// @brief Gets the native addr structure
            /// @return Native structure
            ////////////
            my_sockaddr& get_native_addr();
    };
}
#endif
