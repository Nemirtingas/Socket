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

#include "internals/internal_socket.h"

namespace NetworkLibrary {
    std::string Error::ToString()
    {
        std::string message;

        switch (ErrorCode)
        {
            case ::NetworkLibrary::Error::NoError               : message = "No error."                           ; break;
            case ::NetworkLibrary::Error::OutOfMemory           : message = "Error out of memory."                ; break;
            case ::NetworkLibrary::Error::Access                : message = "Error no access."                    ; break;
            case ::NetworkLibrary::Error::AddrInUse             : message = "Error address in use."               ; break;
            case ::NetworkLibrary::Error::AddrNotAvailable      : message = "Error address not available."        ; break;
            case ::NetworkLibrary::Error::ConnectionRefused     : message = "Error connection refused."           ; break;
            case ::NetworkLibrary::Error::ConnectionAborted     : message = "Error connection aborted."           ; break;
            case ::NetworkLibrary::Error::ConnectionReset       : message = "Error connection reset."             ; break;
            case ::NetworkLibrary::Error::Fault                 : message = "Error fault."                        ; break;
            case ::NetworkLibrary::Error::IsConnected           : message = "Error is already connected."         ; break;
            case ::NetworkLibrary::Error::InProgress            : message = "Error is in progress."               ; break;
            case ::NetworkLibrary::Error::InVal                 : message = "Error in value."                     ; break;
            case ::NetworkLibrary::Error::NotConnected          : message = "Error not connected."                ; break;
            case ::NetworkLibrary::Error::NetworkUnreachable    : message = "Error network unreachable."          ; break;
            case ::NetworkLibrary::Error::WouldBlock            : message = "Error would block."                  ; break;
            case ::NetworkLibrary::Error::AfNotSupported        : message = "Error address family not supported." ; break;
            case ::NetworkLibrary::Error::SockTypeNotSupported  : message = "Error socket don't support type."    ; break;
            case ::NetworkLibrary::Error::MessageSize           : message = "Error message size."                 ; break;
            case ::NetworkLibrary::Error::NotFound              : message = "Error not found."                    ; break;
            case ::NetworkLibrary::Error::TimedOut              : message = "Error timed out."                    ; break;
            case ::NetworkLibrary::Error::HostDown              : message = "Error host down."                    ; break;
            case ::NetworkLibrary::Error::HostUnreachable       : message = "Error host unreachable."             ; break;

            case ::NetworkLibrary::Error::WsaNotInitialised     : message = "Error WinSock not initialized."      ; break;
            case ::NetworkLibrary::Error::WsaNetDown            : message = "Error WinSock net down."             ; break;
            case ::NetworkLibrary::Error::WsaSystemNotReady     : message = "Error WinSock system not ready."     ; break;
            case ::NetworkLibrary::Error::WsaVersionNotSupported: message = "Error WinSock verison not supported."; break;
            case ::NetworkLibrary::Error::WsaProClim            : message = "Error WinSock pro clim."             ; break;

            default: message = "Unknown error: " + std::to_string(NativeCode);
        }

        return message;
    }

    // BasicAddr

    BasicAddr::BasicAddr() {}

    BasicAddr::~BasicAddr() {}

    // BasicSocket

    BasicSocket::BasicSocket():
        _Impl(new Internals::NativeSocket)
    {}

    BasicSocket::~BasicSocket()
    {
        delete _Impl; _Impl = nullptr;
    }

    int64_t BasicSocket::GetNativeFd() const
    {
        return static_cast<int64_t>(_Impl->Socket);
    }

    bool BasicSocket::IsOpen() const
    {
        return _Impl->IsValid();
    }

    NetworkLibrary::Error BasicSocket::Swap(BasicSocket& other) noexcept
    {
        if (!IsSameType(other))
            return Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::InVal);

        auto tmp = other._Impl;
        other._Impl = _Impl;
        _Impl = tmp;
        return Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NoError);
    }

    NetworkLibrary::Error BasicSocket::SetSockOption(int32_t option_name, const void* value, int optlen)
    {
        return _Impl->SetSockOption(NetworkLibrary::Internals::OptionNameToNative(option_name), value, optlen);
    }

    NetworkLibrary::Error BasicSocket::SetNonBlocking(bool non_blocking)
    {
        return _Impl->SetNonBlocking(non_blocking);
    }

    int32_t BasicSocket::GetWaitingSize() const
    {
        return _Impl->GetWaitingSize();
    }

    void BasicSocket::Close()
    {
        _Impl->Close();
    }

    // Connected Socket
    NetworkLibrary::Error ConnectedSocket::Bind(BasicAddr const& addr)
    {
        return Internals::bind(*_Impl, addr);
    }

    NetworkLibrary::Error ConnectedSocket::Listen(int backlog)
    {
        return Internals::listen(*_Impl, backlog);
    }

    NetworkLibrary::Error ConnectedSocket::Accept(ConnectedSocket& new_client, BasicAddr& client_addr)
    {
        return Internals::accept(*_Impl, client_addr, *new_client._Impl);
    }

    NetworkLibrary::Error ConnectedSocket::Connect(BasicAddr const& addr)
    {
        return Internals::connect(*_Impl, addr);
    }

    NetworkLibrary::Error ConnectedSocket::Send(NetBuffer& buffer, int32_t flags)
    {
        return Internals::send(*_Impl, buffer.Buffer, buffer.BufferSize, NetworkLibrary::Internals::SocketFlagsToNative(flags));
    }

    NetworkLibrary::Error ConnectedSocket::Receive(NetBuffer& buffer, int32_t flags)
    {
        return Internals::recv(*_Impl, buffer.Buffer, buffer.BufferSize, NetworkLibrary::Internals::SocketFlagsToNative(flags));
    }

    // Unconnected Socket
    NetworkLibrary::Error UnconnectedSocket::Bind(BasicAddr const& addr)
    {
        return Internals::bind(*_Impl, addr);
    }

    NetworkLibrary::Error UnconnectedSocket::SendTo(BasicAddr const& addr, NetBuffer& buffer, int32_t flags)
    {
        return Internals::sendto(*_Impl, addr, buffer.Buffer, buffer.BufferSize, NetworkLibrary::Internals::SocketFlagsToNative(flags));
    }

    NetworkLibrary::Error UnconnectedSocket::ReceiveFrom(BasicAddr& addr, NetBuffer& buffer, int32_t flags)
    {
        return Internals::recvfrom(*_Impl, addr, buffer.Buffer, buffer.BufferSize, NetworkLibrary::Internals::SocketFlagsToNative(flags));
    }
}
