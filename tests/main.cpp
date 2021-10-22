#include <iostream>
#include <iomanip>
#include <thread>
#include <shared_mutex>
#include <chrono>
#include <condition_variable>
#include <cassert>
#include <exception>
#include <random>
#include <future>
#include <list>

#include <NetworkLibrary/Poll.h>
#include <NetworkLibrary/IPv4.h>
#include <NetworkLibrary/IPv6.h>
#ifdef UNIX_TESTS
#include <NetworkLibrary/Unix.h>
#endif
#ifdef BLUETOOTH_TESTS
#include <NetworkLibrary/Bluetooth.h>
#endif

#include <string.h>

using namespace std::chrono_literals;

void TestIPv4()
{
    auto ifaces = NetworkLibrary::IPv4::GetIfacesAddresses();
    if ((int)ifaces.first != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to get IPv4 network interfaces: " << ifaces.first.ToString() << std::endl;
        return;
    }
    for (auto& iface : ifaces.second)
    {
        std::cout << iface.Name << ' ' << iface.FriendlyName << ' ' << iface.IsUp << std::endl;
        for (auto& ip : iface.Addresses)
        {
            std::cout << "    " << ip.Addr << '/' << ip.MaskCIDR << std::endl;
        }
        std::cout << std::endl;
    }

    std::cout << __FUNCTION__ << " done !" << std::endl << std::endl;
}

void TestIPv4UDP()
{
    char buffer[1024];
    std::string addr_string;
    NetworkLibrary::IPv4::UDP udp1, udp2;
    NetworkLibrary::IPv4::IPv4Addr ipv4_addr;
    NetworkLibrary::Error error;
    NetworkLibrary::NetBuffer net_buff{ buffer, 0 };

    std::cout << __FUNCTION__ << std::endl;

    std::cout << "Parsing IPv4 UDP 0.0.0.0..." << std::endl;
    error = ipv4_addr.FromString("0.0.0.0");
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to parse IPv4(0.0.0.0) address: " << error.ToString() << std::endl;
        return;
    }
    addr_string = ipv4_addr.ToString(true);

    std::cout << "Parsing IPv4 UDP 127.0.0.1:9999..." << std::endl;
    error = ipv4_addr.FromString("127.0.0.1:9999");
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to parse IPv4(127.0.0.1:9999) address: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Creating IPv4 UDP socket1..." << std::endl;
    error = udp1.CreateSocket();
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to create IPv4 UDP socket1: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Creating IPv4 UDP socket2..." << std::endl;
    error = udp2.CreateSocket();
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to create IPv4 UDP socket2: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Binding IPv4 UDP socket on " << ipv4_addr.ToString() << "..." << std::endl;
    error = udp1.Bind(ipv4_addr);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to bind IPv4 UDP socket: " << error.ToString() << std::endl;
        return;
    }

    memcpy(net_buff.Buffer, "Hello from UDP client.", 23);
    net_buff.BufferSize = 23;

    std::cout << "Sending data to peer " << ipv4_addr.ToString(true) << ": " << buffer << "..." << std::endl;
    error = udp2.SendTo(ipv4_addr, net_buff);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to send IPv4 UDP datas: " << error.ToString() << std::endl;
        return;
    }

    memset(net_buff.Buffer, 0, 1024);
    net_buff.BufferSize = 1024;

    std::cout << "Receiving datas from peer..." << std::endl;
    error = udp1.ReceiveFrom(ipv4_addr, net_buff);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to received IPv4 UDP datas: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Received datas from peer " << ipv4_addr.ToString(true) << ": " << buffer << "..." << std::endl;

    std::cout << __FUNCTION__ << " done !" << std::endl << std::endl;
}

