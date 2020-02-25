/**
 * \file    osporting.h
 * \brief   remove the gap between OSs/Platforms
 * \author  Yunhui Fu <yhfudev@gmail.com>
 * \version 1.0
 */
#ifndef _OS_PORTING_H
#define _OS_PORTING_H 1

////////////////////////////////////////////////////////////////////////////////
#include "ugosbase.h"
#include "ugosarduino.h"
#include "ugostypes.h"
#include "ugostime.h"
#include "ugdebug.h"


////////////////////////////////////////////////////////////////////////////////
// Macro
#define UNUSED_VARIABLE(a) ((void)(a))

#ifndef NUM_ARRAY
  #define NUM_ARRAY(a) (sizeof(a)/sizeof((a)[0]))
#endif // NUM_ARRAY

#define UNUSED_VARIABLE(a) ((void)(a))

#define MIN(a,b) (((a)>(b))?(b):(a))

#if defined(ARDUINO) && ! defined(ARDUINO_ARCH_ESP8266) && ! defined(ARDUINO_ARCH_ESP32)
#define constrain(x,a,b) (((x)<(a))?(a):((x)>(b)?(b):(x)))
#define round(x)     ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
#define dtostrf(val, width, prec, buf) sprintf (buf, "%" # width "." # prec "f", (val))
#endif

////////////////////////////////////////////////////////////////////////////////
// AVR related

#include <stdarg.h>
#if defined(__AVR__)
#include <avr/io.h>
#include <avr/interrupt.h>
#endif

#if defined(__AVR__)
  #include <avr/pgmspace.h>
#elif defined(_VARIANT_ARDUINO_STM32_)
#include <avr/pgmspace.h>                  //use for PROGMEM Arduino STM32
#elif defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
  #include <pgmspace.h>
#else
  #include <stdint.h>
  #include <string.h>
  #include <assert.h>
  #define pgm_read_dword_near(addr) *((const uint32_t *)(addr))
  #define pgm_read_dword pgm_read_word_near
  #define pgm_read_word_near(addr) *((const uint16_t *)(addr))
  #define pgm_read_word pgm_read_word_near
  #define pgm_read_byte_near(addr) *((const uint8_t *)(addr))
  #define pgm_read_byte pgm_read_byte_near

  #define memcpy_P memcpy
  #define strcat_P strcat
#endif

#ifndef PSTR
  #define PSTR(a) (a)
#endif // PSTR
#ifndef PROGMEM
  #define PROGMEM
#endif // PROGMEM

#ifndef __ATTR_PROGMEM__
  #define strlen_P strlen
  #define strcpy_P strcpy
  #define strncmp_P strncmp
  #define memcpy_P memcpy
  #define vsnprintf_P vsnprintf
  #define sprintf_P sprintf
  #define snprintf_P snprintf
#endif // __ATTR_PROGMEM__

#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif
#ifndef __LPM
#define __LPM(addr) (*(addr))
#endif
#ifndef __PMT
#define __PMT(args)	args
#endif

#define bitRead(a,i) (((a) >> (i)) & 0x01)

#if defined(__AVR__)
#define ugf_int_t int // char
#else
#define ugf_int_t int
#endif


////////////////////////////////////////////////////////////////////////////////
#endif /* _OS_PORTING_H */
