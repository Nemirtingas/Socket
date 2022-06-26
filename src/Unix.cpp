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

#include <NetworkLibrary/Unix.h>
#include "internals/internal_socket.h"

#if defined(SOCKET_OS_WINDOWS)
    #include <afunix.h>
#endif

#ifndef UNIX_PATH_MAX
  #define UNIX_PATH_MAX 108
#endif

namespace NetworkLibrary {
namespace Unix {
    static constexpr int _AddressFamily = (int)AF_UNIX;

    static constexpr int _TypeUnixDgram = (int)SOCK_DGRAM;
    static constexpr int _ProtoUnixDgram = (int)0;

    static constexpr int _TypeUnixStream = (int)SOCK_STREAM;
    static constexpr int _ProtoUnixStream = (int)0;

    /****************************************
     * 
     * UnixAddr implementation
     * 
     ****************************************/

    SOCKET_HIDE_CLASS(class) UnixAddrImpl
    {
        using my_sockaddr_t = sockaddr_un;
        my_sockaddr_t _SockAddr;

    public:
        UnixAddrImpl() :
            _SockAddr()
        {
            _SockAddr.sun_family = _AddressFamily;
        }

        UnixAddrImpl(UnixAddrImpl const& other) :
            _SockAddr()
        {
            memcpy(&_SockAddr, &other._SockAddr, sizeof(my_sockaddr_t));
        }

        UnixAddrImpl& operator=(UnixAddrImpl const& other)
        {
            memcpy(&_SockAddr, &other._SockAddr, sizeof(my_sockaddr_t));
            return *this;
        }

        UnixAddrImpl(UnixAddr&& other) noexcept = delete;
        UnixAddrImpl& operator=(UnixAddr&& other) noexcept = delete;

        std::string ToString(bool with_port) const
        {
            return _SockAddr.sun_path;
        }

        void* GetAddr()
        {
            return &_SockAddr;
        }

        const void* GetAddr() const
        {
            return &_SockAddr;
        }

        int GetFamily() const
        {
            return _AddressFamily;
        }

        size_t GetLength() const
        {
            return sizeof(my_sockaddr_t);
        }

        NetworkLibrary::Error FromString(std::string str)
        {
            if (str.empty() || str.length() >= UNIX_PATH_MAX)
            {
                return Internals::MakeErrorFromSocketCode(Error::InVal);
            }

            memcpy(_SockAddr.sun_path, str.c_str(), str.length()+1);

            return Internals::MakeErrorFromSocketCode(Error::NoError);
        }
    };

    UnixAddr::UnixAddr() :
        _Impl(new UnixAddrImpl)
    {}

    UnixAddr::UnixAddr(UnixAddr const& other) :
        _Impl(new UnixAddrImpl(*other._Impl))
    {}

    UnixAddr::UnixAddr(UnixAddr&& other) noexcept:
        _Impl(other._Impl)
    {
        other._Impl = nullptr;
    }

    UnixAddr& UnixAddr::operator=(UnixAddr const& other)
    {
        UnixAddrImpl* tmp(new UnixAddrImpl(*other._Impl));
        delete _Impl;
        _Impl = tmp;
        return *this;
    }

    UnixAddr& UnixAddr::operator=(UnixAddr&& other) noexcept
    {
        UnixAddrImpl* tmp = other._Impl;
        other._Impl = _Impl;
        _Impl = tmp;
        return *this;
    }

    UnixAddr::~UnixAddr()
    {
        delete _Impl;
    }

    std::string UnixAddr::ToString(bool with_port) const
    {
        return _Impl->ToString(with_port);
    }

    void* UnixAddr::GetAddr()
    {
        return _Impl->GetAddr();
    }

    const void* UnixAddr::GetAddr() const
    {
        return _Impl->GetAddr();
    }

    int UnixAddr::GetFamily() const
    {
        return _Impl->GetFamily();
    }

    size_t UnixAddr::GetLength() const
    {
        return _Impl->GetLength();
    }

    NetworkLibrary::Error UnixAddr::FromString(std::string str)
    {
        return _Impl->FromString(str);
    }

    /****************************************
     *
     * UnixDgram implementation
     *
     ****************************************/
    UnixDgram::UnixDgram() :
        _BoundAddress(nullptr)
    {}

    UnixDgram::UnixDgram(UnixDgram&& other) noexcept :
        _BoundAddress(nullptr)
    {
        _Impl = other._Impl;
        _BoundAddress = other._BoundAddress;

        other._Impl = nullptr;
        other._BoundAddress = nullptr;
    }

    UnixDgram& UnixDgram::operator=(UnixDgram&& other) noexcept
    {
        auto impl = other._Impl;
        auto bound = other._BoundAddress;

        other._Impl = _Impl;
        other._BoundAddress = _BoundAddress;

        _Impl = impl;
        _BoundAddress = bound;

        return *this;
    }

    UnixDgram::~UnixDgram()
    {
        UnixCleanup();
    }

