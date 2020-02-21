/**
 * \file    ugosarduino.h
 * \brief   porting Arduino functions to other OS
 * \author  Yunhui Fu (yhfudev@gmail.com)
 * \version 1.0
 * \date    2020-02-10
 * \copyright GPL/BSD
 */
#ifndef UG_OS_ARDUINO_H
#define UG_OS_ARDUINO_H 1

#include "ugosbase.h"

////////////////////////////////////////////////////////////////////////////////
// Arduino related
#if defined(ARDUINO)
#define SDL_Delay(a) delay(a)

#else // ! ARDUINO

#ifdef __cplusplus
extern "C" {
#endif

#define LOW  0
#define HIGH 1
#define pinMode(pin, mode)

#define digitalWrite(pin, val) TRACE("digitalWrite(" # pin  ", " # val ")")
#define analogRead(pin) (0)

int digitalRead(uint8_t);

//#define analogWrite(pin,val) TRACE("analogWrite(" # pin  ", " # val ")")
void analogWrite (uint8_t pin, int val);

unsigned long millis(void);
unsigned long micros(void);

#define delay(a) usleep((a) * 1000)

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus__
class PcWire {
public:
    void begin() {}
    void begin(uint8_t I2C_SLAVE_ADDR) {}
    void onReceive( void (*)(int) ) {}
    void onRequest( void (*)(void) ) {}

    uint8_t read(void) {return 0;}
    void write(uint8_t *buf, size_t sz) { }
    void write(uint8_t val) { }
    void beginTransmission (uint8_t pin) {}
    int requestFrom(uint8_t addr, size_t sz) { return 0; }
    void endTransmission() {}
};
static PcWire Wire;

class PcSerial {
public:
    void begin(int baud) {}
    void println(int a) {}
    void println(float a) {}
    bool operator ! () { return true; }
};
static PcSerial Serial;
#endif


#endif // ARDUINO


#ifndef LED_BUILTIN
#ifdef __AVR_ATtiny85__
#define LED_BUILTIN 1
#define NOT_AN_INTERRUPT (-1)
// attiny8
//#define digitalPinToInterrupt(p) ( (p) == 2 ? 0 : NOT_AN_INTERRUPT)
// attiny14:
#define digitalPinToInterrupt(p) ( (p) == 8 ? 0 : NOT_AN_INTERRUPT)

#elif defined (STM32_MCU_SERIES)
#define LED_BUILTIN 33   // Maple Mini

#else
#define LED_BUILTIN 13
#endif

#endif // LED_BUILTIN


#endif // UG_OS_ARDUINO_H

