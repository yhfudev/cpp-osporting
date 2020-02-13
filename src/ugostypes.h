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
#include <sys/types.h> // off_t
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


#endif // UG_OS_TYPES_H
