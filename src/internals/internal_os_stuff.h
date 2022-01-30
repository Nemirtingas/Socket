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

#pragma once

#if defined(WIN64) || defined(_WIN64) || defined(__MINGW64__) ||\
    defined(WIN32) || defined(_WIN32) || defined(__MINGW32__)
    #define SOCKET_OS_WINDOWS
#elif defined(__linux__) || defined(linux)
    #define SOCKET_OS_LINUX
#elif defined(__APPLE__)
    #define SOCKET_OS_APPLE
#else
    //#error "Unknown OS"
#endif

#ifdef __cplusplus
    #define SOCKET_EXTERN_NONE
    #define SOCKET_EXTERN_C   extern "C"
    #define SOCKET_EXTERN_CXX extern
#else
    #define SOCKET_EXTERN_NONE
    #define SOCKET_EXTERN_C   extern
    #define SOCKET_EXTERN_CXX #error "No C++ export in C"
#endif

#if defined(SOCKET_OS_WINDOWS)
    #if defined(__clang__)
        #define SOCKET_CALL_DEFAULT 
        #define SOCKET_CALL_STDL    __stdcall
        #define SOCKET_CALL_CDECL   __cdecl
        #define SOCKET_CALL_FAST    __fastcall
        #define SOCKET_CALL_THIS    __thiscall

        #define SOCKET_MODE_DEFAULT
        #define SOCKET_MODE_EXPORT  __declspec(dllexport)
        #define SOCKET_MODE_IMPORT  __declspec(dllimport)
        #define SOCKET_MODE_HIDDEN

        #define SOCKET_HIDE_CLASS(keyword)                                         SOCKET_EXTERN_NONE SOCKET_MODE_HIDDEN keyword
        #define SOCKET_HIDE_API(return_type, call_convention)                      SOCKET_EXTERN_NONE SOCKET_MODE_HIDDEN return_type call_convention
        #define SOCKET_EXPORT_API(extern_type, return_type, mode, call_convention) extern_type        mode               return_type call_convention
    #else
        #define SOCKET_CALL_DEFAULT 
        #define SOCKET_CALL_STDL    __stdcall
        #define SOCKET_CALL_CDECL   __cdecl
        #define SOCKET_CALL_FAST    __fastcall
        #define SOCKET_CALL_THIS    __thiscall

        #define SOCKET_MODE_DEFAULT
        #define SOCKET_MODE_EXPORT  __declspec(dllexport)
        #define SOCKET_MODE_IMPORT  __declspec(dllimport)
        #define SOCKET_MODE_HIDDEN 

        #define SOCKET_HIDE_CLASS(keyword)                                         SOCKET_EXTERN_NONE SOCKET_MODE_HIDDEN keyword
        #define SOCKET_HIDE_API(return_type, call_convention)                      SOCKET_EXTERN_NONE SOCKET_MODE_HIDDEN return_type call_convention
        #define SOCKET_EXPORT_API(extern_type, return_type, mode, call_convention) extern_type        mode               return_type call_convention
    #endif
#elif defined(SOCKET_OS_LINUX) || defined(SOCKET_OS_APPLE)
    #define SOCKET_CALL_DEFAULT 
    #define SOCKET_CALL_STD     __attribute__((stdcall))
    #define SOCKET_CALL_CDECL   __attribute__((cdecl))
    #define SOCKET_CALL_FAST    __attribute__((fastcall))
    #define SOCKET_CALL_THIS    __attribute__((thiscall))

    #define SOCKET_MODE_DEFAULT
    #define SOCKET_MODE_EXPORT  __attribute__((visibility("default")))
    #define SOCKET_MODE_IMPORT  __attribute__((visibility("default")))
    #define SOCKET_MODE_HIDDEN  __attribute__((visibility("hidden")))

    #define SOCKET_HIDE_CLASS(keyword)                                         SOCKET_EXTERN_NONE keyword     SOCKET_MODE_HIDDEN
    #define SOCKET_HIDE_API(return_type, call_convention)                      SOCKET_EXTERN_NONE SOCKET_MODE_HIDDEN return_type call_convention
    #define SOCKET_EXPORT_API(extern_type, return_type, mode, call_convention) extern_type        mode               return_type call_convention

    //#define LOCAL_API __attribute__((visibility ("internal")))
#endif