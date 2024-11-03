cmake_policy(SET CMP0091 NEW)
cmake_minimum_required(VERSION 3.15)
project(Socket)

if(WIN32) # Setup some variables for Windows build

elseif(APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

elseif(UNIX)

else()
  message(FATAL_ERROR "No CMake for other platforms")

endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_SHARED_LIBS "Build static or shared library" OFF)
option(SOCKET_DYNAMIC_RUNTIME "Link against dynamic runtime (Windows)" ON)
option(SOCKET_UNIX_SUPPORT "Support Unix socket" OFF)
option(SOCKET_BUILD_TESTS "Build tests app" OFF)

if(APPLE)
  set(SOCKET_BLUETOOTH_SUPPORT OFF)
  set(SOCKET_BLUETOOTH_BLUEZ_DEPRECATED OFF)
elseif(UNIX)
  option(SOCKET_BUILD_32BITS "Build 32bits library." OFF)
  option(SOCKET_BLUETOOTH_BLUEZ_DEPRECATED "Use BlueZ deprecated functions." OFF)
  option(SOCKET_BLUETOOTH_SUPPORT "Support Bluetooth socket" OFF)
else()
  option(SOCKET_BLUETOOTH_SUPPORT "Support Bluetooth socket" OFF)
  set(SOCKET_BLUETOOTH_BLUEZ_DEPRECATED OFF)
endif()

set(Socket_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/include/NetworkLibrary/details/Socket.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/NetworkLibrary/Poll.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/NetworkLibrary/IPv4.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/NetworkLibrary/IPv6.h
)

if(${SOCKET_UNIX_SUPPORT})
  set(Socket_headers
    ${Socket_headers}
    ${CMAKE_CURRENT_SOURCE_DIR}/include/NetworkLibrary/Unix.h
  )
endif()

if(${SOCKET_BLUETOOTH_SUPPORT})
  set(Socket_headers
    ${Socket_headers}
    ${CMAKE_CURRENT_SOURCE_DIR}/include/NetworkLibrary/Bluetooth.h
  )
endif()

set(All_Headers
  ${Socket_headers}
  ${CMAKE_CURRENT_SOURCE_DIR}/src/internals/internal_bluetooth.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/internals/internal_socket.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/internals/internal_os_stuff.h
)

########################################
## Build library
add_library(networklibrary
  src/internals/internal_socket.cpp
  src/Poll.cpp
  src/Socket.cpp
  src/IPv4.cpp
  src/IPv6.cpp
  $<$<BOOL:${SOCKET_UNIX_SUPPORT}>:src/Unix.cpp>
  $<$<BOOL:${SOCKET_BLUETOOTH_SUPPORT}>:src/internals/internal_bluetooth.cpp>
  $<$<BOOL:${SOCKET_BLUETOOTH_SUPPORT}>:src/Bluetooth.cpp>
  
  ${All_Headers}
)

set_target_properties(networklibrary PROPERTIES
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<BOOL:SOCKET_DYNAMIC_RUNTIME>:DLL>"
  POSITION_INDEPENDENT_CODE ON
  C_VISIBILITY_PRESET hidden
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN ON
)

target_link_libraries(networklibrary
  PUBLIC
  # Winsocks
  $<$<BOOL:${WIN32}>:ws2_32>
  $<$<BOOL:${WIN32}>:iphlpapi>
  # Windows Bluetooth
  $<$<AND:$<BOOL:${WIN32}>,$<BOOL:${SOCKET_BLUETOOTH_SUPPORT}>>:bthprops>
 
  # Linux Bluetooth
  $<$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>,$<BOOL:${SOCKET_BLUETOOTH_SUPPORT}>>:bluetooth>
  $<$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>,$<BOOL:${SOCKET_BLUETOOTH_SUPPORT}>,$<NOT:$<BOOL:${SOCKET_BLUETOOTH_BLUEZ_DEPRECATED}>>>:dbus-1>
)

target_compile_options(networklibrary
  PRIVATE
  $<$<BOOL:${MSVC}>:/MP>
)

target_include_directories(networklibrary
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  
  PRIVATE
  $<$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>,$<BOOL:${SOCKET_BLUETOOTH_SUPPORT}>>:/usr/include/dbus-1.0>
  $<$<AND:$<BOOL:${SOCKET_BUILD_32BITS}>,$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>,$<BOOL:${SOCKET_BLUETOOTH_SUPPORT}>>:/usr/lib/i386-linux-gnu/dbus-1.0/include>
  $<$<AND:$<NOT:$<BOOL:${SOCKET_BUILD_32BITS}>>,$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>,$<BOOL:${SOCKET_BLUETOOTH_SUPPORT}>>:/usr/lib/x86_64-linux-gnu/dbus-1.0/include>
)

target_compile_definitions(networklibrary
  PRIVATE
  EXPORT_NETWORKLIBRARY_SYMBOLS
  $<$<BOOL:${SOCKET_BLUETOOTH_BLUEZ_DEPRECATED}>:USE_BLUEZ_COMPAT>
)

if(${SOCKET_BUILD_TESTS})
  add_executable(library_test
    tests/main.cpp
  )

  target_link_libraries(library_test
    PRIVATE
    networklibrary
  )

  target_compile_definitions(library_test
    PRIVATE
    $<$<BOOL:${SOCKET_UNIX_SUPPORT}>:UNIX_TESTS>
    $<$<BOOL:${SOCKET_BLUETOOTH_SUPPORT}>:BLUETOOTH_TESTS>
  )
  
  set_target_properties(library_test PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<BOOL:SOCKET_DYNAMIC_RUNTIME>:DLL>"
    POSITION_INDEPENDENT_CODE ON
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
  )
endif()

##################
## Install rules

add_library(Nemirtingas::NetworkLibrary ALIAS networklibrary)
set_target_properties(networklibrary PROPERTIES EXPORT_NAME NetworkLibrary)

##################
## Install rules
install(TARGETS networklibrary EXPORT NetworkLibraryTargets
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/NetworkLibrary/details/Socket.h DESTINATION include/NetworkLibrary/details)
install(FILES ${Socket_headers} DESTINATION include/NetworkLibrary)

# Export targets
install(
  EXPORT NetworkLibraryTargets
  FILE NetworkLibraryConfig.cmake
  NAMESPACE Nemirtingas::
  DESTINATION lib/cmake/NetworkLibrary
)

