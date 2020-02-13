/**
 * \file    hexdump.h
 * \brief   dump the binary content in hex
 * \author  Yunhui Fu <yhfudev@gmail.com>
 * \version 1.0
 * \date    2016-04-23
 */

#ifndef HEX_DUMP_TO_H
#define HEX_DUMP_TO_H 1

#if !defined(DEBUG) || (DEBUG == 0)
#define hex_dump_to_fd(fd, fragment, size)

#else // DEBUG
#include <string.h>
#include <ctype.h>              /* isprint() */
#include <unistd.h>
#include <assert.h>
#include <stdint.h> /* uint8_t */
#define opaque_t uint8_t
typedef int (*bhd_cb_writer_t) (void *fd, opaque_t * fragment, size_t size);

static int ostr_cb_writer_fdwrite (void *fd, opaque_t * fragment, size_t size);
static size_t bulk_hex_dump (void *fd, opaque_t * fragment, size_t size, bhd_cb_writer_t writer, int use_c_style);
#define hex_dump_to_fd(fd, fragment, size) bulk_hex_dump ((void *)(fd), (opaque_t *)(fragment), size, ostr_cb_writer_fdwrite, 0)
#define hex_dump_to_fp(fp, fragment, size) bulk_hex_dump ((void *)(fp), (opaque_t *)(fragment), size, ostr_cb_writer_fwrite, 0)

static int
ostr_cb_writer_fwrite (void *fp, opaque_t * fragment, size_t size)
{
    return fwrite (fragment, 1, size, (FILE *)fp);
}

static int
ostr_cb_writer_fdwrite (void *fd, opaque_t * fragment, size_t size)
{
    return write ((intptr_t)fd, fragment, size);
}

#if 0
static int
bhd_cb_writer_null (void *fd, opaque_t * fragment, size_t size)
{
    return size;
}
#endif // 0

/**
 * \brief output the binary data as HEX format
 * \param fd: the first parameter passed to the user callback function 'writer'
 * \param fragment: the start address of the binary data
 * \param size: the byte size of the binary data
 * \param writer: the output callback function defined by user
 * \param use_c_style: 1 -- the HEX format is for C source code
 */
static size_t
bulk_hex_dump (void *fd, opaque_t * fragment, size_t size, bhd_cb_writer_t writer, int use_c_style)
{
    bhd_cb_writer_t pwriter = NULL;
    size_t size_ret = 0;
    int ret = 0;
    size_t line_num = 0;
    size_t i = 0;
    uint8_t buffer[20];

    /* printf("the buffer data(size=%d=0x%08x):\n", size, size); */
    assert (NULL != writer);
    pwriter = writer;

    if (! use_c_style) {
#define CSTR_HL "\n           +0 +1 +2 +3 +4 +5 +6 +7   +8 +9 +A +B +C +D +E +F\n"
        ret = pwriter(fd, (opaque_t *)CSTR_HL, sizeof(CSTR_HL)-1);
#undef CSTR_HL
        if (ret > 0) {
            size_ret += ret;
        }
    }

    i = line_num << 4;
    while (i < size) {
        if (! use_c_style) {
            ret = sprintf ((char *)buffer, "%08xh: ", (unsigned int)i);
            assert (ret < (int)sizeof (buffer));
            if (ret > 0) {
                size_ret += ret;

                ret = pwriter (fd, buffer, ret);
                if (ret > 0) {
                    size_ret += ret;
                }
            }
        }
        while ((i < ((line_num + 1) << 4))) {
            if (i == (line_num << 4) + 8) {
                if (use_c_style) {
                    ret = pwriter (fd, (uint8_t *)" ", strlen (" "));
                } else {
                    ret = pwriter (fd, (uint8_t *)"- ", strlen ("- "));
                }
                if (ret > 0) {
                    size_ret += ret;
                }
            }

            if (i < size) {
                if (use_c_style) {
                    ret = sprintf ((char *)buffer, "0x%02X, ", fragment[i]);
                } else {
                    ret = sprintf ((char *)buffer, "%02X ", fragment[i]);
                }
                assert (ret < (int)sizeof (buffer));
                if (ret > 0) {
                    ret = pwriter (fd, buffer, ret);
                    if (ret > 0) {
                        size_ret += ret;
                    }
                }
            } else {
                ret = pwriter (fd, (uint8_t *)"   ", strlen ("   "));
                if (ret > 0) {
                    size_ret += ret;
                }
            }

            i++;
        }
        if (! use_c_style) {
            ret = pwriter (fd, (uint8_t *)"; ", strlen ("; "));
            if (ret > 0)
                size_ret += ret;
            i = line_num << 4;
            while ((i < ((line_num + 1) << 4)) && (i < size)) {
                char ch;
                if (i == (line_num << 4) + 8) {
                    ret = pwriter (fd, (uint8_t *)" ", strlen (" "));
                    if (ret > 0)
                        size_ret += ret;
                }
                ch = fragment[i];
                if (!isprint (ch)) {
                    ch = '.';
                }
                ret = sprintf ((char *)buffer, "%c", ch);
                assert (ret < (int)sizeof (buffer));
                ret = pwriter (fd, buffer, ret);
                if (ret > 0) {
                    size_ret += ret;
                }
                i++;
            }
        }
        ret = pwriter (fd, (uint8_t *)"\n", strlen ("\n"));
        if (ret > 0) {
            size_ret += ret;
        }
        line_num++;
        i = line_num << 4;
    }
    return size_ret;
}
#endif // DEBUG


#endif /* HEX_DUMP_TO_H */

