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
#include <initguid.h>

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
        // paramètres entrants : bdaddr_t&
        // paramètres sortants : std::string : l'adresse forme pointée
        ////////////////////////////////////////////////////////////////////////////////
        static std::string inet_ntoa(bdaddr_t& in);
        ////////////////////////////////////////////////////////////////////////////////
        // Méthode : isValidUUID
        // Usage   : vérifie si la chaine est bien au format UUID
        // paramètres entrants : std::string const& struuid
        // paramètres sortants : bool
        // UUID must be formatted like : [0-9a-zA-Z]{8}-([0-9a-zA-Z]{4}-){3}-[0-9a-zA-Z]{12}
        ////////////////////////////////////////////////////////////////////////////////
        static bool isValidUUID(std::string const& struuid);
        ////////////////////////////////////////////////////////////////////////////////
        // Méthode : str2uuid
        // Usage   : Transforme une chaine en uuid
        // paramètres entrants : std::string const& struuid
        // paramètres sortants : uuid_t
        ////////////////////////////////////////////////////////////////////////////////
        static uuid_t str2uuid(std::string const& struuid);
        ////////////////////////////////////////////////////////////////////////////////
        // Méthode : uuid2str
        // Usage   : Transforme un uuid en chaine
        // paramètres entrants : uuid_t const&
        // paramètres entrants : std::string
        ////////////////////////////////////////////////////////////////////////////////
        static std::string uuid2str( uuid_t const& uuid );
        ////////////////////////////////////////////////////////////////////////////////
        // Méthode : scan
        // Usage   : Scan les périphériques bluetooth à proximité
        // paramètres entrants : bool
        // paramètres sortants : std::list<bluetoothDevice> : les appareils trouvés
        ////////////////////////////////////////////////////////////////////////////////
        static std::list<BluetoothDevice> scan(bool flushCache = true);
        ////////////////////////////////////////////////////////////////////////////////
        // Méthode : scanOpenPortFromUUID
        // Usage   : Recherche le canal ouvert du serveur qui a enregistrer l'UUID
        // paramètres entrants : uuid_t &uuid, bdaddr_t &bthaddr
        // paramètres sortants : int : port
        ////////////////////////////////////////////////////////////////////////////////
        static int scanOpenPortFromUUID(uuid_t &uuid, bdaddr_t &bthaddr);
        ////////////////////////////////////////////////////////////////////////////////
        // Méthode : register_sdp_service
        // Usage   : Enregistre le service dans le SDP
        // paramètres entrants : service_t & service, uuid_t uuid, uint8_t port, std::string const&srv_name, std::string const&srv_prov, std::string const&srv_desc
        // paramètres sortants : ///////////
        // exceptions : wsa_not_initialised, error_in_value, socket_exception
        ////////////////////////////////////////////////////////////////////////////////
        static void register_sdp_service(service_t & service, uuid_t uuid, uint8_t port, std::string const&srv_name, std::string const&srv_prov, std::string const&srv_desc);
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
        uuid_t _uuid;
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

        void registerService(uuid_t uuid, uint8_t port, std::string const&name, std::string const&prov, std::string const&desc);
        void unregisterService();
        bool is_registered() const;
        uuid_t const& get_uuid() const;
        std::string const& get_name() const;
        std::string const& get_description() const;
        std::string const& get_provider() const;
    };
}
#endif
