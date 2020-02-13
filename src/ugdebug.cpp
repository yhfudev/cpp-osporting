/**
 * \file    ugdebug.cpp
 * \brief   debug functions
 * \author  Yunhui Fu (yhfudev@gmail.com)
 * \version 1.0
 * \date    2015-06-19
 * \copyright GPL/BSD
 */

#include "ugdebug.h"


////////////////////////////////////////////////////////////////////////////////
// debug
#if defined(ARDUINO)

#if defined(DEBUG) && (DEBUG == 1)
static Stream * m_serial = &Serial;
void debug_set_serial(Stream * out) {m_serial = out;}
void
serial_printf_P (const char *format, ...)
{
    static char buff[256];
    va_list args;
    va_start (args,format);
    vsnprintf_P(buff,sizeof(buff),format,args);
    va_end (args);
    buff[sizeof(buff)/sizeof(buff[0])-1]='\0';
    m_serial->print(buff);
    //delay (1000);
}

#if defined(__AVR__)
// stdin, stdout, stderr: https://forum.arduino.cc/index.php?topic=149785.0
// to support floating point, put the file
// https://github.com/krupski/arduino-1.0.3/blob/master/app/pde.jar
// to arduino-1.0.x/lib/pde.jar
#include <stdio.h>
//#include <stdinout.h>

class initializeSTDINOUT
{
        static size_t initnum;
public:
        // Constructor
        initializeSTDINOUT();
};
// Call the constructor in each compiled file this header is included in
// static means the names won't collide
static initializeSTDINOUT initializeSTDINOUT_obj;

// Function that printf and related will use to print
static int serial_putchar(char c, FILE *f)
{
        if(c == '\n') {
                serial_putchar('\r', f);
        }

        return Serial.write(c) == 1 ? 0: 1;
}
// Function that scanf and related will use to read
static int serial_getchar(FILE *)
{
        // Wait until character is avilable
        while(Serial.available() <= 0) { ; }

        return Serial.read();
}

static FILE serial_stdinout;

static void setup_stdin_stdout()
{
        // Set up stdout and stdin
        fdev_setup_stream(&serial_stdinout, serial_putchar, serial_getchar, _FDEV_SETUP_RW);
        stdout = &serial_stdinout;
        stdin  = &serial_stdinout;
        stderr = &serial_stdinout;
}

// Initialize the static variable to 0
size_t initializeSTDINOUT::initnum = 0;

// Constructor that calls the function to set up stdin and stdout
initializeSTDINOUT::initializeSTDINOUT()
{
        if(initnum++ == 0) {
                setup_stdin_stdout();
        }
}

#endif // __AVR__


////////////////////////////////////////////////////////////////////////////////
// memory size

#if defined(__AVR__)
// show free memory for Arduino
// https://github.com/McNeight/MemoryFree.git

extern unsigned int __heap_start;
extern void *__brkval;

/*
 * The free list structure as maintained by the
 * avr-libc memory allocation routines.
 */
struct __freelist
{
    size_t sz;
    struct __freelist *nx;
};

/* The head of the free list structure */
extern struct __freelist *__flp;

int
freeListSize(void)
{
    struct __freelist* current;
    int total = 0;
    for (current = __flp; current; current = current->nx)
    {
        total += 2; /* Add two bytes for the memory block's header  */
        total += (int) current->sz;
    }

    return total;
}

int
freeMemory(void)
{
    int free_memory;
    if ((int)__brkval == 0)
    {
        free_memory = ((int)&free_memory) - ((int)&__heap_start);
    }
    else
    {
        free_memory = ((int)&free_memory) - ((int)__brkval);
        free_memory += freeListSize();
    }
    return free_memory;
}

#endif // __AVR__

#endif // DEBUG

#endif // ARDUINO

