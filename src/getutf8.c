/**
 * @file    getutf8.c
 * @brief   get utf-8 string value
 * @author  Yunhui Fu (yhfudev@gmail.com)
 * @version 1.0
 * @date    2015-06-19
 * @copyright GPL/BSD
 */

#include "osporting.h"
#include "getutf8.h"

/**
 * @brief conver Unicode value to UTF-8
 *
 * @param val the unicode value
 * @param buf the start of UTF-8 array
 * @param num_item the number of uint8_t items in the array
 *
 * @return the number of uint8_t in the output buffer
 *
 */
ssize_t
to_utf8(utf32_t c, uint8_t * buf, size_t num_item)
{
    uint8_t *out = buf;
    uint8_t *outend = buf + num_item;
    int bits;

    if (num_item < 1) {
        return -1;
    }
    if (NULL == buf) {
        return -1;
    }
    if ((0xD800 <= c) && (c <= 0xDFFF)) {
        /* surrogates */
        TE("invalid unicode value");
        return -1;
    }
    if      (c <    0x80) {  *out++=  c;                bits= -6; }
    else if (c <   0x800) {  *out++= ((c >>  6) & 0x1F) | 0xC0;  bits=  0; }
    else if (c < 0x10000) {  *out++= ((c >> 12) & 0x0F) | 0xE0;  bits=  6; }
    else                  {  *out++= ((c >> 18) & 0x07) | 0xF0;  bits= 12; }
    for ( ; bits >= 0; bits-= 6) {
        if (out >= outend) {
            TE("no enough buffer");
            return -1; //break;
        }
        *out++= ((c >> bits) & 0x3F) | 0x80;
    }

    return out - buf;
}

/**
 * @brief conver Unicode value to UTF-16
 *
 * @param val the unicode value
 * @param buf the start of UTF-16 array
 * @param num_item the number of uint16_t items in the array
 *
 * @return the number of uint16_t in the output buffer
 *
 * The function can handle UTF-16 surrogate pairs
 *
 */
ssize_t
to_utf16(utf32_t val, uint16_t * buf, size_t num_item)
{
    if (num_item < 1) {
        TE("no enough buffer");
        return -1;
    }
    if (NULL == buf) {
        return -1;
    }
    if ((val >= 0x10000) && (num_item < 2)) {
        TE("no enough buffer");
        return -1;
    }
    if ((0xD800 <= val) && (val <= 0xDFFF)) {
        /* surrogates */
        TE("invalid unicode value: 0x%X", val);
        return -1;
    }
    if (val >= 0x10000) {
        val -= 0x10000;
        buf[0] = 0xD800 | (val >> 10);
        buf[1] = 0xDC00 | (val & 0x3FF);
        return 2;
    }
    buf[0] = val;
    return 1;
}

// TODO: reject U+D800—U+DFFF
static utf32_t
get_val_utf8_uni (uint8_t *pstart)
{
    size_t cntleft;
    utf32_t retval = 0;

    if (0 == (0x80 & *pstart)) {
        return *pstart;
    }

    if (((*pstart & 0xE0) ^ 0xC0) == 0) {
        cntleft = 1;
        retval = *pstart & ~0xE0;
    } else if (((*pstart & 0xF0) ^ 0xE0) == 0) {
        cntleft = 2;
        retval = *pstart & ~0xF0;
    } else if (((*pstart & 0xF8) ^ 0xF0) == 0) {
        cntleft = 3;
        retval = *pstart & ~0xF8;
    } else if (((*pstart & 0xFC) ^ 0xF8) == 0) {
        cntleft = 4;
        retval = *pstart & ~0xFC;
    } else if (((*pstart & 0xFE) ^ 0xFC) == 0) {
        cntleft = 5;
        retval = *pstart & ~0xFE;
    } else {
        /* encoding error */
        cntleft = 0;
        retval = 0;
    }
    pstart ++;
    for (; cntleft > 0; cntleft --) {
        retval <<= 6;
        retval |= *pstart & 0x3F;
        pstart ++;
    }
    return retval;
}

// TODO: reject U+D800—U+DFFF
/**
 * @brief 转换 UTF-8 编码的一个字符为本地的 Unicode 字符(utf32_t)
 *
 * @param pstart 存储 UTF-8 字符的指针
 * @param pval 需要返回的 Unicode 字符存放地址指针
 *
 * @return 成功返回下个 UTF-8 字符的位置
 *
 * 转换 UTF-8 编码的一个字符为本地的 Unicode 字符(utf32_t)
 */
