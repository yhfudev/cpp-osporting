/**
 * \file    ugostime.h
 * \brief   time related functions
 * \author  Yunhui Fu (yhfudev@gmail.com)
 * \version 1.0
 * \date    2015-07-15
 */
#ifndef UG_OS_TIME_H
#define UG_OS_TIME_H 1

#include "ugosbase.h"

////////////////////////////////////////////////////////////////////////////////
// time

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
// ESP32
#include <sys/time.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

ssize_t timeval_format(struct timeval *tv, char flg_use_localtime, char *buf, size_t sz);

#ifdef __cplusplus
}
#endif

#elif defined(ARDUINO)
// get time
struct tm {
  int tm_sec;   // seconds after the minute    0-61*
  int tm_min;   // minutes after the hour    0-59
  int tm_hour;  // hours since midnight    0-23
  int tm_mday;  // day of the month    1-31
  int tm_mon;   // months since January    0-11
  int tm_year;  // years since 1900
  int tm_wday;  // days since Sunday    0-6
  int tm_yday;  // days since January 1    0-365
  int tm_isdst; // Daylight Saving Time flag
};
#endif

////////////////////////////////////////////////////////////////////////////////
#endif // UG_OS_TIME_H
