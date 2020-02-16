/**
 * @file    getutf8.h
 * @brief   get utf-8 string value
 * @author  Yunhui Fu (yhfudev@gmail.com)
 * @version 1.0
 * @date    2015-06-19
 * @copyright GPL/BSD
 */
#ifndef _GET_UTF8_H
#define _GET_UTF8_H 1

#include "osporting.h"

#ifdef __cplusplus
extern "C" {
#endif

#define utf32_t uint32_t

ssize_t to_utf8(utf32_t val, uint8_t * buf, size_t sz_buf);
ssize_t to_utf16(utf32_t val, uint16_t * buf, size_t num_item);

uint8_t * get_utf8_value (uint8_t *pstart, utf32_t *pval);
uint16_t * get_utf16_value (uint16_t *pstart, utf32_t *pval);

#define FOREACH_U8STRING(p, pend, pval) for (; (p < pend) && (p = get_utf8_value(p, pval)); )
#define FOREACH_U16STRING(p, pend, pval) for (; (p < pend) && (p = get_utf16_value(p, pval)); )

#ifdef __cplusplus
}
#endif

#endif // _GET_UTF8_H
