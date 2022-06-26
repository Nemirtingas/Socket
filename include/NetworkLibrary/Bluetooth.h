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

#if defined(__APPLE__)
#error "Apple Bluetooth is not supported, please don't include this header."
#endif

#pragma once

#include "details/Socket.h"

namespace NetworkLibrary {
namespace Bluetooth {
#pragma pack(push, 1)
struct BluetoothMac
{
    uint8_t addr[6];
};
#pragma pack(pop)
}
}

namespace NetworkLibrary {
namespace Bluetooth {

////////////
/// @brief Class with address and name about a device
////////////
struct BluetoothDevice
{
    std::string Address;
    std::string Name;

    BluetoothDevice() = default;
    BluetoothDevice(BluetoothDevice const&) = default;
    BluetoothDevice(BluetoothDevice&&) noexcept = default;

    BluetoothDevice& operator=(BluetoothDevice const&) = default;
    BluetoothDevice& operator=(BluetoothDevice&&) noexcept = default;
};

struct BluetoothUUID_t{
    uint8_t uuid[16];
};

////////////
/// @brief Utility Class for manipulating uuids
////////////
class UUID
{
    BluetoothUUID_t _UUID;

public:
    ////////////
    /// @brief 
    ////////////
    UUID();
    UUID(UUID const& other);
    UUID(UUID && other) noexcept;
    UUID& operator=(UUID const& other);
    UUID& operator=(UUID&& other) noexcept;
    ////////////
    /// @brief Gets the stored UUID.
    /// @return Error
    ////////////
    BluetoothUUID_t GetUUID() const;
    ////////////
    /// @brief Sets a Bluetooth uuid16.
    /// @param[in] uuid uuid16
    /// @return Error
    ////////////
    NetworkLibrary::Error SetUUID16(uint16_t uuid);
    ////////////
    /// @brief Sets a Bluetooth uuid32.
    /// @param[in] uuid uuid32
    /// @return Error
    ////////////
    NetworkLibrary::Error SetUUID32(uint32_t uuid);
    ////////////
    /// @brief Sets a Bluetooth uuid128.
    /// @param[in] uuid uuid128
    /// @return Error
    ////////////
    NetworkLibrary::Error SetUUID128(BluetoothUUID_t const& uuid);
    ////////////
    /// @brief Compares 2 UUIDs.
    /// @param[in] other The other UUID
    /// @return Is equal.
    ////////////
    bool operator ==(UUID const& other) const;
    ////////////
    /// @brief Compares 2 UUIDs.
    /// @param[in] other The other UUID
    /// @return Is not equal.
    ////////////
    bool operator !=(UUID const& other) const;
    ////////////
    /// @brief Set the internal value from the string
    ///        Must be formated like to: 01234567-89AB-CDEF-0123-456789ABCDEF
    /// @param[in] str_uuid A string representation of an UUID
    /// @return Error
    ////////////
    NetworkLibrary::Error FromString(std::string const& str_uuid);
    ////////////
    /// @brief Transforms the internal UUID into a human readable string
    /// @return UUID as string
    ////////////
    std::string ToString() const;
    ////////////
    /// @brief Checks if the string is a valid uuid
    ///        Must be formated like so: 01234567-89AB-CDEF-0123-456789ABCDEF
    /// @param[in] str_uuid A string representation of an UUID
    /// @return Is valid
    ////////////
    static bool IsValidUUID(std::string const& str_uuid);
    ////////////
    /// @brief Get the base Bluetooth UUID
    /// @return Bluetooth UUID
    ////////////
    static UUID GetBluetoothBaseUUID();
};

////////////
/// @brief Utility Class for registering/unregistering Bluetooth SDP
////////////
class SDPService
{
    void* _SDPHandle;
public:
    SDPService(SDPService const& other) = delete;
    SDPService& operator=(SDPService const& other) = delete;

