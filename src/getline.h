
#ifndef MYGETLINE_H
#define MYGETLINE_H 1

//#include "config.h"

#if ! defined (__arm__) && ! defined(__AVR__)

#if ! HAVE_GETLINE
#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif

ssize_t getdelim (char **lineptr, size_t *n, int delimiter, FILE *fp);
#define getline(lineptr, n, stream) getdelim (lineptr, n, '\n', stream)


#ifdef __cplusplus
}
#endif

#endif

#endif // ! defined (__arm__) && ! defined(__AVR__)

#endif // MYGETLINE_H
