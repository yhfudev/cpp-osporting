/**
 * @file    getutf8.h
 * @brief   get utf-8 string value
 * @author  Yunhui Fu (yhfudev@gmail.com)
 * @version 1.0
 * @date    2015-06-19
 * @copyright GPL/BSD
 */
#ifndef GET_UTF8_H
#define GET_UTF8_H

#if defined(ARDUINO)
#include <stdint.h> // uint32_t
// there's overflow of the wchar_t due to the 2-byte size in Arduino
// sizeof(wchar_t)=2; sizeof(size_t)=2; sizeof(uint32_t)=4;
// sizeof(int)=2; sizeof(long)=4; sizeof(unsigned)=2;
#define wchar_t uint32_t
#else
#include <stdint.h> // uint32_t
#include <wchar.h>
// x86_64
// sizeof(wchar_t)=4; sizeof(size_t)=8; sizeof(uint32_t)=4;
// sizeof(int)=4; sizeof(long)=8; sizeof(unsigned)=4;
#endif


#ifdef __cplusplus
extern "C" {
#endif

uint8_t * get_utf8_value (uint8_t *pstart, wchar_t *pval);

#define FOREACH_U8STRING(p, pend, pval) for (; (p < pend) && (p = get_utf8_value(p, pval)); )

#ifdef __cplusplus
}
#endif

#endif // GET_UTF8_H
