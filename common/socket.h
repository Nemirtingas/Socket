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

#ifndef __SOCKET_INCLUDED__
#define __SOCKET_INCLUDED__

#include <utils/utils.h>

#if defined(__WINDOWS__)

	#define VC_EXTRALEAN
	#define WIN32_LEAN_AND_MEAN
	#include <WinSock2.h>
	#include <Ws2tcpip.h>
	#ifdef max
		#undef max
	#endif

	#pragma comment(lib, "ws2_32.lib")
#elif defined(__LINUX__)

	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/wait.h>
	#include <sys/socket.h>
	#include <sys/un.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <sys/ioctl.h>
	#include <signal.h>
	#include <sys/poll.h>
	#include <netdb.h>

	typedef int32_t SOCKET;
	#define INVALID_SOCKET static_cast<SOCKET>(-1);
	struct WSAData {};
#endif


#include <cstdint>
#include <string>
#include <vector>
#include <exception>
#include <limits>

struct Socket_Endian
{
    static bool little()
    {
        union {
            uint32_t i;
            uint8_t c[4];
        } check = { 0x12345678 };
        return check.c[0] == 0x78;
    }

    static bool big()
    {
        union {
            uint32_t i;
            uint8_t c[4];
        } check = { 0x12345678 };
        return check.c[0] == 0x12;
    }
};

namespace PortableAPI
{
	class LOCAL_API socket_exception : public std::exception
	{
		std::string mywhat;
	public:
		socket_exception();
		socket_exception(const char* mywhat);
		socket_exception(std::string const& mywhat);
		virtual const char* what() const noexcept;
	};

#define SOCKET_EXCEPTION_CLASS(x, def_msg) \
class LOCAL_API x : public socket_exception \
{\
public:\
	x(const char* mywhat = def_msg):socket_exception(mywhat){}\
	x(std::string const& mywhat):socket_exception(mywhat){}\
}

	// Windows specific exceptions
	SOCKET_EXCEPTION_CLASS(wsa_not_initialised, "A successful WSAStartup call must occur before using this function.");      // WSANOTINITIALISED
	SOCKET_EXCEPTION_CLASS(wsa_net_down, "The network subsystem has failed.");                                               // WSAENETDOWN
	SOCKET_EXCEPTION_CLASS(wsa_sys_not_ready, "The underlying network subsystem is not ready for network communication.");   // WSASYSNOTREADY
	SOCKET_EXCEPTION_CLASS(wsa_version_not_supported, "The version of Windows Sockets support requested is not provided by this particular Windows Sockets implementation."); // WSAVERNOTSUPPORTED
	SOCKET_EXCEPTION_CLASS(wsa_in_progress, "A blocking Windows Sockets 1.1 operation is in progress.");                     // WSAEINPROGRESS
	SOCKET_EXCEPTION_CLASS(wsa_proclim, "A limit on the number of tasks supported by the Windows Sockets implementation has been reached."); // WSAEPROCLIM
	SOCKET_EXCEPTION_CLASS(wsa_fault, "The lpWSAData parameter is not a valid pointer.");                                    // WSAEFAULT

	// Generic exceptions
	SOCKET_EXCEPTION_CLASS(address_in_use, "Another socket is already listening on the same port.");      // EADDRINUSE   - WSAEADDRINUSE
	SOCKET_EXCEPTION_CLASS(connection_refused, "No-one listening on the remote address.");                // ECONNREFUSED - WSAECONNREFUSED
	SOCKET_EXCEPTION_CLASS(connection_reset, "Connection reset by peer.");                                // ECONNRESET   - WSAECONNRESET
	SOCKET_EXCEPTION_CLASS(connection_timeout, "A connection attempt failed because the connected party did not properly respond after a period of time.");  //              - WSAETIMEDOUT
	SOCKET_EXCEPTION_CLASS(error_in_value, "Error in value");                                             // EINVAL       - WSAEINVAL
	SOCKET_EXCEPTION_CLASS(is_connected, "A connect request was made on an already-connected socket.");   // EISCONN      - WSAEISCONN
	SOCKET_EXCEPTION_CLASS(network_unreachable, "Network is unreachable.");                               // ENETUNREACH  - WSAENETUNREACH
	SOCKET_EXCEPTION_CLASS(not_connected, "The socket is not connected.");                                // ENOTCONN     - WSAENOTCONN
	SOCKET_EXCEPTION_CLASS(would_block, "The operation would block");                                     // EWOULDBLOCK  - WSAEWOULDBLOCK

#undef SOCKET_EXCEPTION_CLASS

