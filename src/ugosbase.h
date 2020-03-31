/**
 * \file    ugosbase.h
 * \brief   basic definitions
 * \author  Yunhui Fu (yhfudev@gmail.com)
 * \version 1.0
 * \date    2015-07-19
 * \copyright GPL/BSD
 */
#ifndef UG_OS_BASE_H
#define UG_OS_BASE_H 1

////////////////////////////////////////////////////////////////////////////////
// common
#include <stdint.h> // uint8_t, uint32_t
#include <stdlib.h> // abs() size_t
#if ! defined(ARDUINO)
#include <stdio.h>
#include <ctype.h>
#include <unistd.h> // STDERR_FILENO usleep()
#include <string.h>
#include <limits.h>
#include <sys/types.h> /* ssize_t pid_t off64_t */
#include <sys/time.h>
#include <time.h>
#include <assert.h>
#endif // ! ARDUINO

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#ifndef fsync
#define fsync(fd) _commit(fd)
#endif
#ifndef u_int8_t
#define u_int8_t uint8_t
#define u_int16_t uint16_t
#define u_int32_t uint32_t
#define u_int64_t uint64_t
#endif

#elif ! defined(ARDUINO)
#include <unistd.h> // STDERR_FILENO
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> // inet_ntoa()
#endif

////////////////////////////////////////////////////////////////////////////////
// Arduino related
#if defined(ARDUINO)

#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif
#include <stdarg.h>
#include <limits.h> //INT_MIN

#ifdef __cplusplus__
#include "HardwareSerial.h"
#include "USBAPI.h"
#include <Wire.h>
#endif

#endif // ARDUINO

////////////////////////////////////////////////////////////////////////////////
// C/C++ language

#if defined(ARDUINO)
#    define nullptr NULL
#    define intptr_t int
#else
#ifdef __GNUC__
#  include <features.h>
#  if __GNUC_PREREQ(4,6)
#    define nullptr NULL
//    #define intptr_t int
#  endif
#endif
#endif // ARDUINO

// bool
#ifdef __cplusplus
#define bool_t bool
#else
#define bool_t uint8_t
#define true  1
#define false 0
#define nullptr NULL
#define intptr_t int
#endif // __cplusplus

//#define NULL (0)

////////////////////////////////////////////////////////////////////////////////
// format
#include <inttypes.h> /* for PRIdPTR PRIiPTR PRIoPTR PRIuPTR PRIxPTR PRIXPTR, SCNdPTR SCNiPTR SCNoPTR SCNuPTR SCNxPTR */
#ifndef PRIuSZ
#ifdef __WIN32__                /* or whatever */
#define PRIiSZ "Id"
#define PRIuSZ "Iu"
#elif defined(ARDUINO)
#define PRIiSZ "d"
#define PRIuSZ "d"
#define PRIxSZ "X"
#define SCNxSZ "X"
#else
#define PRIiSZ "zd"
#define PRIuSZ "zu"
#define PRIxSZ "zx"
#define SCNxSZ "zx"
#endif
#define PRIiOFF PRIx64 /*"lld"*/
#define PRIuOFF PRIx64 /*"llu"*/
//#define PRIiOFF "lld"
//#define PRIuOFF "llu"
#endif // PRIuSZ

////////////////////////////////////////////////////////////////////////////////
#endif // UG_OS_BASE_H