    void UnixDgram::UnixCleanup()
    {
        if (_BoundAddress != nullptr && !_BoundAddress->empty())
        {// Best effort, might not always work.
#if defined(SOCKET_OS_WINDOWS)
            int utf16_size = MultiByteToWideChar(CP_UTF8, 0, &(*_BoundAddress)[0], (int)_BoundAddress->size(), nullptr, 0);
            std::wstring wstr(utf16_size, L'\0');
            MultiByteToWideChar(CP_UTF8, 0, &(*_BoundAddress)[0], (int)_BoundAddress->size(), &wstr[0], utf16_size);

            DeleteFileW(wstr.c_str());
#else
            unlink(_BoundAddress->c_str());
#endif
            delete _BoundAddress; _BoundAddress = nullptr;
        }
    }

    NetworkLibrary::Error UnixDgram::CreateSocket()
    {
        NetworkLibrary::Error error = _Impl->CreateSocket(
            (Internals::AddressFamily)_AddressFamily,
            (Internals::SocketTypes)_TypeUnixDgram,
            (Internals::SocketProtocols)_ProtoUnixDgram);

        if (error.ErrorCode == Error::NoError)
        {
            UnixCleanup();
        }

        return error;
    }

    NetworkLibrary::Error UnixDgram::GetSockName(UnixAddr& out_addr)
    {
        return Internals::getsockname(*_Impl, out_addr);
    }

    int UnixDgram::GetFamily() const { return _AddressFamily; }
    int UnixDgram::GetType  () const { return _TypeUnixDgram; }
    int UnixDgram::GetProto () const { return _ProtoUnixDgram; }

    NetworkLibrary::Error UnixDgram::Bind(BasicAddr const& addr)
    {
        NetworkLibrary::Error error = Internals::bind(*_Impl, addr);

        if (error.ErrorCode == Error::NoError)
        {
            UnixCleanup();
            _BoundAddress = new std::string(reinterpret_cast<const sockaddr_un*>(addr.GetAddr())->sun_path);
        }

        return error;
    }

    /****************************************
     *
     * UnixStream implementation
     *
     ****************************************/
    UnixStream::UnixStream():
        _BoundAddress(nullptr)
    {}

    UnixStream::UnixStream(UnixStream&& other) noexcept:
        _BoundAddress(nullptr)
    {
        _Impl = other._Impl;
        _BoundAddress = other._BoundAddress;

        other._Impl = nullptr;
        other._BoundAddress = nullptr;
    }

    UnixStream& UnixStream::operator=(UnixStream&& other) noexcept
    {
        auto impl = other._Impl;
        auto bound = other._BoundAddress;

        other._Impl = _Impl;
        other._BoundAddress = _BoundAddress;

        _Impl = impl;
        _BoundAddress = bound;

        return *this;
    }

    UnixStream::~UnixStream()
    {
        UnixCleanup();
    }

    void UnixStream::UnixCleanup()
    {
        if (_BoundAddress != nullptr && !_BoundAddress->empty())
        {// Best effort, might not always work.
#if defined(SOCKET_OS_WINDOWS)
            int utf16_size = MultiByteToWideChar(CP_UTF8, 0, &(*_BoundAddress)[0], (int)_BoundAddress->size(), nullptr, 0);
            std::wstring wstr(utf16_size, L'\0');
            MultiByteToWideChar(CP_UTF8, 0, &(*_BoundAddress)[0], (int)_BoundAddress->size(), &wstr[0], utf16_size);

            DeleteFileW(wstr.c_str());
#else
            unlink(_BoundAddress->c_str());
#endif
            delete _BoundAddress; _BoundAddress = nullptr;
        }
    }

    NetworkLibrary::Error UnixStream::CreateSocket()
    {
        NetworkLibrary::Error error = _Impl->CreateSocket(
            (Internals::AddressFamily)_AddressFamily,
            (Internals::SocketTypes)_TypeUnixStream,
            (Internals::SocketProtocols)_ProtoUnixStream);

        if (error.ErrorCode == Error::NoError)
        {
            UnixCleanup();
        }

        return error;
    }

    NetworkLibrary::Error UnixStream::GetSockName(UnixAddr& out_addr)
    {
        return Internals::getsockname(*_Impl, out_addr);
    }

    int UnixStream::GetFamily() const { return _AddressFamily; }
    int UnixStream::GetType  () const { return _TypeUnixStream; }
    int UnixStream::GetProto () const { return _ProtoUnixStream; }

    NetworkLibrary::Error UnixStream::Bind(BasicAddr const& addr)
    {
        NetworkLibrary::Error error = Internals::bind(*_Impl, addr);

        if (error.ErrorCode == Error::NoError)
        {
            UnixCleanup();
            _BoundAddress = new std::string(reinterpret_cast<const sockaddr_un*>(addr.GetAddr())->sun_path);
        }

        return error;
    }
}

}