#CMAKE_TOOLCHAIN_FILE

cmake_policy(SET CMP0091 NEW)
project(Socket)
cmake_minimum_required(VERSION 3.0)

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

set(SYSTEM_CODE OFF CACHE BOOL "")
set(FILESYSTEM_CODE OFF CACHE BOOL "")
set(LIBRARY_CODE OFF CACHE BOOL "")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/System EXCLUDE_FROM_ALL)

option(UNIX_SUPPORT "Support Unix socket" OFF)
option(BUILD_SHARED_LIBS "Build static or shared library" OFF)
option(BUILD_TESTS "Build tests app" OFF)

if(APPLE)
  set(BLUETOOTH_SUPPORT OFF)
  set(BLUETOOTH_BLUEZ_DEPRECATED OFF)
elseif(UNIX)
  option(BUILD_32BITS "Build 32bits library." OFF)
  option(BLUETOOTH_BLUEZ_DEPRECATED "Use BlueZ deprecated functions." OFF)
  option(BLUETOOTH_SUPPORT "Support Bluetooth socket" OFF)
else()
  option(BLUETOOTH_SUPPORT "Support Bluetooth socket" OFF)
  set(BLUETOOTH_BLUEZ_DEPRECATED OFF)
endif()

set(Socket_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/include/NetworkLibrary/details/Socket.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/NetworkLibrary/Poll.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/NetworkLibrary/IPv4.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/NetworkLibrary/IPv6.h
)

if(${UNIX_SUPPORT})
  set(Socket_headers
    ${Socket_headers}
    ${CMAKE_CURRENT_SOURCE_DIR}/include/NetworkLibrary/Unix.h
  )
endif()

if(${BLUETOOTH_SUPPORT})
  set(Socket_headers
    ${Socket_headers}
    ${CMAKE_CURRENT_SOURCE_DIR}/include/NetworkLibrary/Bluetooth.h
  )
endif()

########################################
## Build library
add_library(
  networklibrary
  src/internals/internal_socket.cpp
  src/Poll.cpp
  src/Socket.cpp
  src/IPv4.cpp
  src/IPv6.cpp
  $<$<BOOL:${UNIX_SUPPORT}>:src/Unix.cpp>
  $<$<BOOL:${BLUETOOTH_SUPPORT}>:src/internals/internal_bluetooth.cpp>
  $<$<BOOL:${BLUETOOTH_SUPPORT}>:src/Bluetooth.cpp>
)

set_property(TARGET networklibrary PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set_property(TARGET networklibrary PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(
  networklibrary
  PRIVATE
  System::System

  PUBLIC
  # Winsocks
  $<$<BOOL:${WIN32}>:ws2_32>
  $<$<BOOL:${WIN32}>:iphlpapi>
  # Windows Bluetooth
  $<$<AND:$<BOOL:${WIN32}>,$<BOOL:${BLUETOOTH_SUPPORT}>>:bthprops>
 
  # Linux Bluetooth
  $<$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>,$<BOOL:${BLUETOOTH_SUPPORT}>>:bluetooth>
  $<$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>,$<BOOL:${BLUETOOTH_SUPPORT}>,$<NOT:$<BOOL:${BLUETOOTH_BLUEZ_DEPRECATED}>>>:dbus-1>
)

target_compile_options(
  networklibrary
  PRIVATE
  $<$<BOOL:${MSVC}>:/MP>
)

target_include_directories(
  networklibrary
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  
  PRIVATE
  $<$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>,$<BOOL:${BLUETOOTH_SUPPORT}>>:/usr/include/dbus-1.0>
  $<$<AND:$<BOOL:${BUILD_32BITS}>,$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>,$<BOOL:${BLUETOOTH_SUPPORT}>>:/usr/lib/i386-linux-gnu/dbus-1.0/include>
  $<$<AND:$<NOT:$<BOOL:${BUILD_32BITS}>>,$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>,$<BOOL:${BLUETOOTH_SUPPORT}>>:/usr/lib/x86_64-linux-gnu/dbus-1.0/include>
)

target_compile_definitions(
  networklibrary
  PRIVATE
  EXPORT_NETWORKLIBRARY_SYMBOLS
  $<$<BOOL:${BLUETOOTH_BLUEZ_DEPRECATED}>:USE_BLUEZ_COMPAT>
)

if(${BUILD_TESTS})
  add_executable(
    library_test
    tests/main.cpp
  )

  target_link_libraries(
    library_test
    PRIVATE
    networklibrary
  )

  target_compile_definitions(
    library_test
    PRIVATE
    $<$<BOOL:${UNIX_SUPPORT}>:UNIX_TESTS>
    $<$<BOOL:${BLUETOOTH_SUPPORT}>:BLUETOOTH_TESTS>
  )
endif()

##################
## Install rules

add_library(NetworkLibrary::NetworkLibrary ALIAS networklibrary)
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
  NAMESPACE NetworkLibrary::
  DESTINATION lib/cmake/NetworkLibrary
)

