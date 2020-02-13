
AM_LDFLAGS=
AM_CFLAGS=

DEFS+= \
    `getconf LFS_CFLAGS` \
    `getconf LFS64_CFLAGS` \
    -D_GNU_SOURCE \
    -D_FILE_OFFSET_BITS=64 \
    -DHAVE_MMAP64=1 \
    $(NULL)

AM_CFLAGS+= \
    -I$(top_srcdir)/src/ \
    -I$(top_srcdir)/include/ \
    -I$(top_builddir)/ \
    -I$(top_builddir)/src/ \
    -I$(top_builddir)/include/ \
    $(NULL)

AM_LDFLAGS += \
    -L$(top_builddir)/src/ \
    `getconf LFS_LDFLAGS` \
    `getconf LFS64_LDFLAGS` \
    $(NULL)

if DEBUG
# use "valgrind --tool=memcheck --leak-check=yes" to check memory leak, MemWatch will drag the program.
#DEFS+=-DMEMWATCH
DEFS+= -DDEBUG=1
AM_CFLAGS+=-g -Wall

else
AM_CFLAGS+=-O3 -Wall
endif
################################################################################

SRC_BASE= \
    ugdebug.cpp \
    ugosarduino.c \
    ugostime.c \
    ugstring.c \
    getline.c \
    getutf8.c \
    $(NULL)

include_HEADERS = \
    getline.h \
    getutf8.h \
    tree-bsd.h \
    ringbuffer.h \
    hexdump.h \
    osporting.h \
    ugdebug.h \
    ugosarduino.h \
    ugosbase.h \
    ugostime.h \
    ugostypes.h \
    ugstring.h \
    $(NULL)

noinst_HEADERS= \
    $(NULL)

lib_LTLIBRARIES=libosporting.la

libosporting_la_SOURCES=$(SRC_BASE)

libosporting_la_CFLAGS=-Isrc `sdl2-config --cflags`
#libosporting_la_CPPFLAGS=$(libosporting_la_CFLAGS)
libosporting_la_LDFLAGS=`sdl2-config --libs` -lz