void TestIPv4TCP()
{
    char buffer[1024];
    std::string addr_string;
    NetworkLibrary::IPv4::TCP tcp1, tcp2, tcp3;
    NetworkLibrary::IPv4::IPv4Addr ipv4_addr;
    NetworkLibrary::Error error;
    NetworkLibrary::NetBuffer net_buff{ buffer, 0 };

    std::cout << __FUNCTION__ << std::endl;

    std::cout << "Parsing IPv4 TCP 0.0.0.0..." << std::endl;
    error = ipv4_addr.FromString("0.0.0.0");
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to parse IPv4(0.0.0.0) address: " << error.ToString() << std::endl;
        return;
    }
    addr_string = ipv4_addr.ToString(true);

    std::cout << "Parsing IPv4 TCP 127.0.0.1:9999..." << std::endl;
    error = ipv4_addr.FromString("127.0.0.1:9999");
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to parse IPv4(127.0.0.1:9999) address: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Creating IPv4 TCP socket1..." << std::endl;
    error = tcp1.CreateSocket();
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to create IPv4 TCP socket1: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Creating IPv4 TCP socket2..." << std::endl;
    error = tcp2.CreateSocket();
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to create IPv4 UDP socket2: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Binding IPv4 TCP socket on " << ipv4_addr.ToString() << "..." << std::endl;
    error = tcp1.Bind(ipv4_addr);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to bind IPv4 TCP socket: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Listening IPv4 TCP..." << std::endl;
    error = tcp1.Listen();
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to listen IPv4 TCP socket: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "TCP client connecting to " << ipv4_addr.ToString(true) << "..." << std::endl;
    error = tcp2.Connect(ipv4_addr);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to connect to " << ipv4_addr.ToString(true) << " : " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Accepting TCP client..." << std::endl;
    error = tcp1.Accept(tcp3, ipv4_addr);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to accept: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Accepted client " << ipv4_addr.ToString(true) << "." << std::endl;

    memcpy(net_buff.Buffer, "Hello from TCP client.", 23);
    net_buff.BufferSize = 23;

    std::cout << "Sending " << buffer << "..." << std::endl;
    error = tcp2.Send(net_buff);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to send IPv4 TCP datas: " << error.ToString() << std::endl;
        return;
    }

    memset(net_buff.Buffer, 0, 1024);
    net_buff.BufferSize = 1024;

    std::cout << "Receiving datas from TCP client..." << std::endl;
    error = tcp3.Receive(net_buff);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to received IPv4 TCP datas: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Received datas from client " << ipv4_addr.ToString(true) << ": " << buffer << "." << std::endl;

    memcpy(net_buff.Buffer, "Hello from TCP server.", 23);
    net_buff.BufferSize = 23;

    std::cout << "Sending " << buffer << "..." << std::endl;
    error = tcp3.Send(net_buff);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to send IPv4 TCP datas: " << error.ToString() << std::endl;
        return;
    }

    memset(net_buff.Buffer, 0, 1024);
    net_buff.BufferSize = 1024;

    std::cout << "Receiving datas from TCP server..." << std::endl;
    error = tcp2.Receive(net_buff);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to received IPv4 TCP datas: " << error.ToString() << std::endl;
        return;
    }

    tcp1.GetSockName(ipv4_addr);
    std::cout << "Received datas from server " << ipv4_addr.ToString(true) << " : " << buffer << "." << std::endl;

    std::cout << __FUNCTION__ << " done !" << std::endl << std::endl;
}

void TestIPv6()
{
    auto ifaces = NetworkLibrary::IPv6::GetIfacesAddresses();
    if ((int)ifaces.first != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to get IPv6 network interfaces: " << ifaces.first.ToString() << std::endl;
        return;
    }
    for (auto& iface : ifaces.second)
    {
        std::cout << iface.Name << ' ' << iface.FriendlyName << ' ' << iface.IsUp << std::endl;
        for (auto& ip : iface.Addresses)
        {
            std::cout << "    " << ip.Addr << '/' << ip.MaskCIDR << std::endl;
        }
        std::cout << std::endl;
    }

    std::cout << __FUNCTION__ << " done !" << std::endl << std::endl;
}

