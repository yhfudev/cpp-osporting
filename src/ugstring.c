/**
 * \file    ugstring.c
 * \brief   string related functions
 * \author  Yunhui Fu (yhfudev@gmail.com)
 * \version 1.0
 * \date    2020-02-10
 * \copyright GPL/BSD
 */

#include "osporting.h"

#include "ugstring.h"

////////////////////////////////////////////////////////////////////////////////
// string and memory

#ifndef program_to_ram
static char g_p2r_buf_01[50];
static char g_p2r_buf_02[50];
static char * g_p2r_buffers[] = {
    g_p2r_buf_01,
    g_p2r_buf_02,
  };
static int g_p2r_idx = 0;

char *
program_to_ram(const char * cstr_in)
{
    strcpy_P(g_p2r_buffers[g_p2r_idx], cstr_in);
    assert ((0 <= g_p2r_idx) && (g_p2r_idx < 2));
    g_p2r_idx = 1 - g_p2r_idx;
    return g_p2r_buffers[1 - g_p2r_idx];
}
#endif // strlen_P

// for test
#ifdef my_strlen_P
#undef my_strlen_P
#undef my_memmove_P
#undef read_byte_ram
#undef read_byte_rom
#endif
#ifdef my_strncmp_leftstatic
#undef my_strncmp_leftstatic
#undef my_strncmp_bothstatic
#endif
#define my_strncmp_leftstatic(p1, p2, max_sz)  my_strncmp_via_callback((p1), (const char *)(p2), (max_sz), read_byte_rom, read_byte_ram)
#define my_strncmp_bothstatic(p1, p2, max_sz)  my_strncmp_via_callback((p1),               (p2), (max_sz), read_byte_rom, read_byte_rom)

// why wrote a strlen_P()/memmove_P() by myself?
// Because some platform won't give the APIs (maybe because they have no split RAM/ROM address?)
int
my_strlen_P(const char *pstart)
{
    const char *p;
    assert(NULL != pstart);
    p = pstart;
    while (p && pgm_read_byte(p) != '\0') p ++;
    return (p - pstart);
}

void *
my_memmove_P(void *dest, const void *src, size_t sz)
{
    char c;
    const char *p;
    char *d;
    assert(NULL != src);
    assert(NULL != dest);
    if (sz < 1) {
        return dest;
    }
    if (dest == src) {
        return dest;
    }
    p = (const char *)src;
    d = (char *)dest;
    if ((p < d) && (d < p + sz)) {
        // TODO: copy from tail
        p += (sz-1);
        d += (sz-1);
        while(d >= (char *)dest) { c=pgm_read_byte(p); *d = c; p --; d --;}
        return dest;
    }
    while (p < ((char *)src + sz)) { c=pgm_read_byte(p); *d = c; p ++; d ++;}
    return dest;
}

char read_byte_ram(const char *str) { return *str; }
char read_byte_rom(const char *str) { return pgm_read_byte(str); }

int
my_strncmp_via_callback(const char *p1, const char *p2, size_t max_sz, read_byte_cb_t cb_read_byte_left, read_byte_cb_t cb_read_byte_right)
{
    size_t i;
    char c1;
    char c2;

    c1 = cb_read_byte_left(p1);
    c2 = cb_read_byte_right(p2);
    for (i = 0; (i < max_sz); i ++) {
        if (c1 < c2) {
            return -1;
        } else if (c1 > c2) {
            return 1;
        }
        if ((!c1) || (!c2)) {
            break;
        }
        p1 ++;
        p2 ++;
        c1 = cb_read_byte_left(p1);
        c2 = cb_read_byte_right(p2);
    }
    if (i >= max_sz) {
        return 0;
    }
    if (c1 < c2) {
        assert (c1 == 0);
        return -1;
    }
    if (c1 > c2) {
        assert (c2 == 0);
        return 1;
    }
    return 0;
}


#if defined(CIUT_ENABLED) && (CIUT_ENABLED == 1)
#include <ciut.h>