    ////////////
    /// @brief
    ////////////
    SDPService();
    ////////////
    /// @brief
    ////////////
    SDPService(SDPService && other) noexcept;
    ////////////
    /// @brief
    ////////////
    SDPService& operator=(SDPService && other) noexcept;
    ////////////
    /// @brief
    ////////////
    ~SDPService();
    ////////////
    /// @brief Register an uuid/port pair into the Bluetooth SDP
    /// @param[in] uuid     The service uuid
    /// @param[in] port     The service port
    /// @param[in] srv_name The service name
    /// @param[in] srv_prov The service provider
    /// @param[in] srv_desc The service description
    /// @return Error
    ////////////
    NetworkLibrary::Error RegisterSDPServiceRFCOMM(NetworkLibrary::Bluetooth::UUID const& uuid, uint8_t port, std::string const& srv_name, std::string const& srv_prov, std::string const& srv_desc);
    ////////////
    /// @brief Unregister a SDP record
    /// @return Error, if there was an error, handle has not been freed.
    ////////////
    NetworkLibrary::Error UnregisterSDPServiceRFCOMM();
};

////////////
/// @brief Gets all Bluetooth interfaces.
/// @return A list of IfaceInfos.
////////////
std::pair<NetworkLibrary::Error, std::vector<IfaceInfos>> GetIfacesAddresses();
////////////
/// @brief Starts a Bluetooth scan to detect nearby devices
/// @param[in] flushCache Flush the cache before scanning
/// @return A list of BluetoothDevices
////////////
std::vector<NetworkLibrary::Bluetooth::BluetoothDevice> ScanDevices(bool flushCache = true);
////////////
/// @brief Query the remote device's SDP about the UUID's port
/// @param[in]   uuid        The remote service UUID
/// @param[in]   remote_addr The remote addr
/// @param[out]  out_channel The service port.
/// @return Error
////////////
NetworkLibrary::Error GetChannelFromServiceUUID(UUID const& uuid, BasicAddr const& remote_addr, uint8_t& out_channel);

class BluetoothAddr :
    public BasicAddr
{
    class BluetoothAddrImpl* _Impl;

public:
    ////////////
    /// @brief 
    ////////////
    BluetoothAddr();
    ////////////
    /// @brief 
    ////////////
    BluetoothAddr(BluetoothAddr const& other);
    ////////////
    /// @brief 
    ////////////
    BluetoothAddr(BluetoothAddr&& other) noexcept;
    ////////////
    /// @brief 
    ////////////
    BluetoothAddr& operator=(BluetoothAddr const& other);
    ////////////
    /// @brief 
    ////////////
    BluetoothAddr& operator=(BluetoothAddr&& other) noexcept;

    ////////////
    /// @brief 
    ////////////
    virtual ~BluetoothAddr();
    ////////////
    /// @brief Transforms the address to a human readable string
    /// @param[in] with_port Append the port
    /// @return The string representation of the address
    ////////////
    virtual std::string ToString(bool with_port = false) const;
    ////////////
    /// @brief Get this Addr family type.
    /// @return 
    ////////////
    virtual int GetFamily() const;
    ////////////
    /// @brief Get this Addr sockaddr.
    /// @return Reference
    ////////////
    virtual void* GetAddr();
    ////////////
    /// @brief Get this Addr sockaddr.
    /// @return Const reference
    ////////////
    virtual const void* GetAddr() const;
    ////////////
    /// @brief Get this Addr length.
    /// @return Addr length
    ////////////
    virtual size_t GetLength() const;

    ////////////
    /// @brief Fill this BluetoothAddr from string representation.
    /// @param[in] str The string Bluetooth representation.
    /// @return Error code.
    ////////////
    NetworkLibrary::Error FromString(std::string str);
    ////////////
    /// @brief Set this BluetoothAddr ip.
    /// @param[in] addr The host ordered Bluetooth address.
    /// @return
    ////////////
    void SetAddress(BluetoothMac addr);
    ////////////
    /// @brief Set this BluetoothAddr channel.
    /// @param[in] channel The Bluetooth channel.
    /// @return
    ////////////
    void SetChannel(uint8_t channel);
    ////////////
    /// @brief Get this BluetoothAddr host ordered address.
    /// @return Host ordered address.
    ////////////
    BluetoothMac GetAddress() const;
    ////////////
    /// @brief Get this BluetoothAddr channel.
    /// @return Bluetooth channel
    ////////////
    uint8_t GetChannel() const;
    ////////////
    /// @brief Set this BluetoothAddr any address.
    /// @return
    ////////////
    void SetAnyAddr();
    ////////////
    /// @brief Set this BluetoothAddr local address.
    /// @return
    ////////////
    void SetLocalAddr();
};

class RFCOMM :
    public ConnectedSocket
{
public:
    RFCOMM();
    RFCOMM(RFCOMM const& other) = delete;
    RFCOMM(RFCOMM&& other) noexcept;
    RFCOMM& operator=(RFCOMM const& other) = delete;
    RFCOMM& operator=(RFCOMM&& other) noexcept;

    ////////////
    /// @brief Allocates resources to use network functions.
    /// @return Error
    ////////////
    NetworkLibrary::Error CreateSocket();
    ////////////
    /// @brief Gets this socket addr (if any).
    /// @param[out] out_addr Socket address
    /// @return Error
    ////////////
    NetworkLibrary::Error GetSockName(BluetoothAddr& out_addr);

    virtual int GetFamily() const;
    virtual int GetType() const;
    virtual int GetProto() const;
};

}
}