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

#ifndef _UNIX_ADDR_INCLUDED_
#define _UNIX_ADDR_INCLUDED_

#include <Socket/common/basic_socket.h>

#if defined(__WINDOWS__)
    #include <afunix.h>
#elif defined(__LINUX__) || defined(__APPLE__)
    #include <sys/un.h>
#endif

namespace PortableAPI
{
    class LOCAL_API unix_addr : public basic_addr
    {
        public:
            typedef sockaddr_un my_sockaddr;

        private:
            my_sockaddr *_sockaddr;

        public:
            unix_addr();
            unix_addr(unix_addr const&);
            unix_addr(unix_addr &&) noexcept;
            unix_addr& operator =(unix_addr const&);
            unix_addr& operator =(unix_addr &&) noexcept;

            virtual ~unix_addr();
            // Returns addr formated like <path>
            virtual std::string to_string() const;
            // Pass in a formated std::string like <path>
            virtual void from_string(std::string const& str);
            virtual sockaddr& addr();
            virtual sockaddr const& addr() const;
            virtual size_t len() const;
            virtual void set_any_addr();
            void set_path(std::string const& path);
            std::string get_path() const;
            my_sockaddr& get_native_addr();
    };
}
#endif
