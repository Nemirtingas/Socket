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
    ////////////
    /// @brief A bluetooth sock_addr object
    ////////////
    class LOCAL_API bth_addr : public basic_addr
    {
        public:
            using my_sockaddr = sockaddr_rc;
#if defined(__WINDOWS__)
            constexpr static bdaddr_t any_addr = 0ull;
#elif defined(__LINUX__)
            constexpr static bdaddr_t any_addr = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
#endif

        private:
            my_sockaddr *_sockaddr;

        public:
            bth_addr();
            bth_addr(bth_addr const&);
            bth_addr(bth_addr &&) noexcept;
            bth_addr& operator =(bth_addr const&);
            bth_addr& operator =(bth_addr &&) noexcept;

            virtual ~bth_addr();
            ////////////
            /// @brief Transforms the address to a human readable string
            /// @param[in] with_port Append the port
            /// @return Address formated like <addr>[@<channel>]
            ////////////
            virtual std::string to_string(bool with_port = false) const;
            ////////////
            /// @brief Transforms the human readable string into an address
            /// @param[in] str Pass in a formated string like <addr>[@<channel>]
            /// @return 
            ////////////
            virtual void from_string(std::string const& str);
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
            /// @brief Sets the addr
            /// @param[in]  addr The bluetooth addr
            /// @return 
            ////////////
            void set_addr(bdaddr_t const& addr);
            ////////////
            /// @brief Sets the buetooth channel
            /// @param[in]  channel The bluetooth channel
            /// @return 
            ////////////
            void set_channel(uint8_t channel);
            ////////////
            /// @brief Gets the Bluetooth addr
            /// @return The Bluetooth addr
            ////////////
            bdaddr_t get_addr() const;
            ////////////
            /// @brief Gets the Bluetooth channel
            /// @return The Bluetooth channel
            ////////////
            uint8_t get_channel() const;
            ////////////
            /// @brief Gets the native addr structure
            /// @return Native structure
            ////////////
            my_sockaddr& get_native_addr();
    };
}
#endif