void TestIPv6UDP()
{
    char buffer[1024];
    std::string addr_string;
    NetworkLibrary::IPv6::UDP udp1, udp2;
    NetworkLibrary::IPv6::IPv6Addr ipv6_addr;
    NetworkLibrary::Error error;
    NetworkLibrary::NetBuffer net_buff{ buffer, 0 };

    std::cout << __FUNCTION__ << std::endl;

    std::cout << "Parsing IPv6 UDP ::..." << std::endl;
    error = ipv6_addr.FromString("::");
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to parse IPv6(::) address: " << error.ToString() << std::endl;
        return;
    }
    addr_string = ipv6_addr.ToString(true);

    std::cout << "Parsing IPv6 UDP [::1]:9999..." << std::endl;
    error = ipv6_addr.FromString("[::1]:9999");
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to parse IPv6([::1]:9999) address: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Creating IPv6 UDP socket1..." << std::endl;
    error = udp1.CreateSocket();
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to create IPv6 UDP socket1: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Creating IPv6 UDP socket2..." << std::endl;
    error = udp2.CreateSocket();
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to create IPv6 UDP socket2: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Binding IPv6 UDP socket on " << ipv6_addr.ToString() << "..." << std::endl;
    error = udp1.Bind(ipv6_addr);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to bind IPv6 UDP socket: " << error.ToString() << std::endl;
        return;
    }

    memcpy(net_buff.Buffer, "Hello from UDP client.", 23);
    net_buff.BufferSize = 23;

    std::cout << "Sending data to peer " << ipv6_addr.ToString(true) << ": " << buffer << "..." << std::endl;
    error = udp2.SendTo(ipv6_addr, net_buff);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to send IPv6 UDP datas: " << error.ToString() << std::endl;
        return;
    }

    memset(net_buff.Buffer, 0, 1024);
    net_buff.BufferSize = 1024;

    std::cout << "Receiving datas from peer..." << std::endl;
    error = udp1.ReceiveFrom(ipv6_addr, net_buff);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to received IPv6 UDP datas: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Received datas from peer " << ipv6_addr.ToString(true) << ": " << buffer << "..." << std::endl;

    std::cout << __FUNCTION__ << " done !" << std::endl << std::endl;
}

void TestIPv6TCP()
{
    char buffer[1024];
    std::string addr_string;
    NetworkLibrary::IPv6::TCP tcp1, tcp2, tcp3;
    NetworkLibrary::IPv6::IPv6Addr ipv6_addr;
    NetworkLibrary::Error error;
    NetworkLibrary::NetBuffer net_buff{ buffer, 0 };

    std::cout << __FUNCTION__ << std::endl;

    std::cout << "Parsing IPv6 TCP ::..." << std::endl;
    error = ipv6_addr.FromString("::");
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to parse IPv6(::) address: " << error.ToString() << std::endl;
        return;
    }
    addr_string = ipv6_addr.ToString(true);

    std::cout << "Parsing IPv6 TCP [::1]:9999..." << std::endl;
    error = ipv6_addr.FromString("[::1]:9999");
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to parse IPv6([::1]:9999) address: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Creating IPv6 TCP socket1..." << std::endl;
    error = tcp1.CreateSocket();
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to create IPv6 TCP socket1: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Creating IPv6 TCP socket2..." << std::endl;
    error = tcp2.CreateSocket();
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to create IPv6 UDP socket2: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Binding IPv6 TCP socket on " << ipv6_addr.ToString() << "..." << std::endl;
    error = tcp1.Bind(ipv6_addr);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to bind IPv6 TCP socket: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Listening IPv6 TCP..." << std::endl;
    error = tcp1.Listen();
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to listen IPv6 TCP socket: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "TCP client connecting to " << ipv6_addr.ToString(true) << "..." << std::endl;
    error = tcp2.Connect(ipv6_addr);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to connect to " << ipv6_addr.ToString(true) << " : " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Accepting TCP client..." << std::endl;
    error = tcp1.Accept(tcp3, ipv6_addr);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to accept: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Accepted client " << ipv6_addr.ToString(true) << "." << std::endl;

    memcpy(net_buff.Buffer, "Hello from TCP client.", 23);
    net_buff.BufferSize = 23;

    std::cout << "Sending " << buffer << "..." << std::endl;
    error = tcp2.Send(net_buff);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to send IPv6 TCP datas: " << error.ToString() << std::endl;
        return;
    }

    memset(net_buff.Buffer, 0, 1024);
    net_buff.BufferSize = 1024;

    std::cout << "Receiving datas from TCP client..." << std::endl;
    error = tcp3.Receive(net_buff);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to received IPv6 TCP datas: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Received datas from client " << ipv6_addr.ToString(true) << ": " << buffer << "." << std::endl;

    memcpy(net_buff.Buffer, "Hello from TCP server.", 23);
    net_buff.BufferSize = 23;

    std::cout << "Sending " << buffer << "..." << std::endl;
    error = tcp3.Send(net_buff);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to send IPv6 TCP datas: " << error.ToString() << std::endl;
        return;
    }

    memset(net_buff.Buffer, 0, 1024);
    net_buff.BufferSize = 1024;

    std::cout << "Receiving datas from TCP server..." << std::endl;
    error = tcp2.Receive(net_buff);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to received IPv6 TCP datas: " << error.ToString() << std::endl;
        return;
    }

    tcp1.GetSockName(ipv6_addr);
    std::cout << "Received datas from server " << ipv6_addr.ToString(true) << " : " << buffer << "." << std::endl;

    std::cout << __FUNCTION__ << " done !" << std::endl << std::endl;
}