    ////////////////////////////////////////////////////////////////////////////////
    // Classe  : ByteSwapImpl
    // Usage   : Implementation d'�change d'octets en fonction de la taille de param�tre
    // param�tres entrants : T
    // param�tres sortants : T
    ////////////////////////////////////////////////////////////////////////////////
    template<typename T, size_t byte_count>
    struct ByteSwapImpl
    {};

    template<typename T>
    struct ByteSwapImpl<T, 1>
    {
        inline T operator()(T v) { return v; }
    };

    template<typename T>
    struct ByteSwapImpl<T, 2>
    {
        inline T operator()(T v)
        {
            uint16_t& tmp = *reinterpret_cast<uint16_t*>(&v);
            if (Socket_Endian::little())
            {
                tmp = ((tmp & 0x00ff) << 8)
                    | ((tmp & 0xff00) >> 8);
            }
            return v;
        }
    };

    template<typename T>
    struct ByteSwapImpl<T, 4>
    {
        inline T operator()(T v)
        {
            uint32_t& tmp = *reinterpret_cast<uint32_t*>(&v);
            if (Socket_Endian::little())
            {
                tmp = ((tmp & 0x000000ff) << 24)
                    | ((tmp & 0x0000ff00) << 8)
                    | ((tmp & 0x00ff0000) >> 8)
                    | ((tmp & 0xff000000) >> 24);
            }
            return v;
        }
    };

    template<typename T>
    struct ByteSwapImpl<T, 8>
    {
        inline T operator()(T v)
        {
            uint64_t& tmp = *reinterpret_cast<uint64_t*>(&v);
            if (Socket_Endian::little())
            {
                tmp = ((tmp & 0x00000000000000ff) << 56)
                    | ((tmp & 0x000000000000ff00) << 40)
                    | ((tmp & 0x0000000000ff0000) << 24)
                    | ((tmp & 0x00000000ff000000) << 8)
                    | ((tmp & 0x000000ff00000000) >> 8)
                    | ((tmp & 0x0000ff0000000000) >> 24)
                    | ((tmp & 0x00ff000000000000) >> 40)
                    | ((tmp & 0xff00000000000000) >> 56);
            }
            return v;
        }
    };

	class LOCAL_API basic_addr
	{
	public:
		virtual ~basic_addr();
		virtual std::string toString() const = 0;
		virtual void fromString(std::string const&) = 0;
		virtual sockaddr& addr() = 0;
		virtual size_t len() const = 0;
		virtual void set_any_addr() = 0;
	};

	class LOCAL_API Socket
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// namespace : address_family
		// Usage : socket address family
		////////////////////////////////////////////////////////////////////////////////
		enum class address_family
		{
#if defined(__WINDOWS__)
			implink = AF_IMPLINK,
			pip = AF_PUP,
			chaos = AF_CHAOS,
			ns = AF_NS,
			uso = AF_ISO,
			osi = AF_OSI,
			ecma = AF_ECMA,
			datakit = AF_DATAKIT,
			ccitt = AF_CCITT,
			dli = AF_DLI,
			lat = AF_LAT,
			hylink = AF_HYLINK,
			netbios = AF_NETBIOS,
			voiceview = AF_VOICEVIEW,
			firefox = AF_FIREFOX,
			unknown1 = AF_UNKNOWN1,
			ban = AF_BAN,
			atm = AF_ATM,
			cluster = AF_CLUSTER,
			IEEE1284_4 = AF_12844,
			netdes = AF_NETDES,
#endif
            unix = AF_UNIX,
			inet = AF_INET,
			ipx = AF_IPX,
			sna = AF_SNA,
			decnet = AF_DECnet,
			appletalk = AF_APPLETALK,
			inet6 = AF_INET6,
			irda = AF_IRDA,
			unspec = AF_UNSPEC,
		};

		////////////////////////////////////////////////////////////////////////////////
		// namespace : socket_flags
		// Usage : socket read and write flags
		////////////////////////////////////////////////////////////////////////////////
		enum class socket_flags
		{
			normal = 0,
			oob = MSG_OOB,// process out-of-band data
			peek = MSG_PEEK,// peek at incoming message
			dontroute = MSG_DONTROUTE,//send without using routing tables
#if(_WIN32_WINNT >= 0x0502)
			waitall = MSG_WAITALL,//do not complete until packet is completely filled
#endif
		};