uint8_t *
get_utf8_value (uint8_t *pstart, utf32_t *pval)
{
    uint32_t val = 0;
    uint8_t *p = pstart;
    /*size_t maxlen = strlen (pstart);*/

    assert (NULL != pstart);

    if (0 == (0x80 & *p)) {
        val = (size_t)*p;
        p ++;
    } else if (0xC0 == (0xE0 & *p)) {
        val = *p & 0x1F;
        val <<= 6;
        p ++;
        val |= (*p & 0x3F);
        p ++;
        assert ((utf32_t)val == get_val_utf8_uni (pstart));
    } else if (0xE0 == (0xF0 & *p)) {
        val = *p & 0x0F;
        val <<= 6; p ++;
        val |= (*p & 0x3F);
        val <<= 6; p ++;
        val |= (*p & 0x3F);
        p ++;
        assert ((utf32_t)val == get_val_utf8_uni (pstart));
    } else if (0xF0 == (0xF8 & *p)) {
        val = *p & 0x07;
        val <<= 6; p ++;
        val |= (*p & 0x3F);
        val <<= 6; p ++;
        val |= (*p & 0x3F);
        val <<= 6; p ++;
        val |= (*p & 0x3F);
        p ++;
        assert ((utf32_t)val == get_val_utf8_uni (pstart));
    } else if (0xF8 == (0xFC & *p)) {
        val = *p & 0x03;
        val <<= 6; p ++;
        val |= (*p & 0x3F);
        val <<= 6; p ++;
        val |= (*p & 0x3F);
        val <<= 6; p ++;
        val |= (*p & 0x3F);
        val <<= 6; p ++;
        val |= (*p & 0x3F);
        p ++;
        assert ((utf32_t)val == get_val_utf8_uni (pstart));
    } else if (0xFC == (0xFE & *p)) {
        val = *p & 0x01;
        val <<= 6; p ++;
        val |= (*p & 0x3F);
        val <<= 6; p ++;
        val |= (*p & 0x3F);
        val <<= 6; p ++;
        val |= (*p & 0x3F);
        val <<= 6; p ++;
        val |= (*p & 0x3F);
        val <<= 6; p ++;
        val |= (*p & 0x3F);
        p ++;
        assert ((utf32_t)val == get_val_utf8_uni (pstart));
    } else if (0x80 == (0xC0 & *p)) {
        for (; 0x80 == (0xC0 & *p); p++ ) { }
    } else {
        for (; ((0xFE & *p) > 0xFC); p++ ) { }
    }

    if (pval) *pval = val;

    return p;
}

/**
 * @brief conver UTF-16 array to Unicode value (utf32_t)
 *
 * @param pstart the start of UTF-16 array
 * @param pval the pointer to store the unicode value
 *
 * @return the unicode value
 *
 * The function can handle UTF-16 surrogate pairs
 *
 */
static utf32_t
get_val_utf16_uni (uint16_t *pstart)
{
    utf32_t c = 0;

    assert(sizeof(utf32_t) >= 4);

    if (0xD800 <= pstart[0] && pstart[0] <= 0xDBFF) {
        if (0xDC00 <= pstart[1] && pstart[1] <= 0xDFFF) {

            c = 0x10000;
            c += (pstart[0] & 0x03FF) << 10;
            c += (pstart[1] & 0x03FF);
            return c;
        } else {
            // error
            TE("code error");
            return 0;
        }
    }
    c = pstart[0];
    return c;
}

/**
 * @brief conver UTF-16 array to Unicode value (utf32_t)
 *
 * @param pstart the start of UTF-16 array
 * @param pval the pointer to store the unicode value
 *
 * @return the next UTF-16 char pointer
 *
 * The function can handle UTF-16 surrogate pairs
 *
 */
