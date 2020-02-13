/**
 * \file    ugstring.h
 * \brief   string related functions
 * \author  Yunhui Fu (yhfudev@gmail.com)
 * \version 1.0
 * \date    2020-02-10
 */
#ifndef UG_STRING_H
#define UG_STRING_H 1

#include "ugosbase.h"

////////////////////////////////////////////////////////////////////////////////
// string and memory

#ifndef __ATTR_PROGMEM__
  #define strlen_P strlen
  #define strcpy_P strcpy
  #define strncmp_P strncmp
  #define memcpy_P memcpy
  #define vsnprintf_P vsnprintf
  #define sprintf_P sprintf
  #define snprintf_P snprintf
#endif // __ATTR_PROGMEM__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


// read a byte from ROM or RAM
typedef char (* read_byte_cb_t)(const char * str);
#ifndef __ATTR_PROGMEM__
  #define program_to_ram(a) (a)
  #define CSTR_P2R(a) (a)

  #define my_strlen_P(p)        strlen((const char *)(p))
  #define my_strncmp_bothstatic strncmp
  #define my_strncmp_rightstatic strncmp
  #define my_strncmp_P(p1,p2,sz)      strncmp((p1), (p2), (sz))
  #define my_memmove_P(dest, src, sz) memmove((dest), (src), (sz))
  #define my_strcpy_P(a,b)      strcpy((a),(b))

#else
  // convert program address of a string to RAM address space for testing (by a static buffer ring).
  char * program_to_ram(const char * cstr_in);
  #define CSTR_P2R(a) program_to_ram(a) ///< map the string from ROM to RAM

  char read_byte_ram(const char *str);
  char read_byte_rom(const char *str);
  int my_strncmp_via_callback(const char *p1, const char *p2, size_t max_sz, read_byte_cb_t cb_read_byte_left, read_byte_cb_t cb_read_byte_right);

  //#define my_strncmp_leftstatic(p1, p2, max_sz)  my_strncmp_via_callback((p1), (const char *)(p2), (max_sz), read_byte_rom, read_byte_ram)
  #define my_strncmp_rightstatic(p1, p2, max_sz) my_strncmp_via_callback((const char *)(p1), (p2), (max_sz), read_byte_ram, read_byte_rom)
  //#define my_strncmp_rightstatic(p1, p2, max_sz) strncasecmp_P((p1), (p2), (max_sz))
  //#define my_strncmp_bothstatic(p1, p2, max_sz)  my_strncmp_via_callback((p1),               (p2), (max_sz), read_byte_rom, read_byte_rom)

  int my_strlen_P(const char *pstart);
  void * my_memmove_P(void *dest, const void *src, size_t sz);
  #define my_strcpy_P(a,b)      my_memmove_P((a),(b),my_strlen_P(b)+1)
#endif
#define my_strncmp_leftstatic(a,b,c) (0 - my_strncmp_rightstatic((b),(a),(c)))

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // UG_OS_TIME_H
