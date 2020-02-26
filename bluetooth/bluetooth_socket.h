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

#ifndef __BLUETOOTH_SOCKET_INCLUDED__
#define __BLUETOOTH_SOCKET_INCLUDED__

#include <Socket/common/socket.h>

#if defined(__LINUX__)

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

#ifdef __BLUEZ_4__
typedef sdp_session_t* service_t;
#else
typedef std::string service_t;
#endif

#elif defined(__WINDOWS__)

#include <ws2bth.h>
#include <bluetoothapis.h>
#include <initguid.h>

#pragma comment(lib, "Bthprops.lib")

typedef GUID uuid_t;
typedef BTH_ADDR bdaddr_t;
typedef SOCKADDR_BTH sockaddr_rc;
typedef WSAQUERYSETW service_t;

#endif

#include <list>

namespace PortableAPI
{
#define SOCKET_EXCEPTION_CLASS(x) \
class LOCAL_API x : public socket_exception \
{\
public:\
    x(const char* mywhat):socket_exception(mywhat){}\
    x(std::string const& mywhat):socket_exception(mywhat){}\
}

    SOCKET_EXCEPTION_CLASS(sdp_service_exception);

#undef SOCKET_EXCEPTION_CLASS

    class LOCAL_API BluetoothDevice
    {
    public:
        bdaddr_t addr;
        std::string name;

        BluetoothDevice()                            = default;
        BluetoothDevice(BluetoothDevice const&)      = default;
        BluetoothDevice(BluetoothDevice &&) noexcept = default;

        BluetoothDevice& operator=(BluetoothDevice const&)      = default;
        BluetoothDevice& operator=(BluetoothDevice &&) noexcept = default;
    };

    class LOCAL_API Uuid
    {
    public:
        enum class type : uint8_t
        {
            none,
            uuid16,
            uuid32,
            uuid128
        };

    private:
        Uuid::type _type;
        uuid_t _uuid;

    public:
        ////////////////////////////////////////////////////////////////////////////////
        // Méthode : uuid16_to_uuid128
        // Usage   : Transforme un uuid16 en uuid128 (base bluetooth)
        // paramètres entrants : uint16_t
        // paramètres sortants : aucun
        ////////////////////////////////////////////////////////////////////////////////
        void uuid16_to_uuid128(uint16_t uuid);
        ////////////////////////////////////////////////////////////////////////////////
        // Méthode : uuid32_to_uuid128
        // Usage   : Transforme un uuid32 en uuid128 (base bluetooth)
        // paramètres entrants : uint16_t
        // paramètres sortants : aucun
        ////////////////////////////////////////////////////////////////////////////////
        void uuid32_to_uuid128(uint32_t uuid);
        ////////////////////////////////////////////////////////////////////////////////
        // Méthode : uuid128_to_uuid16
        // Usage   : Transforme un uuid128 en uuid16 (base bluetooth)
        // paramètres entrants : uuid_t
        // paramètres sortants : aucun
        ////////////////////////////////////////////////////////////////////////////////
        void uuid128_to_uuid16(uuid_t const& uuid);
        ////////////////////////////////////////////////////////////////////////////////
        // Méthode : uuid128_to_uuid32
        // Usage   : Transforme un uuid128 en uuid32 (base bluetooth)
        // paramètres entrants : uuid_t
        // paramètres sortants : aucun
        ////////////////////////////////////////////////////////////////////////////////
        void uuid128_to_uuid32(uuid_t const& uuid);
        ////////////////////////////////////////////////////////////////////////////////
        // Méthode : isValidUUID
        // Usage   : vérifie si la chaine est bien au format UUID
        // paramètres entrants : std::string const& struuid
        // paramètres sortants : bool
        // UUID must be formatted like : [0-9a-zA-Z]{8}-([0-9a-zA-Z]{4}-){3}-[0-9a-zA-Z]{12}
        ////////////////////////////////////////////////////////////////////////////////
        static bool isValidUUID(std::string const& struuid);
        ////////////////////////////////////////////////////////////////////////////////
        // Méthode : from_string
        // Usage   : Transforme une chaine en uuid
        // paramètres entrants : std::string const& struuid
        // paramètres sortants : aucun
        ////////////////////////////////////////////////////////////////////////////////
        void from_string(std::string const& struuid);
        ////////////////////////////////////////////////////////////////////////////////
        // Méthode : to_string
        // Usage   : Transforme un uuid en chaine
        // paramètres entrants : aucun
        // paramètres entrants : std::string
        ////////////////////////////////////////////////////////////////////////////////
        std::string to_string();

        void set_uuid128(uuid_t const& uuid);

        bool operator ==(Uuid const& other);
        bool operator !=(Uuid const& other);

        uuid_t const& get_native_uuid() const;

        Uuid();
        Uuid(uuid_t const& uuid);
    };