uint16_t *
get_utf16_value (uint16_t *pstart, utf32_t *pval)
{
    utf32_t c = 0;

    assert(sizeof(utf32_t) >= 4);

    if (0xD800 <= pstart[0] && pstart[0] <= 0xDBFF) {
        if (0xDC00 <= pstart[1] && pstart[1] <= 0xDFFF) {

            c = 0x10000;
            c += (pstart[0] & 0x03FF) << 10;
            c += (pstart[1] & 0x03FF);
            if (pval) *pval = c;
            return pstart + 2;
        } else {
            // error
            TE("code error");
            return NULL; //nullptr;
        }
    }
    if (pval) *pval = pstart[0];
    return pstart + 1;
}


////////////////////////////////////////////////////////////////////////////////
#if defined(CIUT_ENABLED) && (CIUT_ENABLED == 1)
#include <ciut.h>

struct utf8_utf32 {
  uint8_t  *utf8;
  size_t   sz_utf8;
  uint32_t utf32;
};

static const uint8_t   utf8_0[] = { 0x41, };
#define utf32_u8_0   0x00000041

static const uint8_t   utf8_1[] = { 0xC3, 0xA9, };
#define utf32_u8_1   0x000000E9

static const uint8_t   utf8_2[] = { 0xC3, 0xB6, };
#define utf32_u8_2   0x000000F6

static const uint8_t   utf8_3[] = { 0xD0, 0x96, };
#define utf32_u8_3   0x00000416

static const uint8_t   utf8_4[] = { 0xE2, 0x82, 0xAC, };
#define utf32_u8_4   0x000020AC

static const uint8_t   utf8_5[] = { 0xF0, 0x9D, 0x84, 0x9E, };
#define utf32_u8_5   0x001D11E

static const uint8_t   utf8_6[] = { 0xF4, 0x8F, 0xBF, 0xBF, };
#define utf32_u8_6   0x0010FFFF

static const uint8_t   utf8_surrogate[] = { 0xED, 0xB2, 0x80, };
#define utf32_u8_surrogate   0x0000DC80

static const struct utf8_utf32 utf8_pair[] = {
#define PAIR(num) {utf8_##num, NUM_ARRAY(utf8_##num), utf32_u8_##num}
  PAIR(0),
  PAIR(1),
  PAIR(2),
  PAIR(3),
  PAIR(4),
  PAIR(5),
  PAIR(6),
  //PAIR(surrogate),
#undef PAIR
};

struct utf16_utf32 {
  uint8_t  *utf16be;
  size_t   sz_utf16be; // the number of items in the utf16
  uint32_t utf32;
};

static const uint8_t   utf16_0[] = { 0x00, 0x41, };
#define utf32_u16_0   0x00000041

static const uint8_t   utf16_1[] = { 0x00, 0xE9, };
#define utf32_u16_1   0x000000E9

static const uint8_t   utf16_2[] = { 0xdb, 0xea, 0xdf, 0xcd, };
#define utf32_u16_2   0x10abcd

static const struct utf16_utf32 utf16_pair[] = {
#define PAIR(num) {utf16_##num, NUM_ARRAY(utf16_##num), utf32_u16_##num}
  PAIR(0),
  PAIR(1),
  PAIR(2),
  //PAIR(3),
  //PAIR(surrogate),
#undef PAIR
};

#define MAX_UNICODE 0x110000