		////////////////////////////////////////////////////////////////////////////////
		//  namespace : types
		//  Usage : socket types
		////////////////////////////////////////////////////////////////////////////////
		enum class types
		{
			stream = SOCK_STREAM,
			dgram = SOCK_DGRAM,
			raw = SOCK_RAW,
			rdm = SOCK_RDM,
			seqpacket = SOCK_SEQPACKET,
		};

		////////////////////////////////////////////////////////////////////////////////
		// namespace : protocols
		// Usage : socket protocols
		////////////////////////////////////////////////////////////////////////////////
		enum class protocols
		{
			icmp = IPPROTO_ICMP,
			igmp = IPPROTO_IGMP,
			tcp = IPPROTO_TCP,
			pup = IPPROTO_PUP,
			udp = IPPROTO_UDP,
			idp = IPPROTO_IDP,
			raw = IPPROTO_RAW,
			max = IPPROTO_MAX,
#if defined(__LINUX__)
			hopopts = IPPROTO_HOPOPTS,
			routing = IPPROTO_ROUTING,
			fragment = IPPROTO_FRAGMENT,
			icmpv6 = IPPROTO_ICMPV6,
			none = IPPROTO_NONE,
			dstopts = IPPROTO_DSTOPTS,
			//mh = IPPROTO_MH,
			ip = IPPROTO_IP,
			ipip = IPPROTO_IPIP,
			egp = IPPROTO_EGP,
			tp = IPPROTO_TP,
			dccp = IPPROTO_DCCP,
			ipv6 = IPPROTO_IPV6,
			rsvp = IPPROTO_RSVP,
			gre = IPPROTO_GRE,
			esp = IPPROTO_ESP,
			ah = IPPROTO_AH,
			mtp = IPPROTO_MTP,
			//beetph = IPPROTO_BEETPH,
			encap = IPPROTO_ENCAP,
			pim = IPPROTO_PIM,
			comp = IPPROTO_COMP,
			sctp = IPPROTO_SCTP,
			udplite = IPPROTO_UDPLITE,
#elif defined(__WINDOWS__)
			ggp = IPPROTO_GGP,
			nd = IPPROTO_ND,
#if(_WIN32_WINNT >= 0x0501)
			hopopts = IPPROTO_HOPOPTS,
			ipv4 = IPPROTO_IPV4,
			ipv6 = IPPROTO_IPV6,
			routin = IPPROTO_ROUTING,
			fragment = IPPROTO_FRAGMENT,
			esp = IPPROTO_ESP,
			ah = IPPROTO_AH,
			icmpv6 = IPPROTO_ICMPV6,
			none = IPPROTO_NONE,
			dstopts = IPPROTO_DSTOPTS,
			iclfxbm = IPPROTO_ICLFXBM,
#endif//(_WIN32_WINNT >= 0x0501)
#if(_WIN32_WINNT >= 0x0600)
			st = IPPROTO_ST,
			cbt = IPPROTO_CBT,
			egp = IPPROTO_EGP,
			igp = IPPROTO_IGP,
			rdp = IPPROTO_RDP,
			pim = IPPROTO_PIM,
			pgm = IPPROTO_PGM,
			l2tp = IPPROTO_L2TP,
			sctp = IPPROTO_SCTP,
#endif//(_WIN32_WINNT >= 0x0600)
#endif
		};

        enum class level
        {
            sol_socket = SOL_SOCKET,
        };

