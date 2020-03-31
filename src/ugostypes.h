/**
 * \file    ugostypes.h
 * \brief   the portable types
 * \author  Yunhui Fu (yhfudev@gmail.com)
 * \version 1.0
 * \date    2015-07-15
 */
#ifndef UG_OS_TYPES_H
#define UG_OS_TYPES_H 1

////////////////////////////////////////////////////////////////////////////////
// type
#if defined(ARDUINO)
#include <stddef.h>
#include <stdint.h>    // uint8_t
#include <stdlib.h>    // size_t
#include <sys/types.h> // ssize_t pid_t off_t
typedef unsigned char uint8_t;
#if ! defined(ARDUINO_ARCH_ESP8266) && ! defined(ARDUINO_ARCH_ESP32)
typedef long int ssize_t;
#endif
//typedef unsigned long int size_t;
//#define off_t size_t
//typedef size_t off_t;
#endif // ARDUINO

#ifndef SEEK_SET
#define SEEK_SET 0x01
#define SEEK_CUR 0x02
#define SEEK_END 0x04
#endif


#if defined(ARDUINO)
#include <stdint.h> // uint32_t
// there's overflow of the wchar_t due to the 2-byte size in Arduino
// sizeof(wchar_t)=2; sizeof(size_t)=2; sizeof(uint32_t)=4;
// sizeof(int)=2; sizeof(long)=4; sizeof(unsigned)=2;
//#define wchar_t uint32_t
#else
#include <stdint.h> // uint32_t
#include <wchar.h>
// x86_64
// sizeof(wchar_t)=4; sizeof(size_t)=8; sizeof(uint32_t)=4;
// sizeof(int)=4; sizeof(long)=8; sizeof(unsigned)=4;
#endif


#endif // UG_OS_TYPES_H
