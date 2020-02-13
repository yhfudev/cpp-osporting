/**
 * \file    testos.cpp
 * \brief   Test porting library
 * \author  Yunhui Fu (yhfudev@gmail.com)
 * \version 1.0
 * \date    2020-02-09
 * \copyright GPL/BSD
 */

#include "osporting.h"

// debug
#include "hexdump.h"
#ifndef hex_dump_to_fd
#define hex_dump_to_fd(a,b,c)
#endif
#ifndef hex_dump_to_fp
#define hex_dump_to_fp(a,b,c)
#endif

void setup()
{
#if defined(ARDUINO)
    Serial.begin(9600);
    // Wait for USB Serial.
    while (!Serial) {}
    delay(200);
    // Read any input
    while (Serial.read() >= 0) {}
#endif
  delay(100);
  TD("millis()=%d", millis());

  char buf[] = "test string";
  TD("dump:");
  hex_dump_to_fp(stdout, buf, sizeof(buf));
}

void loop()
{
}


