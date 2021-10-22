#CMAKE_TOOLCHAIN_FILE

project(Socket)
cmake_minimum_required(VERSION 3.0)

if(NOT X64 AND NOT X86)
  option(BUILD_32BITS "Build 32bits (64bits is default)" OFF)
  if(${BUILD_32BIT})
    set(X64 OFF)
    set(X86 ON)
  else()
    set(X64 ON)
    set(X86 OFF)
  endif()
endif()

if(WIN32) # Setup some variables for Windows build
  # Detect arch on Windows
  if(NOT X64 AND NOT X86 )
    if( ${CMAKE_SIZEOF_VOID_P} EQUAL 8)
      set(X64 ON)
    else()
      set(X86 ON)
    endif()
  endif()

  if(MSVC) # If building with MSVC
    set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        )
    foreach(CompilerFlag ${CompilerFlags})
      string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()

  endif()

elseif(APPLE)
  if(X64)
    set(CMAKE_EXE_LINKER_FLAGS -m64)
    set(CMAKE_SHARED_LINKER_FLAGS -m64)
  elseif(X86)
    set(CMAKE_EXE_LINKER_FLAGS -m32)
    set(CMAKE_SHARED_LINKER_FLAGS -m32)
  else()
    message(FATAL_ERROR "Arch unknown")
  endif()
  #add_compile_options("-stdlib=libc++")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

elseif(UNIX)
  if(X64)
    set(CMAKE_EXE_LINKER_FLAGS -m64)
    set(CMAKE_SHARED_LINKER_FLAGS -m64)
  elseif(X86)
    set(CMAKE_EXE_LINKER_FLAGS -m32)
    set(CMAKE_SHARED_LINKER_FLAGS -m32)
  else()
    message(FATAL_ERROR "Arch unknown")
  endif()

else()
  message(FATAL_ERROR "No CMake for other platforms")

endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(UNIX_SUPPORT "Support Unix socket" OFF)
option(BUILD_SHARED_LIBS "Build static or shared library" OFF)
option(BUILD_TESTS "Build tests app" OFF)

if(APPLE)
  set(BLUETOOTH_SUPPORT OFF)
elseif(UNIX)
  option(BLUETOOTH_BLUEZ_DEPRECATED "Use BlueZ deprecated functions." OFF)
  option(BLUETOOTH_SUPPORT "Support Bluetooth socket" OFF)
else()
  option(BLUETOOTH_SUPPORT "Support Bluetooth socket" OFF)
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
    NetworkLibrary
    src/internals/internal_socket.cpp
    src/Poll.cpp
    src/Socket.cpp
    src/IPv4.cpp
    src/IPv6.cpp
    $<$<BOOL:${UNIX_SUPPORT}>:src/Unix.cpp>
    $<$<BOOL:${BLUETOOTH_SUPPORT}>:src/internals/internal_bluetooth.cpp>
    $<$<BOOL:${BLUETOOTH_SUPPORT}>:src/Bluetooth.cpp>
)

target_link_libraries(
  NetworkLibrary
  PUBLIC
  # Winsocks
  $<$<BOOL:${WIN32}>:ws2_32>
  $<$<BOOL:${WIN32}>:iphlpapi>
  # Windows Bluetooth
  $<$<AND:$<BOOL:${WIN32}>,$<BOOL:${BLUETOOTH_SUPPORT}>>:bthprops>
 
  # Linux Bluetooth
  $<$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>,$<BOOL:${BLUETOOTH_SUPPORT}>>:bluetooth>
  $<$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>,$<BOOL:${BLUETOOTH_SUPPORT}>>:dbus-1>
)

target_include_directories(
  NetworkLibrary
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/utils/include
  $<$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>,$<BOOL:${BLUETOOTH_SUPPORT}>>:/usr/include/dbus-1.0>
  $<$<AND:$<BOOL:X64>,$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>,$<BOOL:${BLUETOOTH_SUPPORT}>>:/usr/lib/x86_64-linux-gnu/dbus-1.0/include>
  $<$<AND:$<BOOL:X86>,$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>,$<BOOL:${BLUETOOTH_SUPPORT}>>:/usr/lib/i386-linux-gnu/dbus-1.0/include>
)

