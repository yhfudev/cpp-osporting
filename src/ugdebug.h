/**
 * \file    ugdebug.h
 * \brief   debug functions
 * \author  Yunhui Fu (yhfudev@gmail.com)
 * \version 1.0
 * \date    2015-06-19
 * \copyright GPL/BSD
 */
#ifndef UG_DEBUG_H
#define UG_DEBUG_H

#include "ugosbase.h"

#ifndef DEBUG
#define DEBUG 1
#endif

////////////////////////////////////////////////////////////////////////////////
// Debug and trace

#if defined(ARDUINO)

#if defined(DEBUG) && (DEBUG == 1)
#include <stdio.h>

#ifdef __cplusplus
//#define _STRIZE(a) #a
//#define _STRMK(a,b) a(b)
//#pragma message( __FILE__ "(" _STRMK( _STRIZE, __LINE__ ) "): here")
//#pragma message(_VAL(__cplusplus))
void debug_set_serial(Stream * out);
#endif // __cplusplus

//#define assert(a)
//#define assert(a) if (!(a)) {TRACE("Assert: " # a ); }

#endif

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
#include <assert.h>
#else
#define assert(a)
#endif

#endif // Arduino



#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void serial_printf_P(const char *format, ...);

#if defined(__AVR__)
int freeMemory(void);
int freeListSize(void);
//#include "user_interface.h"
//#define freeMemory() system_get_free_heap_size()
#else
#define freeMemory() (0)
#endif


#ifdef __cplusplus
}
#endif // __cplusplus


#if defined(__AVR__)
//#define TRACE(fmt, ...) {static const PROGMEM char CONSTSTR[] = "%d %d " fmt " {ln:%d, fn:" __FILE__ "}\n"; serial_printf_P (CONSTSTR, millis(), ##__VA_ARGS__, __LINE__);  }
//#define TRACE(fmt, ...) {static const PROGMEM char CONSTSTR[] = "%d %d " fmt " {%s:%d; freemem=%d}\n"; serial_printf_P (CONSTSTR, millis(), ##__VA_ARGS__, __FILE__, __LINE__, freeMemory());  }
//#define TRACE(fmt, ...) fprintf_P(stderr, PSTR("%d %d " fmt " {ln=%d; freemem=%d}\n"), millis(), ##__VA_ARGS__, __LINE__, freeMemory())
#define TRACE(fmt, ...) fprintf_P(stderr, PSTR(fmt "\n"), ##__VA_ARGS__)

#elif defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
#include <sys/time.h>
ssize_t timeval_format(struct timeval *tv, char flg_use_localtime, char *buf, size_t sz);
#define TRACE(fmt, ...) {struct timeval tv; gettimeofday(&tv, nullptr); char buf[30]; timeval_format(&tv, 1, buf, 30); serial_printf_P ("%s [%s()] " fmt " {ln:%d, fn:" __FILE__ "}\n", buf, __func__, ##__VA_ARGS__, __LINE__); }

#elif defined(ARDUINO)
//#define TRACE(fmt, ...) serial_printf_P ("[%s()] " fmt " {ln:%d, fn:" __FILE__ "}\n", __func__, ##__VA_ARGS__, __LINE__)
//#define TRACE(fmt, ...) {time_t now = time(nullptr); struct tm timeinfo; gmtime_r(&now, &timeinfo); serial_printf_P ("%s [%s()] " fmt " {ln:%d, fn:" __FILE__ "}\n", asctime(&timeinfo), __func__, ##__VA_ARGS__, __LINE__); }
//strftime(char *s, size_t max, const char *format, const struct tm *tm);
//#define TRACE(fmt, ...) {time_t now = time(nullptr); struct tm timeinfo; char buf[30]=""; strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S%z", &timeinfo); gmtime_r(&now, &timeinfo); serial_printf_P ("%s [%s()] " fmt " {ln:%d, fn:" __FILE__ "}\n", buf, __func__, ##__VA_ARGS__, __LINE__); }
#define TRACE(fmt, ...) {time_t now = time(nullptr); struct tm timeinfo; char buf[30]; gmtime_r(&now, &timeinfo); strcpy(buf, asctime(&timeinfo)); buf[strlen(buf)-1] = 0; serial_printf_P ("%s [%s()] " fmt " {ln:%d, fn:" __FILE__ "}\n", buf, __func__, ##__VA_ARGS__, __LINE__); }

#else
#define TRACE1(fmt, ...) { \
    struct timeval tvcur; \
    gettimeofday ((&tvcur), NULL); \
    fprintf (stderr, "%ld.%06ld\t" fmt "\t{ln:%d, fn:" __FILE__ "}\n", (tvcur.tv_sec), (tvcur.tv_usec), ##__VA_ARGS__, __LINE__); \
    fflush (stderr); \
}
#define TRACE(fmt, ...) fprintf (stderr, "[%s()] " fmt " {ln:%d, fn:" __FILE__ "}\n", __func__, ##__VA_ARGS__, __LINE__)
//#define assert(a) if (!(a)) {TRACE("Assert: " # a); my_exit(1);}

#endif


#ifndef TRACE
#define TRACE(...) (0)
#endif

#if defined(DEBUG) && (DEBUG == 1)
// trace debug info
#define TD(fmt,...) TRACE("(D) " fmt, ##__VA_ARGS__)
// trace warnings
#define TW(fmt,...) TRACE("(W) " fmt, ##__VA_ARGS__)
// trace errors
#define TE(fmt,...) TRACE("(E) " fmt, ##__VA_ARGS__)
#define TI(fmt,...) TRACE("(I) " fmt, ##__VA_ARGS__)

#else
#define TD(fmt,...)
#define TW(fmt,...)
#define TE(fmt,...)
#define TI(fmt,...)
#endif // DEBUG


#endif
