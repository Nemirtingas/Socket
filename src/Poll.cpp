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

#include <NetworkLibrary/Poll.h>
#include "internals/internal_socket.h"

namespace NetworkLibrary {

    SYSTEM_HIDE_CLASS(class) PollImpl
    {
        std::vector<pollfd> _PollFds;

    public:
        size_t GetSocketCount() const
        {
            return _PollFds.size();
        }

        NetworkLibrary::Error AddSocket(BasicSocket const& sock, /* PollFlags */ int16_t flags)
        {
            for (auto const& item : _PollFds)
            {
                if (item.fd == sock.GetNativeFd())
                {
                    return NetworkLibrary::Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::IsConnected);
                }
            }

            _PollFds.emplace_back(pollfd{ static_cast<Internals::NativeSocket::socket_t>(sock.GetNativeFd()), flags, 0 });
            return NetworkLibrary::Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NoError);
        }

        NetworkLibrary::Error RemoveSocket(BasicSocket const& sock)
        {
            for (auto it = _PollFds.begin(); it != _PollFds.end(); ++it)
            {
                if (it->fd == sock.GetNativeFd())
                {
                    _PollFds.erase(it);
                    return NetworkLibrary::Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NoError);
                }
            }

            return NetworkLibrary::Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NotFound);
        }

        NetworkLibrary::Error RemoveSocket(size_t index)
        {
            if(index >= _PollFds.size())
                return NetworkLibrary::Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NotFound);

            _PollFds.erase(_PollFds.begin() + index);
            return NetworkLibrary::Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NoError);
        }

        NetworkLibrary::Error SetEvents(BasicSocket const& sock, /* PollFlags */ int16_t flags)
        {
            for (auto it = _PollFds.begin(); it != _PollFds.end(); ++it)
            {
                if (it->fd == sock.GetNativeFd())
                {
                    it->events = flags;
                    return NetworkLibrary::Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NoError);
                }
            }

            return NetworkLibrary::Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NoError);
        }

        NetworkLibrary::Error SetEvents(size_t index, /* PollFlags */ int16_t flags)
        {
            if (index >= _PollFds.size())
                return NetworkLibrary::Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NotFound);

            (_PollFds.begin() + index)->events = flags;
            return NetworkLibrary::Internals::MakeErrorFromSocketCode(NetworkLibrary::Error::NoError);
        }

        int16_t GetRevents(BasicSocket const& sock)
        {
            for (auto it = _PollFds.begin(); it != _PollFds.end(); ++it)
            {
                if (it->fd == sock.GetNativeFd())
                {
                    return it->revents;
                }
            }

            return PollFlags::none;
        }

        int16_t GetRevents(size_t index)
        {
            if (index >= _PollFds.size())
                return PollFlags::none;

            return (_PollFds.begin() + index)->revents;
        }

        int32_t DoPoll(std::chrono::milliseconds timeout)
        {
            return Internals::poll(_PollFds.data(), _PollFds.size(), static_cast<int>(timeout.count()));
        }

        void Clear()
        {
            _PollFds.clear();
        }
    };

    Poll::Poll():
        _Impl(new PollImpl)
    {}

    Poll::Poll(Poll const& other):
        _Impl(new PollImpl(*other._Impl))
    {}

    Poll::Poll(Poll&& other) noexcept :
        _Impl(other._Impl)
    {
        other._Impl = nullptr;
    }

    Poll& Poll::operator=(Poll const& other)
    {
        PollImpl* tmp = new PollImpl(*other._Impl);
        delete _Impl;
        _Impl = tmp;
        return *this;
    }

    Poll& Poll::operator=(Poll&& other) noexcept
    {
        PollImpl* tmp = other._Impl;
        other._Impl = _Impl;
        _Impl = tmp;
        return *this;
    }

    size_t Poll::GetSocketCount() const
    {
        return _Impl->GetSocketCount();
    }

    NetworkLibrary::Error Poll::AddSocket(BasicSocket const& sock, int16_t flags)
    {
        return _Impl->AddSocket(sock, flags);
    }

    NetworkLibrary::Error Poll::RemoveSocket(BasicSocket const& sock)
    {
        return _Impl->RemoveSocket(sock);
    }

    NetworkLibrary::Error Poll::RemoveSocket(size_t index)
    {
        return _Impl->RemoveSocket(index);
    }

    NetworkLibrary::Error Poll::SetEvents(BasicSocket const& sock, /* PollFlags */ int16_t flags)
    {
        return _Impl->SetEvents(sock, flags);
    }

    NetworkLibrary::Error Poll::SetEvents(size_t index, /* PollFlags */ int16_t flags)
    {
        return _Impl->SetEvents(index, flags);
    }

    /* PollFlags */ int16_t Poll::GetRevents(BasicSocket const& sock)
    {
        return _Impl->GetRevents(sock);
    }

    /* PollFlags */ int16_t Poll::GetRevents(size_t index)
    {
        return _Impl->GetRevents(index);
    }

    int32_t Poll::DoPoll(std::chrono::milliseconds timeout)
    {
        return _Impl->DoPoll(timeout);
    }

    void Poll::Clear()
    {
        _Impl->Clear();
    }
}