#ifdef UNIX_TESTS
void TestUnixDgram()
{
    char buffer[1024] = { "Toto" };
    NetworkLibrary::Unix::UnixDgram unix1, unix2;
    NetworkLibrary::Unix::UnixAddr unix_addr;
    NetworkLibrary::Error error;
    NetworkLibrary::NetBuffer net_buff{ buffer, 0 };

    error = unix_addr.FromString("0.0.0.0");
    error = unix_addr.FromString("127.0.0.1:9999");

    error = unix1.CreateSocket();
    error = unix2.CreateSocket();

    error = unix1.Bind(unix_addr);

    // Send 4 bytes
    net_buff.BufferSize = 4;
    error = unix2.SendTo(unix_addr, net_buff);

    memset(net_buff.Buffer, 0, 1024);
    net_buff.BufferSize = 1024;
    error = unix1.ReceiveFrom(unix_addr, net_buff);

    std::cout << __FUNCTION__ << " done !" << std::endl << std::endl;
}

void TestUnixStream(const char* unix_sock_path)
{
    char buffer[1024];
    std::string addr_string;
    NetworkLibrary::Unix::UnixStream unix1, unix2, unix3;
    NetworkLibrary::Unix::UnixAddr unix_addr;
    NetworkLibrary::Error error;
    NetworkLibrary::NetBuffer net_buff{ buffer, 0 };

    std::cout << __FUNCTION__ << std::endl;

    std::cout << "Parsing Unix Stream " << unix_sock_path << "..." << std::endl;
    error = unix_addr.FromString(unix_sock_path);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to parse Unix(" << unix_sock_path << ") address: " << error.ToString() << std::endl;
        return;
    }
    addr_string = unix_addr.ToString(true);

    std::cout << "Creating Unix UNIX socket1..." << std::endl;
    error = unix1.CreateSocket();
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to create Unix UNIX socket1: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Creating Unix UNIX socket2..." << std::endl;
    error = unix2.CreateSocket();
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to create Unix UNIX socket2: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Binding Unix UNIX socket on " << unix_addr.ToString() << "..." << std::endl;
    error = unix1.Bind(unix_addr);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to bind Unix UNIX socket: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Listening Unix UNIX..." << std::endl;
    error = unix1.Listen();
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to listen Unix UNIX socket: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "UNIX client connecting to " << unix_addr.ToString(true) << "..." << std::endl;
    error = unix2.Connect(unix_addr);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to connect to " << unix_addr.ToString(true) << " : " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Accepting UNIX client..." << std::endl;
    error = unix1.Accept(unix3, unix_addr);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to accept: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Accepted client " << unix_addr.ToString(true) << "." << std::endl;

    memcpy(net_buff.Buffer, "Hello from UNIX client.", 24);
    net_buff.BufferSize = 24;

    std::cout << "Sending " << buffer << "..." << std::endl;
    error = unix2.Send(net_buff);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to send Unix UNIX datas: " << error.ToString() << std::endl;
        return;
    }

    memset(net_buff.Buffer, 0, 1024);
    net_buff.BufferSize = 1024;

    std::cout << "Receiving datas from UNIX client..." << std::endl;
    error = unix3.Receive(net_buff);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to received Unix UNIX datas: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Received datas from client " << unix_addr.ToString(true) << ": " << buffer << "." << std::endl;

    memcpy(net_buff.Buffer, "Hello from UNIX server.", 24);
    net_buff.BufferSize = 24;

    std::cout << "Sending " << buffer << "..." << std::endl;
    error = unix3.Send(net_buff);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to send Unix UNIX datas: " << error.ToString() << std::endl;
        return;
    }

    memset(net_buff.Buffer, 0, 1024);
    net_buff.BufferSize = 1024;

    std::cout << "Receiving datas from UNIX server..." << std::endl;
    error = unix2.Receive(net_buff);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to received Unix UNIX datas: " << error.ToString() << std::endl;
        return;
    }

    unix1.GetSockName(unix_addr);
    std::cout << "Received datas from server " << unix_addr.ToString(true) << " : " << buffer << "." << std::endl;

    std::cout << __FUNCTION__ << " done !" << std::endl << std::endl;
}
#endif
#ifdef BLUETOOTH_TESTS
void TestBluetooth()
{
    auto ifaces = NetworkLibrary::Bluetooth::GetIfacesAddresses();
    if ((int)ifaces.first != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to get Bluetooth adapters: " << ifaces.first.ToString() << std::endl;
        return;
    }
    for (auto& iface : ifaces.second)
    {
        std::cout << iface.Name << ' ' << iface.FriendlyName << ' ' << iface.IsUp << std::endl;
        for (auto& ip : iface.Addresses)
        {
            std::cout << "    " << ip.Addr << std::endl;
        }
        std::cout << std::endl;
    }

    std::cout << __FUNCTION__ << " done !" << std::endl << std::endl;
}