        enum class option_name : uint32_t
        {
#if defined(__WINDOWS__)
            so_debug = SO_DEBUG,
            so_acceptconn = SO_ACCEPTCONN,
            so_reuseaddr = SO_REUSEADDR,
            so_keepalive = SO_KEEPALIVE,
            so_dontroute = SO_DONTROUTE,
            so_broadcast = SO_BROADCAST,
            so_useloopback = SO_USELOOPBACK,
            so_linger = SO_LINGER,
            so_oobinline = SO_OOBINLINE,
            so_sndbuf = SO_SNDBUF,
            so_rcvbuf = SO_RCVBUF,
            so_sndlowat = SO_SNDLOWAT,
            so_rcvlowat = SO_RCVLOWAT,
            so_sndtimeo = SO_SNDTIMEO,
            so_rcvtimeo = SO_RCVTIMEO,
            so_error = SO_ERROR,
            so_type = SO_TYPE,
            so_group_id = SO_GROUP_ID,
            so_group_priority = SO_GROUP_PRIORITY,
            so_max_msg_size = SO_MAX_MSG_SIZE,
            #ifdef UNICODE
                so_protocol_info = SO_PROTOCOL_INFOW,
            #else
                so_protocol_info = SO_PROTOCOL_INFOA,
            #endif /* UNICODE */
#elif defined(__LINUX__)
            so_debug = SO_DEBUG,
            so_reuseaddr = SO_REUSEADDR,
            so_keepalive = SO_KEEPALIVE,
            so_dontroute = SO_DONTROUTE,
            so_broadcast = SO_BROADCAST,
            so_linger = SO_LINGER,
            so_oobinline = SO_OOBINLINE,
            so_sndbuf = SO_SNDBUF,
            so_rcvbuf = SO_RCVBUF,
            so_sndlowat = SO_SNDLOWAT,
            so_rcvlowat = SO_RCVLOWAT,
            so_sndtimeo = SO_SNDTIMEO,
            so_rcvtimeo = SO_RCVTIMEO,
            so_error = SO_ERROR,
            so_type = SO_TYPE,
            so_sndbufforce = SO_SNDBUFFORCE,
            so_rcvbufforce = SO_RCVBUFFORCE,
            so_no_check = SO_NO_CHECK,
            so_priority = SO_PRIORITY,
            so_bsdcompat = SO_BSDCOMPAT,
            so_reuseport = SO_REUSEPORT,
            so_passcred = SO_PASSCRED,
            so_peercred = SO_PEERCRED,
#endif
        };

        enum class cmd_name : uint32_t
        {
#if defined(__WINDOWS__)
            fionread = FIONREAD,
            fionbio = FIONBIO,
            fioasync = FIOASYNC,
#elif defined(__LINUX__)
            fionread = FIONREAD,
            fionbio = FIONBIO,
            fioasync = FIOASYNC,
#endif
        };

		////////////////////////////////////////////////////////////////////////////////
		// namespace : shutdown_flags
		// Usage : socket shutdown flags
		////////////////////////////////////////////////////////////////////////////////
		enum class shutdown_flags
		{
#if defined(__LINUX__)
			reveive = SHUT_RD,
			send = SHUT_WR,
			both = SHUT_RDWR,
#elif defined(__WINDOWS__)
			reveive = SD_RECEIVE,
			send = SD_SEND,
			both = SD_BOTH,
#endif
		};

#if _WIN32_WINNT >= 0x0600 || defined(__LINUX__)
		////////////////////////////////////////////////////////////////////////////////
		// namespace : poll_flags
		// Usage : socket poll flags
		////////////////////////////////////////////////////////////////////////////////
		enum class poll_flags
		{
			rdnorm = POLLRDNORM,
			rdband = POLLRDBAND,
			in = POLLIN,
			pri = POLLPRI,
			wrnorm = POLLWRNORM,
			out = POLLOUT,
			wrband = POLLWRBAND,
			err = POLLERR,
			hup = POLLHUP,
			nval = POLLNVAL,
		};
#endif

		constexpr static SOCKET invalid_socket = INVALID_SOCKET;

