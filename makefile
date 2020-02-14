.PHONY: clean, mrproper
 
FLAGS = -W -Wall -Werror -std=c++14
 
EXEC = a.out
CXX = g++
ARCH = -m64
CXXFLAGS = $(FLAGS) -O3 $(ARCH)
#CXXFLAGS = $(FLAGS) -fPIC -O3 -D__EXPORT_SYMBOLS__
CXXCOMPFLAGS = $(ARCH)
#CXXCOMPFLAGS = -shared -fPIC -Wl,-soname,$(EXEC).1
CXXINCLUDES = -I. -I /usr/include/dbus-1.0 -I /usr/lib/x86_64-linux-gnu/dbus-1.0/include
LIBRARIES = -ldbus-1 -lbluetooth
 
SOCKET_COMMON_HEADERS    = Socket/common/basic_socket Socket/common/socket.h
SOCKET_IPV4_HEADERS      = Socket/ipv4/ipv4_addr Socket/ipv4/tcp_socket Socket/ipv4/udp_socket
SOCKET_BLUETOOTH_HEADERS = Socket/bluetooth/bluetooth_socket.h Socket/bluetooth/rfcomm_addr Socket/bluetooth/rfcomm_socket
SOCKET_UNIX_HEADERS      = Socket/unix/unix_socket Socket/unix/unix_addr

#UTILS_DBUS_HEADERS = utils/dbus.h
 
HEADERS  = utils/utils.h
HEADERS += $(SOCKET_COMMON_HEADERS)
HEADERS += $(SOCKET_IPV4_HEADERS)
HEADERS += $(SOCKET_BLUETOOTH_HEADERS)
HEADERS += $(SOCKET_UNIX_HEADERS)

#HEADERS += $(UTILS_DBUS_HEADERS)

SOCKET_COMMON_SOURCES    = Socket/common/socket.cpp
SOCKET_BLUETOOTH_SOURCES = Socket/bluetooth/bluetooth_socket.cpp

#UTILS_DBUS_SOURCES = utils/dbus.cpp

SOURCES  = utils/utils.cpp main.cpp
SOURCES += $(SOCKET_COMMON_SOURCES)
SOURCES += $(SOCKET_IPV4_SOURCES)
SOURCES += $(SOCKET_BLUETOOTH_SOURCES)
SOURCES += $(SOCKET_UNIX_SOURCES)
#SOURCES += $(UTILS_DBUS_SOURCES)
 
OBJS = $(SOURCES:.cpp=.o)
 
all: $(OBJS)
	$(CXX) $(CXXCOMPFLAGS) $^ $(LIBRARIES) -o $(EXEC) && strip $(EXEC)
 
main.o: $(HEADERS)
 
%.o: %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS) $(CXXINCLUDES)
 
clean: 
	find . -name '*.o' -exec rm -f {} \;
 
mrproper: clean
	rm -rf $(EXEC)
 
debug: CXXFLAGS = $(FLAGS) -ggdb
debug: all

uuid_generator: uuid4gen.o
	$(CXX) $(CXXCOMPFLAGS) $^ -o uuid_generator && strip uuid_generator
