/**
 * \file    ugosarduino.c
 * \brief   porting Arduino functions to other OS
 * \author  Yunhui Fu (yhfudev@gmail.com)
 * \version 1.0
 * \date    2020-02-10
 * \copyright GPL/BSD
 */

#include "osporting.h"
#include "ugosarduino.h"

////////////////////////////////////////////////////////////////////////////////
// Arduino related

#if ! defined(ARDUINO)
#include <sys/time.h> // gettimeofday()
#include <time.h>

static struct timeval tv_orig = {0, 0, }; //{.tv_sec = 0, .tv_usec = 0, };
unsigned long
millis(void)
{
    struct timeval tv;
    //struct timezone tz;
    //gettimeofday(&tv,&tz);
    gettimeofday(&tv, NULL);
    if (tv_orig.tv_sec <= 0) {
        tv_orig = tv;
        return 0;
    }
    unsigned long v = (tv.tv_sec - tv_orig.tv_sec) * 1000 - (tv_orig.tv_usec / 1000) + tv.tv_usec / 1000;
    //TRACE ("millis=%lu", v);
    return v;
}

unsigned long micros(void)
{
    struct timeval tv;
    //struct timezone tz;
    //gettimeofday(&tv,&tz);
    gettimeofday(&tv, NULL);
    if (tv_orig.tv_sec <= 0) {
        tv_orig = tv;
        return 0;
    }
    unsigned long v = (tv.tv_sec - tv_orig.tv_sec) * 1000000 - (tv_orig.tv_usec) + tv.tv_usec;
    //TRACE ("millis=%lu", v);
    return v;
}


#define PIN_LED_PWM 11  // the led for PWM(fade) control

#define PIN_MSCONF      12 // config the device as master or slave
#define PIN_PWR_CTRL    5
#define PIN_SW_ONOFF    3
#define PIN_ADC_BATTPWR A0
#define PIN_ADC_EXTPWR  A1

#define LED_BUILTIN 13


#define BUTSW_TIMEOUT_DBOUNCE   30

void
analogWrite (uint8_t pin, int val)
{
    assert (LED_BUILTIN != pin);
    return;
}

static unsigned long m_tm_pre = 0;
static int m_digi_val = 0;
int
digitalRead(uint8_t pin)
{
    if (pin == PIN_MSCONF) {
        return 0;
    }
    if (pin != PIN_SW_ONOFF) {
        return 0;
    }
    unsigned long now = millis();
    if (m_digi_val) {
        if (m_tm_pre + BUTSW_TIMEOUT_DBOUNCE * 5 > now) {
            return 1;
        }
    }
    m_digi_val = 0;
    // return 1 very seconds
    int randv = rand() % 100;
#define TIME_TEST 100
    if (m_tm_pre + (TIME_TEST / 3 + randv) < now) {
        m_tm_pre = now;
        m_digi_val = 1;
        return 1;
    }
    return 0;
}
#endif // ARDUINO