TEST_CASE( .name="get-utf8", .description="test UTF-8 function.", .skip=0 ) {

    SECTION("test get-utf8") {
        utf32_t ret_val;
        uint8_t *p;

        REQUIRE(sizeof(utf32_t) >= 4);
        for (int i = 0; i < NUM_ARRAY(utf8_pair); i ++) {
            p = get_utf8_value(utf8_pair[i].utf8, &ret_val);
            CIUT_LOG("[%d] unicode=0x%08X, utf8 sz=%d; got 0x%08X", i, utf8_pair[i].utf32, utf8_pair[i].sz_utf8, ret_val * sizeof(uint8_t));
            REQUIRE(utf8_pair[i].utf32 == ret_val);
            REQUIRE(utf8_pair[i].utf32 == get_val_utf8_uni(utf8_pair[i].utf8));
            REQUIRE(p == utf8_pair[i].utf8 + utf8_pair[i].sz_utf8);
        }
    }

    SECTION("basic get_utf8_value") {
        utf32_t ret_val;
        uint8_t buf[10];
        buf[0] = 0xFE;
        buf[1] = 0;
        REQUIRE(buf + 1 == get_utf8_value(buf, &ret_val));
        REQUIRE(0 == ret_val);
        buf[0] = 0;
        buf[1] = 0;
        get_utf8_value(buf, &ret_val);
        CIUT_LOG("get_utf8_value()=0x%02X", ret_val);
        REQUIRE(0 == get_val_utf16_uni(buf));
        REQUIRE(buf + 1 == get_utf8_value(buf, &ret_val));
        REQUIRE(0 == ret_val);
    }
    SECTION("basic to-utf8") {
        uint8_t buf[10];
        REQUIRE(-1 == to_utf8(0, nullptr, 0));
        REQUIRE(-1 == to_utf8(0, nullptr, 1));
        REQUIRE(-1 == to_utf8(0, buf, 0));
        REQUIRE(1 == to_utf8(0, buf, 1));
        REQUIRE(1 == to_utf8(0, buf, 2));
        REQUIRE(1 == to_utf8(0, buf, NUM_ARRAY(buf)));

        REQUIRE(-1 == to_utf8(MAX_UNICODE, buf, 1));
        REQUIRE(-1 == to_utf8(MAX_UNICODE, buf, 2));
        //REQUIRE(-1 == to_utf8(MAX_UNICODE, buf, NUM_ARRAY(buf)));
        REQUIRE(4 == to_utf8(MAX_UNICODE, buf, NUM_ARRAY(buf)));
    }
    SECTION("test to-utf8") {
        utf32_t ret_val;
        ssize_t ret;
        uint8_t buf[10];

        REQUIRE(sizeof(utf32_t) >= 4);
        for (int i = 0; i < NUM_ARRAY(utf8_pair); i ++) {
            ret = to_utf8(utf8_pair[i].utf32, buf, NUM_ARRAY(buf));
            CIUT_LOG("[%d] unicode=0x%08X, utf8 sz=%d; ret=%d", i, utf8_pair[i].utf32, utf8_pair[i].sz_utf8, ret);
            REQUIRE(ret == utf8_pair[i].sz_utf8);
            REQUIRE(utf8_pair[i].utf32 == get_val_utf8_uni(buf));
            REQUIRE(0 == memcmp(utf8_pair[i].utf8, buf, ret * sizeof(uint8_t)));
        }
    }
    SECTION("full to-utf8") {
        utf32_t ret_val;
        ssize_t ret;
        uint8_t buf[10];
        uint8_t *p;

        REQUIRE(sizeof(utf32_t) >= 4);
        for (int i = 0; i < MAX_UNICODE; i ++) {
            ret = to_utf8(i, buf, NUM_ARRAY(buf));
            if ((0xD800 <= i) && (i <= 0xDFFF)) {
                REQUIRE(ret < 0);
            } else {
                //CIUT_LOG("[%d] unicode=0x%08X, ret=%d", i, i, ret);
                REQUIRE(ret > 0);
                REQUIRE(buf + ret == get_utf8_value(buf, &ret_val));
                REQUIRE(i == ret_val);
            }
        }
    }
}

