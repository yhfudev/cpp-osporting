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
#if ! defined(ARDUINO)
#include <stdint.h> // uint8_t, uint32_t
#include <stdlib.h>    /* size_t */
#include <stdio.h>
#include <stdlib.h> // abs()
#include <ctype.h>
#include <unistd.h> // usleep()
#include <string.h>
#include <limits.h>
#include <sys/types.h> /* ssize_t pid_t off64_t */
#include <sys/time.h>
#include <time.h>
#include <assert.h>

#endif // ! ARDUINO

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
#elseif defined(ARDUINO)
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