TEST_CASE( .name="prom-string-function", .description="test program ROM string functions.", .skip=0 ) {
#define CSTR_TEST1 PSTR("It's me!")
    const char * p_str1 = CSTR_TEST1;
#define TEST_MAX 20 // MAX_BUFFER_SEGMENT
    char buffer[((int)TEST_MAX/10)*10 + 1];

    SECTION("test parameters") {
        char buffer_comp[sizeof(buffer)];
        // memmove overlay test, though this would not happen if one is in RAM and one is in ROM
        // we test it for RAM

        // the start address of dest is out of src area
        memset(buffer,                      'A', sizeof(buffer)*2/5);
        memset(buffer + sizeof(buffer)*2/5, 'B', sizeof(buffer)*2/5);
        memset(buffer + sizeof(buffer)*4/5, 'C', sizeof(buffer)/5);
        buffer[sizeof(buffer) - 1] = 0;
        CIUT_LOG("orig=%s", buffer);
        CIUT_LOG("move (%d) bytes to pos(%d), from pos(%d)", sizeof(buffer)*2/5, sizeof(buffer)/5, sizeof(buffer)/2);
        my_memmove_P(buffer + sizeof(buffer)/5, buffer + sizeof(buffer)/2, sizeof(buffer)*2/5);
        // fill compare value
        memset(buffer_comp,                             'A', sizeof(buffer_comp)/5);
        memset(buffer_comp + sizeof(buffer_comp)/5,     'B', sizeof(buffer_comp)*3/10);
        memset(buffer_comp + sizeof(buffer_comp)*5/10,  'C', sizeof(buffer_comp)/10);
        memset(buffer_comp + sizeof(buffer_comp)*3/5,   'B', sizeof(buffer_comp)/5);
        memset(buffer_comp + sizeof(buffer_comp)*4/5,   'C', sizeof(buffer_comp)/5);
        buffer_comp[sizeof(buffer_comp) - 1] = 0;
        CIUT_LOG("func=%s", buffer);
        CIUT_LOG("comp=%s", buffer_comp);
        REQUIRE(0 == memcmp(buffer, buffer_comp, sizeof(buffer)));

        CIUT_LOG("----------- %s -----------", "This is a line");
        // the start address of dest is inside src area.
        memset(buffer,                      'A', sizeof(buffer)/5);
        memset(buffer + sizeof(buffer)/5,   'B', sizeof(buffer)*2/5);
        memset(buffer + sizeof(buffer)*3/5, 'C', sizeof(buffer)*2/5);
        buffer[sizeof(buffer) - 1] = 0;
        CIUT_LOG("orig=%s", buffer);
        CIUT_LOG("move (%d) bytes to pos(%d), from pos(%d)", sizeof(buffer)*2/5, sizeof(buffer)*3/10, 0);
        my_memmove_P(buffer + sizeof(buffer)*3/10, buffer, sizeof(buffer)*2/5);
        // fill compare value
        memset(buffer_comp,                             'A', sizeof(buffer_comp)/5);
        memset(buffer_comp + sizeof(buffer_comp)/5,     'B', sizeof(buffer_comp)/10);
        memset(buffer_comp + sizeof(buffer_comp)*3/10,  'A', sizeof(buffer_comp)/5);
        memset(buffer_comp + sizeof(buffer_comp)*5/10,  'B', sizeof(buffer_comp)/5);
        memset(buffer_comp + sizeof(buffer_comp)*7/10,  'C', sizeof(buffer_comp)*3/10);
        buffer_comp[sizeof(buffer_comp) - 1] = 0;
        CIUT_LOG("func=%s", buffer);
        CIUT_LOG("comp=%s", buffer_comp);
        REQUIRE(0 == memcmp(buffer, buffer_comp, sizeof(buffer)));

    }
    SECTION("test functions") {
        REQUIRE((sizeof(CSTR_TEST1) - 1) == my_strlen_P(p_str1));

        REQUIRE(0 == my_strncmp_bothstatic(p_str1, p_str1, my_strlen_P(p_str1)));
        REQUIRE(0 == my_strncmp_bothstatic(p_str1, p_str1, sizeof(CSTR_TEST1)));
        REQUIRE(0 == my_strncmp_bothstatic(p_str1, p_str1, sizeof(CSTR_TEST1) + 1));

        REQUIRE(buffer == my_memmove_P (buffer, p_str1, my_strlen_P(p_str1)+1));
        REQUIRE(strlen(buffer) == my_strlen_P(p_str1));
        REQUIRE(strlen(buffer) == sizeof(CSTR_TEST1)-1);
        REQUIRE(0 == my_strncmp_leftstatic(p_str1, buffer, my_strlen_P(p_str1)));
        REQUIRE(0 == my_strncmp_leftstatic(p_str1, buffer, sizeof(CSTR_TEST1) - 1));
        REQUIRE(0 == my_strncmp_leftstatic(p_str1, buffer, sizeof(CSTR_TEST1)));
        REQUIRE(0 == my_strncmp_leftstatic(p_str1, buffer, sizeof(CSTR_TEST1) + 1));
        REQUIRE(0 == my_strncmp_rightstatic(buffer, p_str1, my_strlen_P(p_str1)));
        REQUIRE(0 == my_strncmp_rightstatic(buffer, p_str1, sizeof(CSTR_TEST1) - 1));
        REQUIRE(0 == my_strncmp_rightstatic(buffer, p_str1, sizeof(CSTR_TEST1)));
        REQUIRE(0 == my_strncmp_rightstatic(buffer, p_str1, sizeof(CSTR_TEST1) + 1));

        REQUIRE(strlen(buffer) > 0);
        buffer[strlen(buffer) - 1] ++;
        REQUIRE(strlen(buffer) > 0);
        REQUIRE(0 > my_strncmp_leftstatic(p_str1, buffer, my_strlen_P(p_str1)));
        REQUIRE(0 > my_strncmp_leftstatic(p_str1, buffer, sizeof(CSTR_TEST1) - 1));
        REQUIRE(0 > my_strncmp_leftstatic(p_str1, buffer, sizeof(CSTR_TEST1)));
        REQUIRE(0 > my_strncmp_leftstatic(p_str1, buffer, sizeof(CSTR_TEST1) + 1));
        REQUIRE(0 < my_strncmp_rightstatic(buffer, p_str1, my_strlen_P(p_str1)));
        REQUIRE(0 < my_strncmp_rightstatic(buffer, p_str1, sizeof(CSTR_TEST1) - 1));
        REQUIRE(0 < my_strncmp_rightstatic(buffer, p_str1, sizeof(CSTR_TEST1)));
        REQUIRE(0 < my_strncmp_rightstatic(buffer, p_str1, sizeof(CSTR_TEST1) + 1));
        REQUIRE(strlen(buffer) > 0);
        buffer[strlen(buffer) - 1] -= 2;
        REQUIRE(strlen(buffer) > 0);
        REQUIRE(0 < my_strncmp_leftstatic(p_str1, buffer, my_strlen_P(p_str1)));
        REQUIRE(0 < my_strncmp_leftstatic(p_str1, buffer, sizeof(CSTR_TEST1) - 1));
        REQUIRE(0 < my_strncmp_leftstatic(p_str1, buffer, sizeof(CSTR_TEST1)));
        REQUIRE(0 < my_strncmp_leftstatic(p_str1, buffer, sizeof(CSTR_TEST1) + 1));
        REQUIRE(0 > my_strncmp_rightstatic(buffer, p_str1, my_strlen_P(p_str1)));
        REQUIRE(0 > my_strncmp_rightstatic(buffer, p_str1, sizeof(CSTR_TEST1) - 1));
        REQUIRE(0 > my_strncmp_rightstatic(buffer, p_str1, sizeof(CSTR_TEST1)));
        REQUIRE(0 > my_strncmp_rightstatic(buffer, p_str1, sizeof(CSTR_TEST1) + 1));

#undef CSTR_TEST1
    }
}

