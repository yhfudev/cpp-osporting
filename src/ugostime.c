/**
 * \file    ugostime.c
 * \brief   time related functions
 * \author  Yunhui Fu (yhfudev@gmail.com)
 * \version 1.0
 * \date    2020-02-10
 * \copyright GPL/BSD
 */

#include "ugosbase.h"
#include "ugostime.h"

////////////////////////////////////////////////////////////////////////////////
// time
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
ssize_t
timeval_format(struct timeval *tv, char flg_use_localtime, char *buf, size_t sz)
{
    int w;
    ssize_t sz_all = -1;
    struct tm timeinfo;
    struct tm *gm = nullptr; // &timeinfo;

    if (flg_use_localtime) {
        //gm = localtime(&(tv->tv_sec));
        gm = &timeinfo; localtime_r(&(tv->tv_sec), gm);
    } else {
        //gm = gmtime(&(tv->tv_sec));
        gm = &timeinfo; gmtime_r(&(tv->tv_sec), gm);
    }
    if (! gm) {
        return -1;
    }
    sz_all = (ssize_t)strftime(buf, sz, "%Y-%m-%dT%H:%M:%S", gm);
    if ((sz_all > 0) && ((size_t)sz_all < sz)) {
        w = snprintf(buf + sz_all, sz - (size_t)sz_all, ".%06ld", tv->tv_usec);
        sz_all = (w > 0) ? sz_all + w : -1;
    }
    w = strftime(buf + sz_all, sz - (size_t)sz_all, "%z", gm);
    sz_all = (w > 0) ? sz_all + w : -1;
    return sz_all;
}
#endif

////////////////////////////////////////////////////////////////////////////////
#if defined(CIUT_ENABLED) && (CIUT_ENABLED == 1)
#include <ciut.h>

TEST_CASE( .name="gettimeofday", .description="test system gettimeofday.", .skip=0 ) {

    SECTION("test gettimeofday") {
        struct timeval tv = {0, 0};
        struct timezone tz = {0, 0};
        gettimeofday(&tv, &tz);
        CIUT_LOG("gettimeofday() return tv={%ld.%06d}; tz={%d,%d}.", tv.tv_sec, tv.tv_usec, tz.tz_minuteswest, tz.tz_dsttime);
        delay(1000);
        gettimeofday(&tv, &tz);
        CIUT_LOG("after delay, gettimeofday() return tv={%ld.%06d}; tz={%d,%d}.", tv.tv_sec, tv.tv_usec, tz.tz_minuteswest, tz.tz_dsttime);
    }
}

#endif /* CIUT_ENABLED */