void TestBluetoothRFCOMMClient(const char* server_address, const char* bluetooth_service_uuid)
{
    std::string error_str;
    std::string addr_string;
    char buffer[1024];
    NetworkLibrary::Bluetooth::RFCOMM rfcomm1;
    NetworkLibrary::Bluetooth::BluetoothAddr bth_addr;
    NetworkLibrary::Bluetooth::UUID uuid;
    NetworkLibrary::Error error;
    NetworkLibrary::NetBuffer net_buff{ buffer, 0 };
    uint8_t channel;

    std::cout << __FUNCTION__ << std::endl;

    std::cout << "Parsing Bluetooth service uuid..." << std::endl;
    error = uuid.FromString(bluetooth_service_uuid);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to parse UUID: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Parsing Bluetooth server address..." << std::endl;
    error = bth_addr.FromString(server_address);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to parse Bluetooth address: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Querying SDP for RFCOMM channel from UUID..." << std::endl;
    error = NetworkLibrary::Bluetooth::GetChannelFromServiceUUID(uuid, bth_addr, channel);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to get channel at " << bth_addr.ToString() << " from uuid " << uuid.ToString() << ": " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Creating Bluetooth client socket..." << std::endl;
    error = rfcomm1.CreateSocket();
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to create Bluetooth socket: " << error.ToString() << std::endl;
        return;
    }

    bth_addr.SetChannel(channel);

    std::cout << "Connecting to Bluetooth: " << bth_addr.ToString() << std::endl;

    error = rfcomm1.Connect(bth_addr);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to connected to " << bth_addr.ToString(true) << ": " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Sending datas to server..." << std::endl;

    memcpy(buffer, "Hi from test client.", 21);
    net_buff.BufferSize = 21;
    error = rfcomm1.Send(net_buff);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to send datas : " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Datas sent, waiting for reply..." << std::endl;

    net_buff.BufferSize = 1024;
    error = rfcomm1.Receive(net_buff);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to receive datas : " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Datas received: " << buffer << std::endl;

    std::cout << __FUNCTION__ << " done !" << std::endl << std::endl;
}