TEST_CASE( .name="get-utf16", .description="test UTF-16BE function.", .skip=0 ) {

    SECTION("test get-utf16") {
        utf32_t ret_val;
        uint16_t *p;
        uint16_t buf[2];

        REQUIRE(sizeof(utf32_t) >= 4);

        for (int i = 0; i < NUM_ARRAY(utf16_pair); i ++) {
            // convert to local buffer from bigendian
            for (int j = 0; j * sizeof(uint16_t) < utf16_pair[i].sz_utf16be; j ++) {
                buf[j] = (utf16_pair[i].utf16be[j*2+0] & 0xFF);
                buf[j] <<= 8;
                buf[j] |= (utf16_pair[i].utf16be[j*2+1] & 0xFF);
            }
            CIUT_LOG("[%d] buf[0]=0x%02X, buf[1]=0x%02X", i, buf[0], buf[1]);
            p = get_utf16_value(buf, &ret_val);
            CIUT_LOG("[%d] unicode=0x%08X, utf16 sz=%d; got 0x%08X", i, utf16_pair[i].utf32, utf16_pair[i].sz_utf16be, ret_val * sizeof(uint16_t));
            REQUIRE(utf16_pair[i].utf32 == ret_val);
            REQUIRE(utf16_pair[i].utf32 == get_val_utf16_uni(buf));
            REQUIRE(p == buf + (utf16_pair[i].sz_utf16be/sizeof(uint16_t)) );
        }
    }

    SECTION("basic get_val_utf16_uni") {
        uint16_t buf[10];
        buf[0] = 0xD800;
        buf[1] = 0;
        REQUIRE(0 == get_val_utf16_uni(buf));
        buf[0] = 0xD800;
        buf[1] = 0xD800;
        REQUIRE(0 == get_val_utf16_uni(buf));
        buf[0] = 0xD800;
        buf[1] = 0xDC00;
        CIUT_LOG("get_val_utf16_uni()=0x%02X", get_val_utf16_uni(buf));
        REQUIRE(0x10000 == get_val_utf16_uni(buf));
    }
    SECTION("basic get_utf16_value") {
        utf32_t ret_val;
        uint16_t buf[10];
        buf[0] = 0xD800;
        buf[1] = 0;
        REQUIRE(NULL == get_utf16_value(buf, &ret_val));
        buf[0] = 0xD800;
        buf[1] = 0xD800;
        REQUIRE(NULL == get_utf16_value(buf, &ret_val));
        buf[0] = 0xD800;
        buf[1] = 0xDC00;
        get_utf16_value(buf, &ret_val);
        CIUT_LOG("get_utf16_value()=0x%02X", ret_val);
        REQUIRE(0x10000 == get_val_utf16_uni(buf));
        REQUIRE(buf + 2 == get_utf16_value(buf, &ret_val));
        REQUIRE(0x10000 == ret_val);
    }
    SECTION("basic to-utf16") {
        uint16_t buf[10];
        REQUIRE(-1 == to_utf16(0, nullptr, 0));
        REQUIRE(-1 == to_utf16(0, nullptr, 1));
        REQUIRE(-1 == to_utf16(0, buf, 0));
        REQUIRE(1 == to_utf16(0, buf, 1));
        REQUIRE(1 == to_utf16(0, buf, 2));
        REQUIRE(1 == to_utf16(0, buf, NUM_ARRAY(buf)));
        REQUIRE(-1 == to_utf16(0x10001, buf, 1));
        REQUIRE(2 == to_utf16(0x10001, buf, 2));
    }
    SECTION("test to-utf16") {
        utf32_t ret_val;
        ssize_t ret;
        uint16_t buf[2];
        uint8_t buf2[4];

        REQUIRE(sizeof(utf32_t) >= 4);
        for (int i = 0; i < NUM_ARRAY(utf16_pair); i ++) {
            ret = to_utf16(utf16_pair[i].utf32, buf, NUM_ARRAY(buf));
            CIUT_LOG("[%d] unicode=0x%08X, utf16 sz=%d; ret=%d", i, utf16_pair[i].utf32, utf16_pair[i].sz_utf16be, ret);
            REQUIRE(ret * sizeof(uint16_t) == utf16_pair[i].sz_utf16be);
            // convert to bigendian
            for (int j = 0; j < ret; j ++) {
                buf2[j*2+0] = ((buf[j] >> 8) & 0xFF);
                buf2[j*2+1] = ((buf[j] >> 0) & 0xFF);
            }
            REQUIRE(utf16_pair[i].utf32 == get_val_utf16_uni(buf));
            REQUIRE(0 == memcmp(utf16_pair[i].utf16be, buf2, utf16_pair[i].sz_utf16be) );
        }
    }
    SECTION("full to-utf16") {
        utf32_t ret_val;
        ssize_t ret;
        uint16_t buf[10];
        uint16_t *p;
        int i;

        REQUIRE(sizeof(utf32_t) >= 4);
        i = 0;
        for (; i < MAX_UNICODE; i ++) {

            ret = to_utf16(i, buf, NUM_ARRAY(buf));
            //CIUT_LOG("[%d] unicode=0x%08X, utf16 ret=%d", i, i, ret);

            if ((0xD800 <= i) && (i <= 0xDFFF)) {
                REQUIRE(ret < 0);
            } else {
                p = get_utf16_value(buf, &ret_val);
                //CIUT_LOG("[%d] unicode=0x%08X, ret=%d, buf=%p, retp=%p", i, i, ret_val, buf, p);
                REQUIRE(i == ret_val);
                REQUIRE(buf + ret == p);
            }
        }
    }
}

#endif /* CIUT_ENABLED */