TEST_CASE( .name="prom-string-function2", .description="test program ROM string functions.", .skip=0 ) {
#define TEST_MAX 20 // MAX_BUFFER_SEGMENT
    char buffer[((int)TEST_MAX/10)*10 + 1];

    SECTION("test functions") {
#define CSTR_TEST1 PSTR("*IDN?")
#define CSTR_TEST2 PSTR("*HELP?")
        REQUIRE(buffer == my_memmove_P (buffer, CSTR_TEST2, my_strlen_P(CSTR_TEST1)+1));
        REQUIRE(0 < my_strncmp_leftstatic(CSTR_TEST1, buffer, my_strlen_P(CSTR_TEST1)));
        REQUIRE(0 > my_strncmp_rightstatic(buffer, CSTR_TEST1, my_strlen_P(CSTR_TEST1)));

        REQUIRE(buffer == my_memmove_P (buffer, CSTR_TEST1, my_strlen_P(CSTR_TEST1)+1));
        REQUIRE(0 > my_strncmp_leftstatic(CSTR_TEST2, buffer, my_strlen_P(CSTR_TEST1)));
        REQUIRE(0 < my_strncmp_rightstatic(buffer, CSTR_TEST2, my_strlen_P(CSTR_TEST1)));
    }
}

#endif /* CIUT_ENABLED */