void TestBluetoothRFCOMMServer(const char* bluetooth_service_uuid)
{
    std::string error_str;
    char buffer[1024];
    NetworkLibrary::Bluetooth::RFCOMM rfcomm1, rfcomm2;
    NetworkLibrary::Bluetooth::BluetoothAddr bth_addr;
    NetworkLibrary::Bluetooth::SDPService sdp_service;
    NetworkLibrary::Bluetooth::UUID service_uuid;
    NetworkLibrary::Error error;
    NetworkLibrary::NetBuffer net_buff{ buffer, 0 };

    bth_addr.SetAnyAddr();
    bth_addr.SetChannel(6);

    std::cout << __FUNCTION__ << std::endl;

    std::cout << "Parsing Bluetooth service uuid..." << std::endl;
    error = service_uuid.FromString(bluetooth_service_uuid);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to parse UUID: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Creating Bluetooth server socket..." << std::endl;
    error = rfcomm1.CreateSocket();
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to create Bluetooth socket: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Binding Bluetooth server socket on: " << bth_addr.ToString(true) << std::endl;
    error = rfcomm1.Bind(bth_addr);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to bind on " << bth_addr.ToString(true) << ": " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Listening on Bluetooth server socket..." << std::endl;
    error = rfcomm1.Listen();
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to listen: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Registering Bluetooth SDP Record..." << std::endl;
    error = sdp_service.RegisterSDPServiceRFCOMM(service_uuid, 6, "Bluetooth Test Record", "NetworkLibrary", "Portable NetworkLibrary");
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to listen: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Accepting Bluetooth client..." << std::endl;
    error = rfcomm1.Accept(rfcomm2, bth_addr);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to accept: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Accepted Bluetooth client: " << bth_addr.ToString() << std::endl;
    std::cout << "Receiving client datas..." << std::endl;

    net_buff.BufferSize = 1024;
    error = rfcomm2.Receive(net_buff);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to receive datas: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Received datas: " << buffer << std::endl;
    std::cout << "Sending datas to client..." << std::endl;

    memcpy(buffer, "Hi from test server.", 21);
    net_buff.BufferSize = 21;
    error = rfcomm2.Send(net_buff);
    if ((int)error != NetworkLibrary::Error::NoError)
    {
        std::cout << "Failed to send datas: " << error.ToString() << std::endl;
        return;
    }

    std::cout << "Datas sent." << std::endl;

    std::cout << __FUNCTION__ << " done !" << std::endl << std::endl;
}
#endif

int main()
{
    const char bth_uuid[] = "00000000-0000-0000-0000-000000000000";
    const char bth_server[] = "00:00:00:00:00:00";

    TestIPv4();
    TestIPv4UDP();
    TestIPv4TCP();

    TestIPv6();
    TestIPv6UDP();
    TestIPv6TCP();

#ifdef UNIX_TESTS
    TestUnixStream("unix1.sock");
#endif

#ifdef BLUETOOTH_TESTS
    TestBluetooth();
    //TestBluetoothRFCOMMServer(bth_uuid);
    //TestBluetoothRFCOMMClient(bth_server, bth_uuid);
#endif

    return 0;
}