		////////////////////////////////////////////////////////////////////////////////
		// M�thode : accept
		// Usage   : Accepte la connexion d'un client sur s et r�cup�re les informations client
		// param�tres entrants : SOCKET s, basic_addr &addr
		// param�tres sortants : SOCKET : client
		////////////////////////////////////////////////////////////////////////////////
		static SOCKET accept(SOCKET s, basic_addr &addr);
		////////////////////////////////////////////////////////////////////////////////
		// M�thode : bind
		// Usage   : Associe l'adresse local_addr au s
		// param�tres entrants : SOCKET s, basic_addr &addr
		// param�tres sortants : aucun
		////////////////////////////////////////////////////////////////////////////////
		static void bind(SOCKET s, basic_addr &addr);
		////////////////////////////////////////////////////////////////////////////////
		// M�thode : closeSocket
		// Usage   : Lib�re les ressources du socket s
		// param�tres entrants : SOCKET s
		// param�tres sortants : aucun
		////////////////////////////////////////////////////////////////////////////////
		static void closeSocket(SOCKET s);
		////////////////////////////////////////////////////////////////////////////////
		// M�thode : connect
		// Usage   : Connect le s � l'adresse remote_addr
		// param�tres entrants : SOCKET s, basic_addr &addr
		// param�tres sortants : aucun
		////////////////////////////////////////////////////////////////////////////////
		static void connect(SOCKET s, basic_addr &addr);
		////////////////////////////////////////////////////////////////////////////////
		// M�thode : ioctlsocket
		// Usage   : Permet de modifier les param�tres d'un socket (non bloquant etc...)
		// param�tres entrants : SOCKET s, long cmd, unsigned long* arg
		// param�tres sortants : int : erreur
		////////////////////////////////////////////////////////////////////////////////
		static int ioctlsocket(SOCKET s, long cmd, unsigned long* arg);
        ////////////////////////////////////////////////////////////////////////////////
        // M�thode : setsockopt
        // Usage   : Permet de modifier les options d'un socket (broadcast etc...)
        // param�tres entrants : SOCKET s, int level, int optname, const void* optval, int optlen
        // param�tres sortants : int : erreur
        ////////////////////////////////////////////////////////////////////////////////
        static int setsockopt(SOCKET s, int level, int optname, const void* optval, int optlen);
        ////////////////////////////////////////////////////////////////////////////////
        // M�thode : getsockopt
        // Usage   : Permet de lire les options d'un socket (broadcast etc...)
        // param�tres entrants : SOCKET s, int level, int optname, void* optval, int* optlen
        // param�tres sortants : int : erreur
        ////////////////////////////////////////////////////////////////////////////////
        static int getsockopt(SOCKET s, int level, int optname, void* optval, int* optlen);
		////////////////////////////////////////////////////////////////////////////////
		// M�thode : listen
		// Usage   : Permet � s d'�tre en �coute et d'accepter les clients
		// param�tres entrants : SOCKET s, int waiting_connection = 5
		// param�tres sortants : aucun
		////////////////////////////////////////////////////////////////////////////////
		static void listen(SOCKET s, int waiting_connection = 5);
		////////////////////////////////////////////////////////////////////////////////
		// M�thode : recv
		// Usage   : re�oit des donn�es sur s (mode connect�)
		// param�tres entrants : SOCKET s, void* buffer, size_t len, Socket::socket_flags flags = Socket::socket_flags::normal
		// param�tres sortants : size_t : taille des donn�es
		////////////////////////////////////////////////////////////////////////////////
		static size_t recv(SOCKET s, void* buffer, size_t len, Socket::socket_flags flags = Socket::socket_flags::normal);
		////////////////////////////////////////////////////////////////////////////////
		// M�thode : recvfrom
		// Usage   : re�oit des donn�es sur s et r�cup�re la provenance (mode d�connect�)
		// param�tres entrants : SOCKET s, basic_addr &addr, void* buffer, size_t len, Socket::socket_flags flags = Socket::socket_flags::normal
		// param�tres sortants : size_t : taille des donn�es
		////////////////////////////////////////////////////////////////////////////////
		static size_t recvfrom(SOCKET s, basic_addr &addr, void* buffer, size_t len, Socket::socket_flags flags = Socket::socket_flags::normal);
		////////////////////////////////////////////////////////////////////////////////
		// M�thode : send
		// Usage   : Envoie des donn�es sur s (mode connect�)
		// param�tres entrants : SOCKET s, const void* buffer, size_t len, Socket::socket_flags flags = Socket::socket_flags::normal
		// param�tres sortants : size_t : taille des donn�es
		////////////////////////////////////////////////////////////////////////////////
		static size_t send(SOCKET s, const void* buffer, size_t len, Socket::socket_flags flags = Socket::socket_flags::normal);
		////////////////////////////////////////////////////////////////////////////////
		// M�thode : sendto
		// Usage   : Envoie des donn�es sur _Sock (mode d�connect�)
		// param�tres entrants : SOCKET s, basic_addr &addr, const void* buffer, size_t len, Socket::socket_flags flags = Socket::socket_flags::normal
		// param�tres sortants : size_t : taille des donn�es
		////////////////////////////////////////////////////////////////////////////////
		static size_t sendto(SOCKET s, basic_addr &addr, const void* buffer, size_t len, Socket::socket_flags flags = Socket::socket_flags::normal);
		////////////////////////////////////////////////////////////////////////////////
		// M�thode : shutdown
		// Usage   : Ferme la lecture, l'�criture ou les 2 de _Sock
		// param�tres entrants : SOCKET s, Socket::shutdown_flags how = Socket::shutdown_flags::both
		// param�tres sortants : int
		////////////////////////////////////////////////////////////////////////////////
		static int shutdown(SOCKET s, Socket::shutdown_flags how = Socket::shutdown_flags::both);
		////////////////////////////////////////////////////////////////////////////////
		// M�thode : socket
		// Usage   : Cr�e le socket avec les informations pass�es
		// param�tres entrants : Socket::address_family af, Socket::types type, Socket::protocols proto
		// param�tres sortants : SOCKET
		////////////////////////////////////////////////////////////////////////////////
		static SOCKET socket(Socket::address_family af, Socket::types type, Socket::protocols proto);
		////////////////////////////////////////////////////////////////////////////////
		// M�thode : getaddrinfo
		// Usage   : r�cup�re l'addrinfo en fonction du nom ou de la notation X.X.X.X
		// (IPV4|IPV6)
		// param�tres entrants : const char* _Nodename, const char* _Servicename, const addrinfo *_Hints, addrinfo **_Result
		// param�tres sortants : int ; erreur
		////////////////////////////////////////////////////////////////////////////////
		static int getaddrinfo(const char* _Nodename, const char* _Servicename, const addrinfo *_Hints, addrinfo **_Result);
		////////////////////////////////////////////////////////////////////////////////
		// M�thode : getnameinfo
		// Usage   : R�cup�re le nom de l'h�te en fonction de son _Addr
		// param�tres entrants : const sockaddr *_Addr, socklen_t _Addrlen, char *_Host, size_t _Hostlen, char *_Serv, size_t _Servlen, int _Flags
		// param�tres sortants : int : erreur
		////////////////////////////////////////////////////////////////////////////////
		static int getnameinfo(const sockaddr *_Addr, socklen_t _Addrlen, char *_Host, size_t _Hostlen, char *_Serv, size_t _Servlen, int _Flags);
		////////////////////////////////////////////////////////////////////////////////
		// M�thode : inet_addr
		// Usage   : Transforme l'adresse IPV4 sous forme de chaine en unsigned long (4octets)
		// param�tres entrants : std::string& addr : addresse forme point�e
		// param�tres sortants : unsigned long : l'adresse en 4 octets
		////////////////////////////////////////////////////////////////////////////////
		static uint32_t inet_addr(std::string const& addr);
		////////////////////////////////////////////////////////////////////////////////
		// M�thode : inet_ntoa
		// Usage   : Transforme l'adresse en format chaine
		// param�tres entrants : in_addr&
		// param�tres sortants : std::string : l'adresse forme point�e
		////////////////////////////////////////////////////////////////////////////////
		static std::string inet_ntoa(in_addr& _In);
		////////////////////////////////////////////////////////////////////////////////
		// M�thode : select
		// Usage   : Fait comme poll mais avec plus de limitations
		// param�tres entrants : int _Nfds, fd_set *_Readfd, fd_set *_Writefd, fd_set *_Exceptfd, timeval *_Timeout
		// param�tres sortants : int : nb de socket qui ont r�pondu au select
		////////////////////////////////////////////////////////////////////////////////
		static int select(int _Nfds, fd_set *_Readfd, fd_set *_Writefd, fd_set *_Exceptfd, timeval *_Timeout);
#if(_WIN32_WINNT >= 0x0600) || defined(__LINUX__)
		////////////////////////////////////////////////////////////////////////////////
		// M�thode : poll
		// Usage   : Fait un sondage sur quel socket de _Fds � des donn�es � lire ou 
		// �crire en fonction des flags
		// param�tres entrants : pollfd *_Fds, unsigned long _Nfds, int _Timeout
		// param�tres sortants : int : erreur
		////////////////////////////////////////////////////////////////////////////////
		static int poll(pollfd *_Fds, unsigned long _Nfds, int _Timeout);
#endif

		static void InitSocket(uint32_t version = 0x0202);
        static void SetLastError(int error);
		//static void FreeSocket();
		static WSAData const& GetWSAData();

		~Socket();

        template<typename T>
        static inline T net_swap(T v)
        {
            return ByteSwapImpl<T, sizeof(T)>()(v);
        }

		private:
			Socket(uint32_t version);
			Socket(Socket const&) = delete;
			Socket(Socket &&) = delete;
			Socket& operator=(Socket const&) = delete;
			Socket& operator=(Socket &&) = delete;
	}; 
}
#endif
