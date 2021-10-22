/* Copyright (C) Nemirtingas
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

#include <cstdint>
#include <string>
#include <vector>
#include <chrono>

namespace NetworkLibrary
{
    namespace Internals {
        class NativeSocket;
    }

    ////////////
    /// @brief recv, recvfrom, send, sendto flags enum 
    ////////////
    namespace SocketFlags {
        extern int32_t normal;
        extern int32_t oob;       // process out-of-band data
        extern int32_t peek;      // peek at incoming message
        extern int32_t dontroute; //send without using routing tables
    }

    ////////////
    /// @brief Poll flags enum 
    ////////////
    namespace PollFlags {
        extern int16_t none  ;
        extern int16_t in    ;
        extern int16_t pri   ;
        extern int16_t out   ;
        extern int16_t err   ;
        extern int16_t hup   ;
        extern int16_t nval  ;
        extern int16_t rdnorm;
        extern int16_t rdband;
        extern int16_t wrnorm;
        extern int16_t wrband;
    }

	////////////
    /// @brief NetworkLibrary error codes, use Error::NativeCode to get the OS native error code.
    ////////////
    struct Error
    {
        static constexpr int UnknownError         = -1;
        static constexpr int NoError              =  0;
        static constexpr int OutOfMemory          =  1;
        static constexpr int Access               =  2;
        static constexpr int AddrInUse            =  3;
        static constexpr int AddrNotAvailable     =  4;
        static constexpr int ConnectionRefused    =  5;
        static constexpr int ConnectionAborted    =  6;
        static constexpr int ConnectionReset      =  7;
        static constexpr int Fault                =  8;
        static constexpr int IsConnected          =  9;
        static constexpr int InProgress           =  10;
        static constexpr int InVal                =  11;
        static constexpr int NotConnected         =  12;
        static constexpr int NetworkUnreachable   =  13;
        static constexpr int WouldBlock           =  14;
        static constexpr int AfNotSupported       =  15;
        static constexpr int SockTypeNotSupported =  16;
        static constexpr int MessageSize          =  17;
        static constexpr int NotFound             =  18;
        static constexpr int TimedOut             =  19;
        static constexpr int HostDown             =  20;
        static constexpr int HostUnreachable      =  21;

        // Windows Only
        static constexpr int WsaNotInitialised      = 10000;
        static constexpr int WsaNetDown             = 10001;
        static constexpr int WsaSystemNotReady      = 10002;
        static constexpr int WsaVersionNotSupported = 10003;
        static constexpr int WsaProClim             = 10004;

        int ErrorCode;
        int NativeCode;

        std::string ToString();

        explicit operator int() const { return ErrorCode; }
    };

    struct NetBuffer
    {
        void* Buffer;
        size_t BufferSize;
    };

    ////////////
    /// @brief An abstract class to represent a Network Address, like a sockaddr*
    ////////////
    class BasicAddr
    {
    protected:
        BasicAddr();

    public:
        ////////////
        /// @brief 
        ////////////
        virtual ~BasicAddr();
        ////////////
        /// @brief Transforms the address to a human readable string
        /// @param[in] with_port Append the port
        /// @return The string representation of the address
        ////////////
        virtual std::string ToString(bool with_port) const = 0;
        ////////////
        /// @brief Get this Addr family type.
        /// @return 
        ////////////
        virtual int GetFamily() const = 0;
        ////////////
        /// @brief Get this Addr sockaddr.
        /// @return Reference
        ////////////
        virtual void* GetAddr() = 0;
        ////////////
        /// @brief Get this Addr sockaddr.
        /// @return Const reference
        ////////////
        virtual const void* GetAddr() const = 0;
        ////////////
        /// @brief Get this Addr length.
        /// @return Addr length
        ////////////
        virtual size_t GetLength() const = 0;
    };

    struct IfaceAddr
    {
        std::string Addr;
        uint32_t MaskCIDR;
    };

    struct IfaceInfos
    {
        std::string Name;
        std::string FriendlyName;
        std::string Description;
        bool IsUp;
        std::vector<IfaceAddr> Addresses;
    };

    ////////////
    /// @brief An abstract class to represent a Network Socket
    ////////////
    class BasicSocket
    {
    protected:
        class Internals::NativeSocket* _Impl;

    public:
        BasicSocket();
        BasicSocket(BasicSocket const& other) = delete;
        BasicSocket(BasicSocket&& other) noexcept = delete;
        BasicSocket& operator=(BasicSocket const& other) = delete;
        BasicSocket& operator=(BasicSocket && other) noexcept = delete;
        virtual ~BasicSocket();

        virtual int GetFamily() const = 0;
        virtual int GetType  () const = 0;
        virtual int GetProto () const = 0;

        inline bool IsSameType(BasicSocket const& other) const { return this->GetFamily() == other.GetFamily() && this->GetType() == other.GetType() && this->GetProto() == other.GetProto(); }

        ////////////
        /// @brief Returns the native socket file descriptor.
        /// @return Socket FD
        ////////////
        int64_t GetNativeFd() const;
        ////////////
        /// @brief Returns if the socket is actually open.
        /// @return Is socket open.
        ////////////
        bool IsOpen() const;
        ////////////
        /// @brief Swaps two BasicSocket if they have the same type.
        /// @return Error
        ////////////
        NetworkLibrary::Error Swap(BasicSocket& other) noexcept;
        ////////////
        /// @brief Sets the socket to non-blocking IO.
		/// @param[in] non_blocking Non-blocking value.
        /// @return Error
        ////////////
        NetworkLibrary::Error SetNonBlocking(bool non_blocking);
        ////////////
        /// @brief Gets the bytes count ready to be read on the socket.
        /// @return Waiting size.
        ////////////
        int32_t GetWaitingSize() const;

        ////////////
        /// @brief Closes this socket.
        /// @return
        ////////////
        void Close();
    };

    ////////////
    /// @brief An abstract class to represent a connected socket (like tcp).
    ////////////
    class ConnectedSocket :
        public BasicSocket
    {
    public:
        ////////////
        /// @brief Bind the socket on an Address.
        /// @param[in] addr The address to bind on.
        /// @return Error code
        ////////////
        virtual NetworkLibrary::Error Bind(BasicAddr const& addr);
        ////////////
        /// @brief Start listening on bound address, accepting connections.
        /// @param[in] backlog The max amount of waiting connections.
        /// @return Error code
        ////////////
        virtual NetworkLibrary::Error Listen(int backlog = 5);
        ////////////
        /// @brief Try to connect to the remote address.
        /// @param[in] new_client The address of the new client.
        /// @param[in] client_addr  The new client socket.
        /// @return Error code
        ////////////
        virtual NetworkLibrary::Error Accept(ConnectedSocket& new_client, BasicAddr& client_addr);
        ////////////
        /// @brief Try to connect to the remote address.
        /// @param[in] addr The address to bind on.
        /// @return Error code
        ////////////
        virtual NetworkLibrary::Error Connect(BasicAddr const& addr);
        ////////////
        /// @brief Sends datas to address.
        /// @param[in] buffer The datas to send. buffer.BufferSize will be filled with the sent size.
        /// @param[in] flags  The send flags.
        /// @return Error code
        ////////////
        virtual NetworkLibrary::Error Send(NetBuffer& buffer, int32_t flags = SocketFlags::normal);
        ////////////
        /// @brief Retrieves waiting datas on socket.
        /// @param[in] buffer The datas to send. buffer.BufferSize will be filled with the sent size.
        /// @param[in] flags  The receive flags.
        /// @return Error code
        ////////////
        virtual NetworkLibrary::Error Receive(NetBuffer& buffer, int32_t flags = SocketFlags::normal);
    };

    ////////////
    /// @brief An abstract class to represent an unconnected socket (like udp).
    ////////////
    class UnconnectedSocket :
        public BasicSocket
    {
    public:
        ////////////
        /// @brief Bind the socket on an Address.
        /// @param[in] addr The address to bind on.
        /// @return Error code
        ////////////
        virtual NetworkLibrary::Error Bind(BasicAddr const& addr);
        ////////////
        /// @brief Sends datas to address.
        /// @param[in] addr   The address to send to.
        /// @param[in] buffer The datas to send. buffer.BufferSize will be filled with the sent size.
        /// @param[in] flags  The send flags.
        /// @return Error code
        ////////////
        virtual NetworkLibrary::Error SendTo(BasicAddr const& addr, NetBuffer& buffer, int32_t flags = SocketFlags::normal);
        ////////////
        /// @brief Retrieves waiting datas on socket.
        /// @param[in] addr   The address to send to.
        /// @param[in] buffer The datas to send. buffer.BufferSize will be filled with the sent size.
        /// @param[in] flags  The receive flags.
        /// @return Error code
        ////////////
        virtual NetworkLibrary::Error ReceiveFrom(BasicAddr& addr, NetBuffer& buffer, int32_t flags = SocketFlags::normal);
    };
}