    class LOCAL_API BluetoothSocket : public Socket
    {
    public:
        ////////////////////////////////////////////////////////////////////////////////
        // namespace : address_family
        // Usage : socket address family
        ////////////////////////////////////////////////////////////////////////////////
        enum class address_family
        {
#if defined(__WINDOWS__)
            bth = AF_BTH,
#elif defined(__LINUX__)
            bth = AF_BLUETOOTH,
#endif
        };

        ////////////////////////////////////////////////////////////////////////////////
        // namespace : protocols
        // Usage : socket protocols
        ////////////////////////////////////////////////////////////////////////////////
        enum class protocols
        {
#if defined(__WINDOWS__)
            rfcomm = BTHPROTO_RFCOMM,
            l2cap = BTHPROTO_L2CAP,
#elif defined(__LINUX__)
            rfcomm = BTPROTO_RFCOMM,
            l2cap = BTPROTO_L2CAP,
#endif
        };

#if defined(__WINDOWS__)
        //                                                                                 Here, Windows is Host ordered
        static constexpr uint8_t bth_base_uuid[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB };
#elif defined(__LINUX__)
        //                                                                                 Here, Linux is big endian
        static constexpr uint8_t bth_base_uuid[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB };
#endif

        ////////////////////////////////////////////////////////////////////////////////
        // Méthode : inet_addr
        // Usage   : Transforme l'adresse bluetooth sous forme de chaine en unsigned long (4octets)
        // paramètres entrants : std::string& addr : addresse forme pointée
        // paramètres sortants : bdaddr_t : l'adresse en 6 octets
        ////////////////////////////////////////////////////////////////////////////////
        static bdaddr_t inet_addr(std::string const& addr);
        ////////////////////////////////////////////////////////////////////////////////
        // Méthode : inet_ntoa
        // Usage   : Transforme l'adresse en format chaine
        // paramètres entrants : bdaddr_t const&
        // paramètres sortants : std::string : l'adresse forme pointée
        ////////////////////////////////////////////////////////////////////////////////
        static std::string inet_ntoa(bdaddr_t const& in);
        ////////////////////////////////////////////////////////////////////////////////
        // Méthode : scan
        // Usage   : Scan les périphériques bluetooth à proximité
        // paramètres entrants : bool
        // paramètres sortants : std::list<bluetoothDevice> : les appareils trouvés
        ////////////////////////////////////////////////////////////////////////////////
        static std::list<BluetoothDevice> scan(bool flushCache = true);
        ////////////////////////////////////////////////////////////////////////////////
        // Méthode : scanOpenPortFromUUID
        // Usage   : Recherche le canal ouvert du serveur qui a enregistré l'UUID
        // paramètres entrants : Uuid const&uuid, bdaddr_t const&bthaddr
        // paramètres sortants : int : port
        ////////////////////////////////////////////////////////////////////////////////
        static int scanOpenPortFromUUID(Uuid const&uuid, bdaddr_t const&bthaddr);
        ////////////////////////////////////////////////////////////////////////////////
        // Méthode : register_sdp_service
        // Usage   : Enregistre le service dans le SDP
        // paramètres entrants : service_t & service, uuid_t const& uuid, uint8_t port, std::string const&srv_name, std::string const&srv_prov, std::string const&srv_desc
        // paramètres sortants : ///////////
        // exceptions : wsa_not_initialised, error_in_value, socket_exception
        ////////////////////////////////////////////////////////////////////////////////
        static void register_sdp_service(service_t & service, uuid_t const& uuid, uint8_t port, std::string const&srv_name, std::string const&srv_prov, std::string const&srv_desc);
        ////////////////////////////////////////////////////////////////////////////////
        // Méthode : register_sdp_service
        // Usage   : Arrête le service SDP précédemment enregistré
        // paramètres entrants : service_t & service
        // paramètres sortants : ///////////
        // exceptions : wsa_not_initialised, error_in_value, socket_exception
        ////////////////////////////////////////////////////////////////////////////////
        static void unregister_sdp_service(service_t &service);
    };

    class LOCAL_API SDPService
    {
        friend class BluetoothSocket;

        bool _registered;
        service_t *_service;
        Uuid _uuid;
        std::string _name;
        std::string _description;
        std::string _provider;

        SDPService(SDPService const&) = delete;
        SDPService& operator =(SDPService const&) = delete;

    public:
        SDPService();
        SDPService(SDPService &&) noexcept;
        SDPService& operator =(SDPService &&) noexcept;
        ~SDPService();

        void registerService(Uuid const& uuid, uint8_t port, std::string const&name, std::string const&prov, std::string const&desc);
        void unregisterService();
        bool is_registered() const;
        Uuid const& get_uuid() const;
        std::string const& get_name() const;
        std::string const& get_description() const;
        std::string const& get_provider() const;
    };
}
#endif
