/**
 * \file    main.cpp
 * \brief   main function
 * \author  Yunhui Fu (yhfudev@gmail.com)
 * \version 1.0
 * \date    2020-02-09
 * \copyright GPL/BSD
 */

#if defined(ARDUINO_ARCH_ESP8266)
extern "C" void setup();
extern "C" void loop();
#else
void setup();
void loop();
#endif

#if ! defined(ARDUINO)
int main(void)
{
  setup();
  while(1) {
    loop();
  }
}
#endif