target_compile_definitions(
  NetworkLibrary
  PRIVATE
  EXPORT_NETWORKLIBRARY_SYMBOLS
  $<$<BOOL:${BLUETOOTH_BLUEZ_DEPRECATED}>:USE_BLUEZ_COMPAT>
)

target_compile_options(
  NetworkLibrary
  PRIVATE
  
  $<$<BOOL:${UNIX}>:-fPIC -fpermissive -fvisibility=hidden -Wl,--exclude-libs,ALL>
  $<$<AND:$<BOOL:${UNIX}>,$<BOOL:${X86}>>:-m32>
  $<$<AND:$<BOOL:${UNIX}>,$<BOOL:${X64}>>:-m64>
  
  $<$<AND:$<CONFIG:>,$<BOOL:${MSVC}>>:/MP>
  $<$<AND:$<CONFIG:Debug>,$<BOOL:${MSVC}>>:/MP>
  $<$<AND:$<CONFIG:Release>,$<BOOL:${MSVC}>>:/MP>
)

if(${BUILD_TESTS})
  add_executable(
    library_test
    tests/main.cpp
  )

  target_include_directories(
    library_test
    PRIVATE
	
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  )

  target_link_libraries(
    library_test
	PRIVATE
	
    NetworkLibrary
  
    # Winsocks
    $<$<BOOL:${WIN32}>:ws2_32>
    # Windows Bluetooth
    $<$<AND:$<BOOL:${WIN32}>,$<BOOL:${BLUETOOTH_SUPPORT}>>:bthprops>
   
    # Linux Bluetooth
    $<$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>,$<BOOL:${BLUETOOTH_SUPPORT}>>:bluetooth>
    $<$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>,$<BOOL:${BLUETOOTH_SUPPORT}>>:dbus-1>
  )

  target_compile_definitions(
    library_test
    PRIVATE
    $<$<BOOL:${UNIX_SUPPORT}>:UNIX_TESTS>
    $<$<BOOL:${BLUETOOTH_SUPPORT}>:BLUETOOTH_TESTS>
  )

  target_compile_options(
    library_test
    PRIVATE
  
    $<$<BOOL:${UNIX}>:-fpermissive -fvisibility=hidden -Wl,--exclude-libs,ALL>
    $<$<AND:$<BOOL:${UNIX}>,$<BOOL:${X86}>>:-m32>
    $<$<AND:$<BOOL:${UNIX}>,$<BOOL:${X64}>>:-m64>
  
    $<$<AND:$<CONFIG:>,$<BOOL:${MSVC}>>:/MP>
    $<$<AND:$<CONFIG:Debug>,$<BOOL:${MSVC}>>:/MP>
    $<$<AND:$<CONFIG:Release>,$<BOOL:${MSVC}>>:/MP>
  )
endif()

##################
## Install rules
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})

if(WIN32)
  if(X64)
    set(OUT_DIR win64)
  else()
    set(OUT_DIR win32)
  endif()
elseif(APPLE)
  if(X64)
    set(OUT_DIR macosx64)
  else()
    set(OUT_DIR macosx32)
  endif()
elseif(UNIX)
  if(X64)
    set(OUT_DIR linux64)
  else()
    set(OUT_DIR linux32)
  endif()
endif()

install(
  TARGETS NetworkLibrary
  ARCHIVE DESTINATION release/${OUT_DIR}
  LIBRARY DESTINATION release/${OUT_DIR}
  RUNTIME DESTINATION release/${OUT_DIR}
)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/NetworkLibrary/details/Socket.h DESTINATION release/${OUT_DIR}/include/NetworkLibrary/details)
install(FILES ${Socket_headers} DESTINATION release/${OUT_DIR}/include/NetworkLibrary)