/**
 * \file    ringbuffer.c
 * \brief   Ring buffer for write/read
 * \author  Yunhui Fu <yhfudev@gmail.com>
 * \version 1.0
 */

#include "ringbuffer.h"

#if defined(ARDUINO)
#define assert(a)

#if 1
#undef TD
#define TD(...)
#undef TI
#define TI(...)

#if 1 //#if !defined(DEBUG) || (DEBUG == 0)
#undef TW
#define TW(...)
#undef TE
#define TE(...)
#endif // 0

#endif // 1
#endif // ARDUINO

#if defined(DEBUG) && (DEBUG == 1)
#include "hexdump.h"
#endif
#ifndef hex_dump_to_fd
#define hex_dump_to_fd(a,b,c)
#endif
#ifndef hex_dump_to_fp
#define hex_dump_to_fp(a,b,c)
#endif


/**
 * init a ring buffer structure
 * \param prb the ring buffer structure
 * \param byte_size the byte size of the whole buffer
 * \return 0 on success; -1 on error
 */
int
rbuf_init(void *prb, size_t byte_size)
{
    ring_buffer_t *p = (ring_buffer_t *)prb;
    if ((byte_size) <= sizeof(ring_buffer_t)) {
        TE("no enough spare memory for both the ring buffer structure and data!");
        return -1;
    }
    memset((prb), 0, sizeof(ring_buffer_t));
    (p)->pos_write = 1;
    (p)->sz_buf = (byte_size) - sizeof(ring_buffer_t);
    (p)->buf1 = (unsigned char *)(p + 1);
    return 0;
}

/**
 * \brief write data to ring buffer
 * \param prb the ring buffer structure
 * \param buf the buffer to be writen
 * \param sz the size of buffer
 * \return the size of data written to the ring buffer; -1 on error
 */
ssize_t
rbuf_write(void *prb, uint8_t * buf, size_t sz)
{
    ring_buffer_t *p = (ring_buffer_t *)prb;
    size_t sz_wr = 0;

    assert (NULL != prb);
    if (sz < 1 || NULL == buf) {
        TE("input size parameter error!");
        return -1;
    }
    //assert ((p)->pos_write != (p)->pos_read);
    sz_wr = rbuf_spare(prb);
    if (sz_wr < 1) {
        TE("out of space!");
        return -1;
    }
    if (sz > sz_wr) {
        TD("adjust sz=%d to smaller spare size=%d.", sz, sz_wr);
        sz = sz_wr;
    }
    assert ((p)->pos_write != (p)->pos_read);
    assert (sz <= sz_wr);
    assert (sz > 0);
    assert (sz <= (p)->sz_buf);
    assert ((p)->sz_buf > (p)->pos_write);

    // the first part
    sz_wr = ((p)->sz_buf) - (p)->pos_write;
    if (sz_wr > sz) {
        sz_wr = sz;
    }
    TD("copy first part pos=%d, buf='%s', size=%d.", (p)->pos_write, buf, sz_wr);
    memmove ((p)->buf1 + (p)->pos_write, buf, sz_wr);
    (p)->pos_write = ((p)->pos_write + sz_wr) % ((p)->sz_buf);

    // second part
    if (sz_wr < sz) {
        assert (0 == (p)->pos_write);
        sz_wr = sz - sz_wr;
        TD("copy second part pos=%d, buf='%s' size=%d.", (p)->pos_write, buf + (sz - sz_wr), sz_wr);
        memmove ((p)->buf1 + (p)->pos_write, buf + (sz - sz_wr), sz_wr);
        (p)->pos_write = ((p)->pos_write + sz_wr) % ((p)->sz_buf);
    }

    return sz;
}

/**
 * \brief peek data from ring buffer without advancing the inter read pointer
 * \param prb the ring buffer structure
 * \param offset the offset of the reading data from the current read position
 * \param sz the size of data
 * \param userdata the userdata pointer to be passed to callback function
 * \param cb_write the callback function pointer to be called when find the data
 * \return the size of data read to the buffer; -1 on error which need to skip to next message; 0 on reach to end of buffer which need to retry again
 */
ssize_t
rbuf_peek_cb(void *prb, size_t offset, size_t sz, void * userdata, rbuf_callback_write_t cb_write)
{
    ring_buffer_t *p = (ring_buffer_t *)prb;
    size_t sz_rd = 0;
    size_t sz_cur = 0;
    size_t virt_pos_read;
    ssize_t ret;

    assert (NULL != prb);
    if (sz < 1 || NULL == cb_write) {
       TE("input size parameter error!");
        return -1;
    }
    //assert ((p)->pos_write != (p)->pos_read);
    sz_rd = rbuf_size(prb);
    if (sz_rd < 1) {
        TE("no data available!");
        return -1;
    }
    if (offset >= sz_rd) {
        TE("offset out of range: offset=%d, size=%d.", offset, sz_rd);
        return -1;
    }
    if (sz + offset > sz_rd) {
        TD("adjust sz=%d to smaller data size=%d.", sz, sz_rd);
        sz = sz_rd - offset;
    }
    virt_pos_read = ((p)->pos_read + offset) % ((p)->sz_buf);

    assert (sz + offset <= sz_rd);
    assert (sz > 0);
    assert (sz + offset <= ((p)->sz_buf));
    assert (((p)->sz_buf) > (p)->pos_read);
    assert (((p)->sz_buf) > virt_pos_read);

    // the first part
    sz_rd = ((p)->sz_buf) - 1 - virt_pos_read;
    if (sz_rd > sz) {
        sz_rd = sz;
    }

    sz_cur = 0;
    TD("copy first part pos=%d, size=%d.", ((virt_pos_read + 1) % ((p)->sz_buf)), sz_rd);
    //memmove (buf, (p)->buf1 + ((virt_pos_read + 1) % ((p)->sz_buf)), sz_rd);
    ret = cb_write(userdata, sz, 0, (p)->buf1 + ((virt_pos_read + 1) % ((p)->sz_buf)), sz_rd);
    if (ret < 0) {
        TE("user callback return error!");
        return -1;
    }
    if (ret != sz_rd) {
        return 0;
    }

    sz_cur += sz_rd;
    // second part
    if (sz_cur < sz) {
        assert (0 == ((virt_pos_read + sz_cur + 1) % ((p)->sz_buf)));
        sz_rd = sz - sz_cur;
        TD("copy second part pos=%d, size=%d.", ((virt_pos_read + sz_cur + 1) % ((p)->sz_buf)), sz_rd);
        //memmove (buf + sz_cur, (p)->buf1 + ((virt_pos_read + sz_cur + 1) % ((p)->sz_buf)), sz_rd);
        ret = cb_write(userdata, sz, sz_cur, (p)->buf1 + ((virt_pos_read + sz_cur + 1) % ((p)->sz_buf)), sz_rd);
        if (ret < 0) {
            TE("user callback return error!");
            return -1;
        }
        if (ret != sz_rd) {
            return 0;
        }
    }
    return sz;
}

struct _rbuf_peek_cb_t {
    uint8_t * target;
    size_t sz_buf; // the total size of the 'target' buffer
    size_t sz_ret; // the data size
    size_t sz_more; // the size of buffer need to extended
};

/**
 * \brief callback for filling target buffer from ring-buffer
 * \param userdata the _rbuf_peek_cb_t structure
 * \param sz_max the total size of data
 * \param off_target the offset of the data in the buffer
 * \param buf the data from ring buffer
 * \param sz_buf the size of data
 * \return the size of data processed; -1 on error which need to skip to next message; 0 on reach to end of buffer which need to retry again
 */
static ssize_t
cb_write_peek (void * userdata, size_t sz_max, size_t off_target, uint8_t * buf, size_t sz_buf)
{
    struct _rbuf_peek_cb_t * rec = (struct _rbuf_peek_cb_t *)userdata;

    assert (off_target == rec->sz_ret);

    if (off_target + sz_buf > rec->sz_buf) {
        rec->sz_more = off_target + sz_buf - rec->sz_buf;
        TW("need more buffer from 'target'");
        return 0;
    }

    memmove (rec->target + off_target, buf, sz_buf);

    if (rec->sz_ret < off_target + sz_buf) {
        rec->sz_ret = off_target + sz_buf;
    }
    return sz_buf;
}

/**
 * \brief peek data from ring buffer without advancing the inter read pointer
 * \param prb the ring buffer structure
 * \param offset the offset of the reading data from the current read position
 * \param buf the buffer to be filled by data from ring buffer
 * \param sz the size of buffer
 * \return the size of data read to the buffer; -1 on error
 */
ssize_t
rbuf_peek(void *prb, size_t offset, uint8_t * buf, size_t sz)
{
    struct _rbuf_peek_cb_t rec;
    memset(&rec, 0, sizeof(rec));
    rec.target = buf;
    rec.sz_buf = sz;
    return rbuf_peek_cb(prb, offset, sz, &rec, cb_write_peek);
}

/**
 * \brief read data from ring buffer
 * \param prb the ring buffer structure
 * \param buf the buffer to be filled by data from ring buffer
 * \param sz the size of buffer
 * \return the size of data read to the buffer; -1 on error
 */
ssize_t
rbuf_read(void *prb, uint8_t * buf, size_t sz)
{
    ring_buffer_t *p = (ring_buffer_t *)prb;
    ssize_t ret;
    ret = rbuf_peek(prb, 0, buf, sz);
    if (ret > 0) {
        rbuf_forward(prb, ret);
    }
    return ret;
}

/**
 * \brief discard data and forword in ring buffer
 * \param prb the ring buffer structure
 * \param sz the byte size of data to be discarded in the buffer
 * \return the byte size of data to be discarded in the buffer; -1 on error
 */
ssize_t
rbuf_forward(void *prb, size_t sz)
{
    ring_buffer_t *p = (ring_buffer_t *)prb;
    if (rbuf_size(prb) < 1) {
        return 0;
    }
    if (sz > rbuf_size(prb)) {
        sz = rbuf_size(prb);
    }
    if (sz > 0) {
        (p)->pos_read = ((p)->pos_read + sz) % ((p)->sz_buf);
    }
    return sz;
}

/**
 * \brief write data to ring buffer
 * \param prb the ring buffer structure
 * \param buf the buffer to be writen
 * \param num_items the number of items in the buffer
 * \return the number of items written to the ring buffer; -1 on error
 * This function is used with MACRO version of ring buffer
 */
ssize_t
macro_rbuf_write(void *prb, void * buf, size_t num_items)
{
    size_t sz_wr = 0;

    assert (NULL != prb);
    if (num_items < 1 || NULL == buf) {
        TE("input size parameter error!");
        return -1;
    }
    sz_wr = RBUF_SPARE(prb);
    if (sz_wr < 1) {
        TE("out of space!");
        return -1;
    }
    if (num_items > sz_wr) {
        TD("adjust sz=%d to smaller spare size=%d.", num_items, sz_wr);
        num_items = sz_wr;
    }
    assert (num_items <= sz_wr);
    assert (num_items > 0);
    assert (num_items <= RBUF_MAX_ITEMS(prb));
    assert (RBUF_MAX_ITEMS(prb) > RBUF_POS_WR(prb));

    // the first part
    sz_wr = RBUF_MAX_ITEMS(prb) - RBUF_POS_WR(prb);
    if (sz_wr > num_items) {
        sz_wr = num_items;
    }
    TD("copy first part pos=%d, size=%d.", RBUF_POS_WR(prb), sz_wr);
    memmove (RBUF_ITEM_ADDR(prb, RBUF_POS_WR(prb)), buf, RBUF_ITEM_SIZE(prb) * sz_wr);
    RBUF_POS_WR(prb) = (RBUF_POS_WR(prb) + sz_wr) % RBUF_MAX_ITEMS(prb);

    // second part
    if (sz_wr < num_items) {
        assert (0 == RBUF_POS_WR(prb));
        sz_wr = num_items - sz_wr;
        TD("copy second part pos=%d, size=%d.", RBUF_POS_WR(prb), sz_wr);
        memmove (RBUF_ITEM_ADDR(prb, RBUF_POS_WR(prb))
            , (char *)buf + RBUF_ITEM_SIZE(prb) * (num_items - sz_wr)
            , RBUF_ITEM_SIZE(prb) * sz_wr);
        RBUF_POS_WR(prb) = (RBUF_POS_WR(prb) + sz_wr) % RBUF_MAX_ITEMS(prb);
    }

    return num_items;
}

/**
 * \brief peek data from ring buffer and save to buf without advancing the inter read pointer
 * \param prb the ring buffer structure
 * \param offset the offset of the reading data from the current read position
 * \param buf the buffer to be filled by data from ring buffer
 * \param num_items the number of items in the buffer
 * \return the number of items read to the buffer; -1 on error
 * This function is used with MACRO version of ring buffer
 */
ssize_t
macro_rbuf_peek(void *prb, size_t offset, void * buf, size_t num_items)
{
    size_t sz_rd = 0;
    size_t sz_cur = 0;
    size_t virt_pos_read;

    assert (NULL != prb);
    if (num_items < 1 || NULL == buf) {
       TE("input size parameter error!");
        return -1;
    }
    sz_rd = RBUF_SIZE(prb);
    if (sz_rd < 1) {
        TE("no data available!");
        return -1;
    }
    if (offset >= sz_rd) {
        TE("offset out of range: offset=%d, size=%d.", offset, sz_rd);
        return -1;
    }
    if (num_items + offset > sz_rd) {
        TD("adjust sz=%d to smaller data size=%d.", num_items, sz_rd);
        num_items = sz_rd - offset;
    }
    virt_pos_read = (RBUF_POS_RD(prb) + offset) % RBUF_MAX_ITEMS(prb);

    assert (num_items + offset <= sz_rd);
    assert (num_items > 0);
    assert (num_items + offset <= RBUF_MAX_ITEMS(prb));
    assert (RBUF_MAX_ITEMS(prb) > RBUF_POS_RD(prb));
    assert (RBUF_MAX_ITEMS(prb) > virt_pos_read);

    // the first part
    sz_rd = RBUF_MAX_ITEMS(prb) - 1 - virt_pos_read;
    if (sz_rd > num_items) {
        sz_rd = num_items;
    }

    TD("copy first part pos=%d, size=%d.", ((virt_pos_read + 1) % RBUF_MAX_ITEMS(prb)), sz_rd);
    sz_cur = 0;
    memmove (buf
        , RBUF_ITEM_ADDR(prb, ((virt_pos_read + 1) % RBUF_MAX_ITEMS(prb)))
        , RBUF_ITEM_SIZE(prb) * sz_rd);
    sz_cur += sz_rd;
    // second part
    if (sz_cur < num_items) {
        assert (0 == ((virt_pos_read + sz_cur + 1) % RBUF_MAX_ITEMS(prb)));
        sz_rd = num_items - sz_cur;
        TD("copy second part pos=%d, size=%d.", ((virt_pos_read + sz_cur + 1) % RBUF_MAX_ITEMS(prb)), sz_rd);
        memmove ((char *)buf + RBUF_ITEM_SIZE(prb) * sz_cur
            , RBUF_ITEM_ADDR(prb, ((virt_pos_read + sz_cur + 1) % RBUF_MAX_ITEMS(prb)))
            , RBUF_ITEM_SIZE(prb) * sz_rd);
    }
    return num_items;
}

/**
 * \brief read data from ring buffer and save to buf
 * \param prb the ring buffer structure
 * \param buf the buffer to be filled by data from ring buffer
 * \param num_items the number of items in the buffer
 * \return the number of items read to the buffer; -1 on error
 * This function is used with MACRO version of ring buffer
 */
ssize_t
macro_rbuf_read(void *prb, void * buf, size_t num_items)
{
    ssize_t ret;
    ret = macro_rbuf_peek(prb, 0, buf, num_items);
    if (ret > 0) {
        macro_rbuf_forward(prb, ret);
    }
    return ret;
}

/**
 * \brief discard data and forword in ring buffer
 * \param prb the ring buffer structure
 * \param num_items the number of items to be discarded in the buffer
 * \return he number of items to be discarded in the buffer; -1 on error
 */
ssize_t
macro_rbuf_forward(void *prb, size_t num_items)
{
    if (RBUF_SIZE(prb) < 1) {
        return 0;
    }
    if (num_items > RBUF_SIZE(prb)) {
        num_items = RBUF_SIZE(prb);
    }
    if (num_items > 0) {
        RBUF_POS_RD(prb) = (RBUF_POS_RD(prb) + num_items) % RBUF_MAX_ITEMS(prb);
    }
    return num_items;
}


#if defined(CIUT_ENABLED) && (CIUT_ENABLED == 1)
#include <ciut.h>

//#include "gpib-hwconf.h" // MAX_BUFFER_SEGMENT

#ifndef MAX_BUFFER_SEGMENT
#define MAX_BUFFER_SEGMENT 250
#endif

static void
rbuf_fill_test_buffer(uint8_t *buf, uint8_t start_value, size_t size)
{
    size_t i;
    for (i = 0; i < size; i ++) {
        buf[i] = start_value;
        start_value ++;
    }
}

#define MAX_SIZE (12*6)

TEST_CASE( .name="ring-buffer", .description="test ring buffer.", .skip=0 ) {
    ring_buffer_t *prb = NULL;
    uint8_t boundary[sizeof(ring_buffer_t)*2 + (MAX_SIZE + sizeof(size_t)*3 + 1) ];

    uint8_t buffer[MAX_BUFFER_SEGMENT];
    uint8_t buffer2[MAX_BUFFER_SEGMENT];
    uint8_t buffer_comp[MAX_BUFFER_SEGMENT];
    size_t sz_buf = sizeof(buffer);
    ssize_t sz_rd = -1;
    uint8_t cur_val = 0; // current start write value
    uint8_t cur_rd = 0; // current read value

    REQUIRE ((sizeof(boundary) - sizeof(ring_buffer_t)*2) / 6 * 6 <= (sizeof(boundary) - sizeof(ring_buffer_t)*2));
#define TEST_RBUF_SIZE rbuf_occupied_bytes(((MAX_SIZE + sizeof(size_t)*3 + 1) - sizeof(ring_buffer_t)) / 6 * 6)

#define MASK_BOUNDARY 'A'
    prb = (ring_buffer_t *) boundary;
    prb ++;
    assert (boundary < (uint8_t *)prb && (uint8_t *)prb <= (boundary + sizeof(ring_buffer_t)));
    assert ((uint8_t *)(prb+1) < (boundary + sizeof(boundary) - 1));

#define CHECK_BOUNDARY(b, end) \
    { \
        uint8_t *p; \
        for (p = (uint8_t *)(b); p < (uint8_t *)(end); p ++) { \
            REQUIRE(MASK_BOUNDARY == *p); \
        } \
    }

    SECTION("test ring buffer, init") {
        REQUIRE(-1 == rbuf_init(prb, 0));
        REQUIRE(-1 == rbuf_init(prb, 1));
        REQUIRE(-1 == rbuf_init(prb, sizeof(ring_buffer_t)-1));
        REQUIRE(-1 == rbuf_init(prb, sizeof(ring_buffer_t)));
        REQUIRE(0 == rbuf_init(prb, sizeof(ring_buffer_t)+1));
    }

    SECTION("test ring buffer, parameters") {
        memset(boundary, MASK_BOUNDARY, sizeof(boundary));
        rbuf_init(prb, TEST_RBUF_SIZE);
        REQUIRE(-1 == rbuf_write(prb, NULL, 0));
        REQUIRE(-1 == rbuf_write(prb, buffer, 0));
        REQUIRE(-1 == rbuf_read(prb, NULL, 0));
        REQUIRE(-1 == rbuf_read(prb, buffer, 0));
        CHECK_BOUNDARY(boundary, prb);
        CHECK_BOUNDARY((boundary + sizeof(boundary) - sizeof(ring_buffer_t)), ((char *)boundary + sizeof(boundary)));
        rbuf_reset(prb);
        CHECK_BOUNDARY(boundary, prb);
        CHECK_BOUNDARY((boundary + sizeof(boundary) - sizeof(ring_buffer_t)), ((char *)boundary + sizeof(boundary)));
    }

    SECTION("test ring buffer, basic forward") {
        int ret;
        cur_val = 0;
        cur_rd = 0;
        memset(boundary, MASK_BOUNDARY, sizeof(boundary));
        rbuf_init(prb, TEST_RBUF_SIZE);
        REQUIRE(0 == rbuf_size(prb));
        REQUIRE(rbuf_max(prb) == rbuf_spare(prb));
        REQUIRE(0 == rbuf_forward(prb, 1));
        REQUIRE(1 == rbuf_write(prb, buffer, 1));
        REQUIRE(1 == rbuf_size(prb));
        REQUIRE(1 == rbuf_forward(prb, 1));
        REQUIRE(0 == rbuf_size(prb));
        REQUIRE(1 == rbuf_write(prb, buffer, 1));
        REQUIRE(1 == rbuf_size(prb));
        REQUIRE(1 == rbuf_forward(prb, 200));
        REQUIRE(0 == rbuf_size(prb));
    }

    SECTION("test ring buffer, test forward") {
        int ret;
        cur_val = 0;
        cur_rd = 0;
        memset(boundary, MASK_BOUNDARY, sizeof(boundary));
        rbuf_init(prb, TEST_RBUF_SIZE);
        REQUIRE(0 == rbuf_size(prb));
        REQUIRE(rbuf_max(prb) == rbuf_spare(prb));

        // fullfill the ring buffer
        sz_buf = sizeof(buffer);
        CIUT_LOG("rbuf_write(sz=%d)", sz_buf);
        REQUIRE(sz_buf > 0);
        memset(buffer, 0, sizeof(buffer));
        rbuf_fill_test_buffer(buffer_comp, cur_val, sz_buf);
        sz_rd = rbuf_write(prb, buffer_comp, sz_buf);
        CIUT_LOG("rbuf_write() return %d", sz_rd);
        CIUT_LOG("rbuf_size()=%d, rbuf_spare()=%d, rbuf_max()=%d", rbuf_size(prb), rbuf_spare(prb), rbuf_max(prb));
        REQUIRE(sz_rd > 0);
        cur_val += sz_rd;
        REQUIRE(sz_rd < sz_buf);
        REQUIRE(sz_rd == rbuf_size(prb));
        REQUIRE(0 == rbuf_spare(prb));

        // read all of the ring buffer
        sz_buf = rbuf_size(prb);
        sz_rd = sz_buf/2;
        for (; sz_rd>0;) {
            rbuf_forward(prb, sz_rd);
            REQUIRE(sz_buf - sz_rd == rbuf_size(prb));
            sz_buf = rbuf_size(prb);
            sz_rd = sz_buf/2;
        }
    }
    SECTION("test ring buffer, test peek") {
        int ret;
        cur_val = 0;
        cur_rd = 0;
        memset(boundary, MASK_BOUNDARY, sizeof(boundary));
        rbuf_init(prb, TEST_RBUF_SIZE);
        REQUIRE(0 == rbuf_size(prb));
        REQUIRE(rbuf_max(prb) == rbuf_spare(prb));

        // fullfill the ring buffer
        sz_buf = sizeof(buffer);
        CIUT_LOG("rbuf_write(sz=%d)", sz_buf);
        REQUIRE(sz_buf > 0);
        memset(buffer, 0, sizeof(buffer));
        rbuf_fill_test_buffer(buffer_comp, cur_val, sz_buf);
        sz_rd = rbuf_write(prb, buffer_comp, sz_buf);
        CIUT_LOG("rbuf_write() return %d", sz_rd);
        CIUT_LOG("rbuf_size()=%d, rbuf_spare()=%d, rbuf_max()=%d", rbuf_size(prb), rbuf_spare(prb), rbuf_max(prb));
        REQUIRE(sz_rd > 0);
        cur_val += sz_rd;
        REQUIRE(sz_rd < sz_buf);
        REQUIRE(sz_rd == rbuf_size(prb));
        REQUIRE(0 == rbuf_spare(prb));

        // read all of the ring buffer
        while (rbuf_size(prb) > 0) {
            ret = rbuf_peek(prb, 0, buffer2, 1);
            ret = rbuf_read(prb, buffer, 1);        
            REQUIRE(buffer[0] == buffer2[0]);
        }
    }
    SECTION("test ring buffer, basic read write") {
        int ret;
        cur_val = 0;
        cur_rd = 0;
        memset(boundary, MASK_BOUNDARY, sizeof(boundary));
        rbuf_init(prb, TEST_RBUF_SIZE);
        REQUIRE(0 == rbuf_size(prb));
        REQUIRE(rbuf_max(prb) == rbuf_spare(prb));

        // fullfill the ring buffer
        sz_buf = sizeof(buffer);
        CIUT_LOG("rbuf_write(sz=%d)", sz_buf);
        REQUIRE(sz_buf > 0);
        memset(buffer, 0, sizeof(buffer));
        rbuf_fill_test_buffer(buffer_comp, cur_val, sz_buf);
        sz_rd = rbuf_write(prb, buffer_comp, sz_buf);
        CIUT_LOG("rbuf_write() return %d", sz_rd);
        CIUT_LOG("rbuf_size()=%d, rbuf_spare()=%d, rbuf_max()=%d", rbuf_size(prb), rbuf_spare(prb), rbuf_max(prb));
        REQUIRE(sz_rd > 0);
        cur_val += sz_rd;
        REQUIRE(sz_rd < sz_buf);
        REQUIRE(sz_rd == rbuf_size(prb));
        REQUIRE(0 == rbuf_spare(prb));

        // try to write to fulled ring buffer
        REQUIRE(-1 == rbuf_write(prb, buffer_comp, sizeof(buffer_comp)));

        // read all of the ring buffer
        ret = rbuf_peek(prb, 0, buffer2, sz_rd);
        REQUIRE(ret == rbuf_size(prb));
        REQUIRE(ret == rbuf_size(prb));
        ret = rbuf_peek(prb, sz_rd/2, buffer2+sz_rd/2, sz_rd/2);
        REQUIRE(ret == sz_rd/2);
        ret = rbuf_peek(prb, 1, buffer2+1, 1);
        REQUIRE(ret == 1);
        ret = rbuf_peek(prb, sz_rd-1, buffer2+sz_rd-1, 1);
        REQUIRE(ret == 1);

        ret = rbuf_read(prb, buffer, sz_rd);
        REQUIRE(0 == rbuf_size(prb));
        REQUIRE(rbuf_max(prb) == rbuf_spare(prb));
        CIUT_LOG("rbuf_read() return %d", sz_rd);
        CIUT_LOG("rbuf_size()=%d, rbuf_spare()=%d, rbuf_max()=%d", rbuf_size(prb), rbuf_spare(prb), rbuf_max(prb));
        CIUT_LOG("buf=%s", buffer);
        REQUIRE(sz_rd > 0);
        cur_rd += sz_rd;
        REQUIRE(sz_rd < sz_buf);
        REQUIRE(0 == memcmp(buffer, buffer_comp, sz_rd));
        REQUIRE(0 == memcmp(buffer2, buffer_comp, sz_rd));

        // try to read empty ring buffer
        REQUIRE(-1 == rbuf_read(prb, buffer, sizeof(buffer)));

        CHECK_BOUNDARY(boundary, prb);
        CHECK_BOUNDARY((boundary + sizeof(boundary) - sizeof(ring_buffer_t)), ((char *)boundary + sizeof(boundary)));
        rbuf_reset(prb);
        CHECK_BOUNDARY(boundary, prb);
        CHECK_BOUNDARY((boundary + sizeof(boundary) - sizeof(ring_buffer_t)), ((char *)boundary + sizeof(boundary)));
    }
    SECTION("test ring buffer, reset after read and write") {
        cur_val = 0;
        cur_rd = 0;
        memset(boundary, MASK_BOUNDARY, sizeof(boundary));
        rbuf_init(prb, TEST_RBUF_SIZE);
        REQUIRE(0 == rbuf_size(prb));
        CIUT_LOG("rbuf_max=%d", rbuf_max(prb));
        REQUIRE(rbuf_max(prb) % 12 == 0);
        REQUIRE(rbuf_max(prb) == rbuf_spare(prb));

        // full the ring buffer, after that size()=FULL, spare()=0
        sz_buf = rbuf_max(prb);
        CIUT_LOG("rbuf_write(sz=%d)", sz_buf);
        REQUIRE(sz_buf > 0);
        memset(buffer, 0, sizeof(buffer));
        rbuf_fill_test_buffer(buffer_comp, cur_val, sz_buf);
        sz_rd = rbuf_write(prb, buffer_comp, sz_buf);
        CIUT_LOG("rbuf_write() return %d", sz_rd);
        CIUT_LOG("rbuf_size()=%d, rbuf_spare()=%d, rbuf_max()=%d", rbuf_size(prb), rbuf_spare(prb), rbuf_max(prb));
        REQUIRE(sz_rd > 0);
        cur_val += sz_rd;
        REQUIRE(sz_rd == sz_buf);
        REQUIRE(sz_rd == rbuf_size(prb));
        REQUIRE(rbuf_max(prb) == rbuf_size(prb));
        REQUIRE(0 == rbuf_spare(prb));

        // read 5/6 of the buffer, after that size()=1/6, spare()=5/6
        sz_buf = rbuf_max(prb) * 5 / 6;
        CIUT_LOG("rbuf_read(sz=%d)", sz_buf);
        memset(buffer, 0, sizeof(buffer));
        sz_rd = rbuf_peek(prb, 0, buffer2, sz_buf);
        sz_rd = rbuf_read(prb, buffer, sz_buf);
        CIUT_LOG("rbuf_read() return %d", sz_rd);
        CIUT_LOG("rbuf_size()=%d, rbuf_spare()=%d, rbuf_max()=%d", rbuf_size(prb), rbuf_spare(prb), rbuf_max(prb));
        CIUT_LOG("buf=%s", buffer);
        REQUIRE(sz_rd > 0);
        REQUIRE((rbuf_max(prb) - sz_buf) == rbuf_size(prb));
        REQUIRE(sz_rd == rbuf_spare(prb));
        CIUT_LOG("rbuf_max=%d; rbuf_size=%d", rbuf_max(prb), rbuf_size(prb));
        REQUIRE(rbuf_max(prb)*2/12 == rbuf_size(prb));
        REQUIRE(rbuf_max(prb)*10/12 == rbuf_spare(prb));
        rbuf_fill_test_buffer(buffer_comp, cur_rd, sizeof(buffer_comp));
        REQUIRE(0 == memcmp(buffer, buffer_comp, sz_rd));
        REQUIRE(0 == memcmp(buffer2, buffer_comp, sz_rd));
        cur_rd += sz_rd;

        // reset the buffer and check boundary
        CHECK_BOUNDARY(boundary, prb);
        CHECK_BOUNDARY((boundary + sizeof(boundary) - sizeof(ring_buffer_t)), ((char *)boundary + sizeof(boundary)));
        rbuf_reset(prb);
        CHECK_BOUNDARY(boundary, prb);
        CHECK_BOUNDARY((boundary + sizeof(boundary) - sizeof(ring_buffer_t)), ((char *)boundary + sizeof(boundary)));
    }

    SECTION("test ring buffer, partial read write") {
        cur_val = 0;
        cur_rd = 0;
        memset(boundary, MASK_BOUNDARY, sizeof(boundary));
        rbuf_init(prb, TEST_RBUF_SIZE);
        REQUIRE(0 == rbuf_size(prb));
        REQUIRE(rbuf_max(prb) == rbuf_spare(prb));
        REQUIRE(rbuf_max(prb) % 12 == 0);

        // full the ring buffer, after that size()=FULL, spare()=0
        sz_buf = rbuf_max(prb);
        CIUT_LOG("rbuf_write(sz=%d)", sz_buf);
        REQUIRE(sz_buf > 0);
        memset(buffer, 0, sizeof(buffer));
        rbuf_fill_test_buffer(buffer_comp, cur_val, sz_buf);
        sz_rd = rbuf_write(prb, buffer_comp, sz_buf);
        CIUT_LOG("rbuf_write() return %d", sz_rd);
        CIUT_LOG("rbuf_size()=%d, rbuf_spare()=%d, rbuf_max()=%d", rbuf_size(prb), rbuf_spare(prb), rbuf_max(prb));
        REQUIRE(sz_rd > 0);
        cur_val += sz_rd;
        REQUIRE(sz_rd == sz_buf);
        REQUIRE(sz_rd == rbuf_size(prb));
        REQUIRE(rbuf_max(prb) == rbuf_size(prb));
        REQUIRE(0 == rbuf_spare(prb));

        // read 5/6 of the buffer, after that size()=1/6, spare()=5/6
        sz_buf = rbuf_max(prb) * 5 / 6;
        CIUT_LOG("rbuf_read(sz=%d)", sz_buf);
        memset(buffer, 0, sizeof(buffer));
        sz_rd = rbuf_peek(prb, 0, buffer2, sz_buf);
        sz_rd = rbuf_read(prb, buffer, sz_buf);
        CIUT_LOG("rbuf_read() return %d", sz_rd);
        CIUT_LOG("rbuf_size()=%d, rbuf_spare()=%d, rbuf_max()=%d", rbuf_size(prb), rbuf_spare(prb), rbuf_max(prb));
        CIUT_LOG("buf=%s", buffer);
        REQUIRE(sz_rd > 0);
        REQUIRE((rbuf_max(prb) - sz_buf) == rbuf_size(prb));
        REQUIRE(sz_rd == rbuf_spare(prb));
        REQUIRE(rbuf_max(prb)*2/12 == rbuf_size(prb));
        REQUIRE(rbuf_max(prb)*10/12 == rbuf_spare(prb));
        rbuf_fill_test_buffer(buffer_comp, cur_rd, sizeof(buffer_comp));
        REQUIRE(0 == memcmp(buffer, buffer_comp, sz_rd));
        REQUIRE(0 == memcmp(buffer2, buffer_comp, sz_rd));
        cur_rd += sz_rd;

        // write all, after that size()=6/6, spare()=0/6
        sz_buf = rbuf_max(prb)*2;
        rbuf_fill_test_buffer(buffer_comp, cur_val, sizeof(buffer_comp));
        sz_rd = rbuf_write(prb, buffer_comp, sz_buf);
        CIUT_LOG("rbuf_write() return %d", sz_rd);
        CIUT_LOG("rbuf_size()=%d, rbuf_spare()=%d, rbuf_max()=%d", rbuf_size(prb), rbuf_spare(prb), rbuf_max(prb));
        REQUIRE(sz_rd > 0);
        cur_val += sz_rd;
        REQUIRE(rbuf_max(prb)*12/12 == rbuf_size(prb));
        REQUIRE(rbuf_max(prb)*0/12 == rbuf_spare(prb));

        // read 3/6, after that size()=3/6, spare()=3/6
        sz_buf = rbuf_max(prb)*6/12;
        CIUT_LOG("rbuf_read(sz=%d)", sz_buf);
        memset(buffer, 0, sizeof(buffer));
        memset(buffer2, 0, sizeof(buffer));
        sz_rd = rbuf_peek(prb, 0, buffer2, sz_buf);
        sz_rd = rbuf_read(prb, buffer, sz_buf);
        CIUT_LOG("rbuf_read() return %d", sz_rd);
        CIUT_LOG("rbuf_size()=%d, rbuf_spare()=%d, rbuf_max()=%d", rbuf_size(prb), rbuf_spare(prb), rbuf_max(prb));
        CIUT_LOG("buf=%s", buffer);
        REQUIRE(sz_rd > 0);
        REQUIRE(sz_rd == sz_buf);
        REQUIRE(rbuf_max(prb)*6/12 == rbuf_size(prb));
        REQUIRE(rbuf_max(prb)*6/12 == rbuf_spare(prb));
        rbuf_fill_test_buffer(buffer_comp, cur_rd, sizeof(buffer_comp));
        hex_dump_to_fp(stderr, buffer_comp, sz_rd);
        hex_dump_to_fp(stderr, buffer, sz_rd);
        hex_dump_to_fp(stderr, buffer2, sz_rd);
        REQUIRE(0 == memcmp(buffer, buffer_comp, sz_rd));
        REQUIRE(0 == memcmp(buffer2, buffer_comp, sz_rd));
        cur_rd += sz_rd;

        // write another 2/6, after that size()=5/6, spare()=1/6
        sz_buf = rbuf_max(prb)*2/6;
        rbuf_fill_test_buffer(buffer_comp, cur_val, sz_buf);
        sz_rd = rbuf_write(prb, buffer_comp, sz_buf);
        CIUT_LOG("rbuf_write() return %d", sz_rd);
        CIUT_LOG("rbuf_size()=%d, rbuf_spare()=%d, rbuf_max()=%d", rbuf_size(prb), rbuf_spare(prb), rbuf_max(prb));
        REQUIRE(sz_rd > 0);
        REQUIRE(sz_rd == sz_buf);
        cur_val += sz_rd;
        REQUIRE(rbuf_max(prb)*5/6 == rbuf_size(prb));
        REQUIRE(rbuf_max(prb)*1/6 == rbuf_spare(prb));

        // read all of them, after that size()=0, spare()=FULL
        sz_buf = rbuf_max(prb) + 1;
        CIUT_LOG("rbuf_read(sz=%d)", sz_buf);
        sz_rd = rbuf_peek(prb, 0, buffer2, sz_buf);
        sz_rd = rbuf_read(prb, buffer, sz_buf);
        CIUT_LOG("rbuf_read() return %d", sz_rd);
        CIUT_LOG("rbuf_size()=%d, rbuf_spare()=%d, rbuf_max()=%d", rbuf_size(prb), rbuf_spare(prb), rbuf_max(prb));
        CIUT_LOG("buf=%s", buffer);
        REQUIRE(sz_rd > 0);
        REQUIRE(0 == rbuf_size(prb));
        REQUIRE(rbuf_max(prb) == rbuf_spare(prb));
        REQUIRE(0 == rbuf_size(prb));
        rbuf_fill_test_buffer(buffer_comp, cur_rd, sizeof(buffer_comp));
        REQUIRE(0 == memcmp(buffer, buffer_comp, sz_rd));
        REQUIRE(0 == memcmp(buffer2, buffer_comp, sz_rd));
        cur_rd += sz_rd;

        CHECK_BOUNDARY(boundary, prb);
        CHECK_BOUNDARY((boundary + sizeof(boundary) - sizeof(ring_buffer_t)), ((char *)boundary + sizeof(boundary)));
        rbuf_reset(prb);
        CHECK_BOUNDARY(boundary, prb);
        CHECK_BOUNDARY((boundary + sizeof(boundary) - sizeof(ring_buffer_t)), ((char *)boundary + sizeof(boundary)));
    }
}

//#undef MAX_SIZE
//#define MAX_SIZE 12
#define ITEM_T char
static void
rbuf_fill_test_buffer_macro(ITEM_T *buf, ITEM_T start_value, size_t size)
{
    size_t i;
    for (i = 0; i < size; i ++) {
        buf[i] = start_value;
        start_value ++;
    }
}


TEST_CASE( .name="macro-ring", .description="test MACRO ring buffer.", .skip=0 ) {

    void *prb = NULL;

    ITEM_T buffer[MAX_BUFFER_SEGMENT];
    ITEM_T buffer2[MAX_BUFFER_SEGMENT];
    ITEM_T buffer_comp[MAX_BUFFER_SEGMENT];

    ITEM_T boundary[MAX_SIZE*2];

    size_t sz_buf = sizeof(buffer) / sizeof(ITEM_T);
    ssize_t sz_rd = -1;
    ITEM_T cur_val = 0; // current start write value
    ITEM_T cur_rd = 0; // current read value

#undef MASK_BOUNDARY
#define MASK_BOUNDARY ('A')
    prb = (char *)boundary + (sizeof(boundary)/2 - (sizeof(ITEM_T) * (MAX_SIZE/2)));
    CIUT_LOG("boundary=%p; prb=%p, boundary end=%p", boundary, prb, (char *)boundary + sizeof(boundary));

    assert ((char *)boundary < (char *)prb && (char *)prb <= ((char *)boundary + sizeof(boundary)));
    assert ( (char *)(prb) + RBUF_OCCUPIED_BYTES(MAX_SIZE+1, sizeof(ITEM_T)) < ((char *)boundary + sizeof(boundary) - 1));
    memset(boundary, MASK_BOUNDARY, sizeof(boundary));
#define CHECK_BOUNDARY(b, end) \
    { \
        char *p; \
        for (p = (char *)(b); p < (char *)(end); p ++) { \
            REQUIRE(MASK_BOUNDARY == *p); \
        } \
    }

    SECTION("test ring buffer, parameters") {
        memset(boundary, MASK_BOUNDARY, sizeof(boundary));
        RBUF_INIT(prb, RBUF_OCCUPIED_BYTES(MAX_SIZE+1, sizeof(ITEM_T)), sizeof(ITEM_T));
        REQUIRE(-1 == RBUF_WRITE(prb, NULL, 0));
        REQUIRE(-1 == RBUF_WRITE(prb, buffer, 0));
        REQUIRE(-1 == RBUF_READ(prb, NULL, 0));
        REQUIRE(-1 == RBUF_READ(prb, buffer, 0));
        CHECK_BOUNDARY(boundary, prb);
        CHECK_BOUNDARY((char *)(prb) + RBUF_OCCUPIED_BYTES(MAX_SIZE+1, sizeof(ITEM_T)), ((char *)boundary + sizeof(boundary)));
        RBUF_RESET(prb);
        CHECK_BOUNDARY(boundary, prb);
        CHECK_BOUNDARY((char *)(prb) + RBUF_OCCUPIED_BYTES(MAX_SIZE+1, sizeof(ITEM_T)), ((char *)boundary + sizeof(boundary)));
    }

    SECTION("test ring buffer, basic forward") {
        int ret;
        cur_val = 0;
        cur_rd = 0;
        memset(boundary, MASK_BOUNDARY, sizeof(boundary));
        RBUF_INIT(prb, RBUF_OCCUPIED_BYTES(MAX_SIZE+1, sizeof(ITEM_T)), sizeof(ITEM_T));
        REQUIRE(0 == RBUF_SIZE(prb));
        REQUIRE(RBUF_MAX(prb) == RBUF_SPARE(prb));
        REQUIRE(0 == RBUF_FORWARD(prb, 1));
        REQUIRE(1 == RBUF_WRITE(prb, buffer, 1));
        REQUIRE(1 == RBUF_SIZE(prb));
        REQUIRE(1 == RBUF_FORWARD(prb, 1));
        REQUIRE(0 == RBUF_SIZE(prb));
        REQUIRE(1 == RBUF_WRITE(prb, buffer, 1));
        REQUIRE(1 == RBUF_SIZE(prb));
        REQUIRE(1 == RBUF_FORWARD(prb, 200));
        REQUIRE(0 == RBUF_SIZE(prb));
    }

    SECTION("test ring buffer, forward") {
        int ret;
        cur_val = 0;
        cur_rd = 0;
        memset(boundary, MASK_BOUNDARY, sizeof(boundary));
        RBUF_INIT(prb, RBUF_OCCUPIED_BYTES(MAX_SIZE+1, sizeof(ITEM_T)), sizeof(ITEM_T));
        CIUT_LOG("max number=%d, OCCUPIED_BYTES(sz=%d)", MAX_SIZE+1, RBUF_OCCUPIED_BYTES(MAX_SIZE+1, sizeof(ITEM_T)), sizeof(ITEM_T));
        CIUT_LOG("rbuf_size()=%d, rbuf_spare()=%d, rbuf_max()=%d", RBUF_SIZE(prb), RBUF_SPARE(prb), RBUF_MAX(prb));
        CIUT_LOG("RBUF_POS_RD()=%d, RBUF_POS_WR()=%d, RBUF_ITEM_SIZE()=%d, RBUF_MAX_ITEMS()=%d", RBUF_POS_RD(prb), RBUF_POS_WR(prb), RBUF_ITEM_SIZE(prb), RBUF_MAX_ITEMS(prb));
        CIUT_LOG("boundary sz=%d", sizeof(boundary));
        hex_dump_to_fp(stderr, boundary, sizeof(boundary));
        CIUT_LOG("prb sz=%d", RBUF_OCCUPIED_BYTES(MAX_SIZE+1, sizeof(ITEM_T)));
        hex_dump_to_fp(stderr, prb, RBUF_OCCUPIED_BYTES(MAX_SIZE+1, sizeof(ITEM_T)));
        REQUIRE(0 == RBUF_SIZE(prb));
        REQUIRE(RBUF_MAX(prb) == RBUF_SPARE(prb));

        // fullfill the ring buffer
        sz_buf = sizeof(buffer) / sizeof(ITEM_T);
        CIUT_LOG("rbuf_write(sz=%d)", sz_buf);
        REQUIRE(sz_buf > 0);
        memset(buffer, 0, sizeof(buffer));
        rbuf_fill_test_buffer_macro(buffer_comp, cur_val, sz_buf);
        sz_rd = RBUF_WRITE(prb, buffer_comp, sz_buf);
        CIUT_LOG("rbuf_write() return %d", sz_rd);
        CIUT_LOG("rbuf_size()=%d, rbuf_spare()=%d, rbuf_max()=%d", RBUF_SIZE(prb), RBUF_SPARE(prb), RBUF_MAX(prb));
        REQUIRE(sz_rd > 0);
        cur_val += sz_rd;
        REQUIRE(sz_rd < sz_buf);
        REQUIRE(sz_rd == RBUF_SIZE(prb));
        REQUIRE(0 == RBUF_SPARE(prb));

        sz_buf = RBUF_SIZE(prb);
        sz_rd = sz_buf/2;
        for (; sz_rd>0;) {
            RBUF_FORWARD(prb, sz_rd);
            REQUIRE(sz_buf - sz_rd == RBUF_SIZE(prb));
            sz_buf = RBUF_SIZE(prb);
            sz_rd = sz_buf/2;
        }
    }
    SECTION("test ring buffer, basic read write") {
        int ret;
        cur_val = 0;
        cur_rd = 0;
        memset(boundary, MASK_BOUNDARY, sizeof(boundary));
        RBUF_INIT(prb, RBUF_OCCUPIED_BYTES(MAX_SIZE+1, sizeof(ITEM_T)), sizeof(ITEM_T));
        CIUT_LOG("max number=%d, OCCUPIED_BYTES(sz=%d)", MAX_SIZE+1, RBUF_OCCUPIED_BYTES(MAX_SIZE+1, sizeof(ITEM_T)), sizeof(ITEM_T));
        CIUT_LOG("rbuf_size()=%d, rbuf_spare()=%d, rbuf_max()=%d", RBUF_SIZE(prb), RBUF_SPARE(prb), RBUF_MAX(prb));
        CIUT_LOG("RBUF_POS_RD()=%d, RBUF_POS_WR()=%d, RBUF_ITEM_SIZE()=%d, RBUF_MAX_ITEMS()=%d", RBUF_POS_RD(prb), RBUF_POS_WR(prb), RBUF_ITEM_SIZE(prb), RBUF_MAX_ITEMS(prb));
        CIUT_LOG("boundary sz=%d", sizeof(boundary));
        hex_dump_to_fp(stderr, boundary, sizeof(boundary));
        CIUT_LOG("prb sz=%d", RBUF_OCCUPIED_BYTES(MAX_SIZE+1, sizeof(ITEM_T)));
        hex_dump_to_fp(stderr, prb, RBUF_OCCUPIED_BYTES(MAX_SIZE+1, sizeof(ITEM_T)));
        REQUIRE(0 == RBUF_SIZE(prb));
        REQUIRE(RBUF_MAX(prb) == RBUF_SPARE(prb));

        // fullfill the ring buffer
        sz_buf = sizeof(buffer) / sizeof(ITEM_T);
        CIUT_LOG("rbuf_write(sz=%d)", sz_buf);
        REQUIRE(sz_buf > 0);
        memset(buffer, 0, sizeof(buffer));
        rbuf_fill_test_buffer_macro(buffer_comp, cur_val, sz_buf);
        sz_rd = RBUF_WRITE(prb, buffer_comp, sz_buf);
        CIUT_LOG("rbuf_write() return %d", sz_rd);
        CIUT_LOG("rbuf_size()=%d, rbuf_spare()=%d, rbuf_max()=%d", RBUF_SIZE(prb), RBUF_SPARE(prb), RBUF_MAX(prb));
        REQUIRE(sz_rd > 0);
        cur_val += sz_rd;
        REQUIRE(sz_rd < sz_buf);
        REQUIRE(sz_rd == RBUF_SIZE(prb));
        REQUIRE(0 == RBUF_SPARE(prb));

        // try to write to fulled ring buffer
        REQUIRE(-1 == RBUF_WRITE(prb, buffer_comp, sizeof(buffer_comp)/sizeof(ITEM_T)));

        // read all of the ring buffer
        ret = RBUF_PEEK(prb, 0, buffer2, sz_rd);
        ret = RBUF_PEEK(prb, sz_rd/2, buffer2+sz_rd/2, sz_rd/2);
        ret = RBUF_PEEK(prb, 1, buffer2+1, 1);
        ret = RBUF_PEEK(prb, sz_rd-1, buffer2+sz_rd-1, 1);

        ret = RBUF_READ(prb, buffer, sz_rd);
        REQUIRE(0 == RBUF_SIZE(prb));
        REQUIRE(RBUF_MAX(prb) == RBUF_SPARE(prb));
        CIUT_LOG("rbuf_read() return %d", sz_rd);
        CIUT_LOG("rbuf_size()=%d, rbuf_spare()=%d, rbuf_max()=%d", RBUF_SIZE(prb), RBUF_SPARE(prb), RBUF_MAX(prb));
        CIUT_LOG("buf=%s", buffer);
        REQUIRE(sz_rd > 0);
        cur_rd += sz_rd;
        REQUIRE(sz_rd < sz_buf);
        REQUIRE(0 == memcmp(buffer, buffer_comp, sizeof(ITEM_T) * sz_rd));
        REQUIRE(0 == memcmp(buffer2, buffer_comp, sizeof(ITEM_T) * sz_rd));

        // try to read empty ring buffer
        REQUIRE(-1 == RBUF_READ(prb, buffer, sizeof(buffer)/sizeof(ITEM_T)));

        CHECK_BOUNDARY(boundary, prb);
        CHECK_BOUNDARY((char *)(prb) + RBUF_OCCUPIED_BYTES(MAX_SIZE+1, sizeof(ITEM_T)), ((char *)boundary + sizeof(boundary)));
        RBUF_RESET(prb);
        CHECK_BOUNDARY(boundary, prb);
        CHECK_BOUNDARY((char *)(prb) + RBUF_OCCUPIED_BYTES(MAX_SIZE+1, sizeof(ITEM_T)), ((char *)boundary + sizeof(boundary)));
    }
    SECTION("test ring buffer, reset after read write") {
        cur_val = 0;
        cur_rd = 0;
        memset(boundary, MASK_BOUNDARY, sizeof(boundary));
        RBUF_INIT(prb, RBUF_OCCUPIED_BYTES(MAX_SIZE+1, sizeof(ITEM_T)), sizeof(ITEM_T));
        REQUIRE(0 == RBUF_SIZE(prb));
        REQUIRE(RBUF_MAX(prb) == RBUF_SPARE(prb));

        // full the ring buffer, after that size()=FULL, spare()=0
        sz_buf = RBUF_MAX(prb);
        CIUT_LOG("rbuf_write(sz=%d)", sz_buf);
        REQUIRE(sz_buf > 0);
        memset(buffer, 0, sizeof(buffer));
        rbuf_fill_test_buffer_macro(buffer_comp, cur_val, sz_buf);
        sz_rd = RBUF_WRITE(prb, buffer_comp, sz_buf);
        CIUT_LOG("rbuf_write() return %d", sz_rd);
        CIUT_LOG("rbuf_size()=%d, rbuf_spare()=%d, rbuf_max()=%d", RBUF_SIZE(prb), RBUF_SPARE(prb), RBUF_MAX(prb));
        REQUIRE(sz_rd > 0);
        cur_val += sz_rd;
        REQUIRE(sz_rd == sz_buf);
        REQUIRE(sz_rd == RBUF_SIZE(prb));
        REQUIRE(RBUF_MAX(prb) == RBUF_SIZE(prb));
        REQUIRE(0 == RBUF_SPARE(prb));

        // read 5/6 of the buffer, after that size()=1/6, spare()=5/6
        sz_buf = RBUF_MAX(prb) * 5 / 6;
        CIUT_LOG("rbuf_read(sz=%d)", sz_buf);
        memset(buffer, 0, sizeof(buffer));
        memset(buffer2, 0, sizeof(buffer));
        sz_rd = RBUF_PEEK(prb, 0, buffer2, sz_buf);
        sz_rd = RBUF_READ(prb, buffer, sz_buf);
        CIUT_LOG("rbuf_read() return %d", sz_rd);
        CIUT_LOG("rbuf_size()=%d, rbuf_spare()=%d, rbuf_max()=%d", RBUF_SIZE(prb), RBUF_SPARE(prb), RBUF_MAX(prb));
        CIUT_LOG("buf=%s", buffer);
        REQUIRE(sz_rd > 0);
        REQUIRE((RBUF_MAX(prb) - sz_buf) == RBUF_SIZE(prb));
        REQUIRE(sz_rd == RBUF_SPARE(prb));
        REQUIRE(RBUF_MAX(prb)*2/12 == RBUF_SIZE(prb));
        REQUIRE(RBUF_MAX(prb)*10/12 == RBUF_SPARE(prb));
        rbuf_fill_test_buffer_macro(buffer_comp, cur_rd, sizeof(buffer_comp)/sizeof(ITEM_T));
        REQUIRE(0 == memcmp(buffer, buffer_comp, sizeof(ITEM_T) * sz_rd));
        REQUIRE(0 == memcmp(buffer2, buffer_comp, sizeof(ITEM_T) * sz_rd));
        cur_rd += sz_rd;

        // reset the buffer and check boundary
        CHECK_BOUNDARY(boundary, prb);
        CHECK_BOUNDARY((char *)(prb) + RBUF_OCCUPIED_BYTES(MAX_SIZE+1, sizeof(ITEM_T)), ((char *)boundary + sizeof(boundary)));
        CIUT_LOG("boundary sz=%d", sizeof(boundary));
        hex_dump_to_fp(stderr, boundary, sizeof(boundary));
        CIUT_LOG("prb sz=%d", RBUF_OCCUPIED_BYTES(MAX_SIZE+1, sizeof(ITEM_T)));
        hex_dump_to_fp(stderr, prb, RBUF_OCCUPIED_BYTES(MAX_SIZE+1, sizeof(ITEM_T)));

        RBUF_RESET(prb);
        CHECK_BOUNDARY(boundary, prb);
        CHECK_BOUNDARY((char *)(prb) + RBUF_OCCUPIED_BYTES(MAX_SIZE+1, sizeof(ITEM_T)), ((char *)boundary + sizeof(boundary)));
    }
    SECTION("test ring buffer, partial read write") {
        cur_val = 0;
        cur_rd = 0;
        memset(boundary, MASK_BOUNDARY, sizeof(boundary));
        RBUF_INIT(prb, RBUF_OCCUPIED_BYTES(MAX_SIZE+1, sizeof(ITEM_T)), sizeof(ITEM_T));
        REQUIRE(0 == RBUF_SIZE(prb));
        REQUIRE(RBUF_MAX(prb) == RBUF_SPARE(prb));

        // full the ring buffer, after that size()=FULL, spare()=0
        sz_buf = RBUF_MAX(prb);
        CIUT_LOG("rbuf_write(sz=%d)", sz_buf);
        REQUIRE(sz_buf > 0);
        memset(buffer, 0, sizeof(buffer));
        rbuf_fill_test_buffer_macro(buffer_comp, cur_val, sz_buf);
        sz_rd = RBUF_WRITE(prb, buffer_comp, sz_buf);
        CIUT_LOG("rbuf_write() return %d", sz_rd);
        CIUT_LOG("rbuf_size()=%d, rbuf_spare()=%d, rbuf_max()=%d", RBUF_SIZE(prb), RBUF_SPARE(prb), RBUF_MAX(prb));
        REQUIRE(sz_rd > 0);
        cur_val += sz_rd;
        REQUIRE(sz_rd == sz_buf);
        REQUIRE(sz_rd == RBUF_SIZE(prb));
        REQUIRE(RBUF_MAX(prb) == RBUF_SIZE(prb));
        REQUIRE(0 == RBUF_SPARE(prb));

        // read 5/6 of the buffer, after that size()=1/6, spare()=5/6
        sz_buf = RBUF_MAX(prb) * 5 / 6;
        CIUT_LOG("rbuf_read(sz=%d)", sz_buf);
        memset(buffer, 0, sizeof(buffer));
        memset(buffer2, 0, sizeof(buffer));
        sz_rd = RBUF_PEEK(prb, 0, buffer2, sz_buf);
        sz_rd = RBUF_READ(prb, buffer, sz_buf);
        CIUT_LOG("rbuf_read() return %d", sz_rd);
        CIUT_LOG("rbuf_size()=%d, rbuf_spare()=%d, rbuf_max()=%d", RBUF_SIZE(prb), RBUF_SPARE(prb), RBUF_MAX(prb));
        CIUT_LOG("buf=%s", buffer);
        REQUIRE(sz_rd > 0);
        REQUIRE((RBUF_MAX(prb) - sz_buf) == RBUF_SIZE(prb));
        REQUIRE(sz_rd == RBUF_SPARE(prb));
        REQUIRE(RBUF_MAX(prb)*2/12 == RBUF_SIZE(prb));
        REQUIRE(RBUF_MAX(prb)*10/12 == RBUF_SPARE(prb));
        rbuf_fill_test_buffer_macro(buffer_comp, cur_rd, sizeof(buffer_comp)/sizeof(ITEM_T));
        REQUIRE(0 == memcmp(buffer, buffer_comp, sizeof(ITEM_T) * sz_rd));
        REQUIRE(0 == memcmp(buffer2, buffer_comp, sizeof(ITEM_T) * sz_rd));
        cur_rd += sz_rd;

        // write all, after that size()=6/6, spare()=0/6
        sz_buf = RBUF_MAX(prb)*2;
        rbuf_fill_test_buffer_macro(buffer_comp, cur_val, sizeof(buffer_comp)/sizeof(ITEM_T));
        sz_rd = RBUF_WRITE(prb, buffer_comp, sz_buf);
        CIUT_LOG("rbuf_write() return %d", sz_rd);
        CIUT_LOG("rbuf_size()=%d, rbuf_spare()=%d, rbuf_max()=%d", RBUF_SIZE(prb), RBUF_SPARE(prb), RBUF_MAX(prb));
        REQUIRE(sz_rd > 0);
        cur_val += sz_rd;
        REQUIRE(RBUF_MAX(prb)*12/12 == RBUF_SIZE(prb));
        REQUIRE(RBUF_MAX(prb)*0/12 == RBUF_SPARE(prb));

        // read 3/6, after that size()=3/6, spare()=3/6
        sz_buf = RBUF_MAX(prb)*6/12;
        CIUT_LOG("rbuf_read(sz=%d)", sz_buf);
        memset(buffer, 0, sizeof(buffer));
        memset(buffer2, 0, sizeof(buffer));
        sz_rd = RBUF_PEEK(prb, 0, buffer2, sz_buf);
        sz_rd = RBUF_READ(prb, buffer, sz_buf);
        CIUT_LOG("rbuf_read() return %d", sz_rd);
        CIUT_LOG("rbuf_size()=%d, rbuf_spare()=%d, rbuf_max()=%d", RBUF_SIZE(prb), RBUF_SPARE(prb), RBUF_MAX(prb));
        CIUT_LOG("buf=%s", buffer);
        REQUIRE(sz_rd > 0);
        REQUIRE(sz_rd == sz_buf);
        REQUIRE(RBUF_MAX(prb)*6/12 == RBUF_SIZE(prb));
        REQUIRE(RBUF_MAX(prb)*6/12 == RBUF_SPARE(prb));
        rbuf_fill_test_buffer_macro(buffer_comp, cur_rd, sizeof(buffer_comp)/sizeof(ITEM_T));
        REQUIRE(0 == memcmp(buffer, buffer_comp, sizeof(ITEM_T) * sz_rd));
        REQUIRE(0 == memcmp(buffer2, buffer_comp, sizeof(ITEM_T) * sz_rd));
        cur_rd += sz_rd;

        // write another 2/6, after that size()=5/6, spare()=1/6
        sz_buf = RBUF_MAX(prb)*2/6;
        rbuf_fill_test_buffer_macro(buffer_comp, cur_val, sz_buf);
        sz_rd = RBUF_WRITE(prb, buffer_comp, sz_buf);
        CIUT_LOG("rbuf_write() return %d", sz_rd);
        CIUT_LOG("rbuf_size()=%d, rbuf_spare()=%d, rbuf_max()=%d", RBUF_SIZE(prb), RBUF_SPARE(prb), RBUF_MAX(prb));
        REQUIRE(sz_rd > 0);
        REQUIRE(sz_rd == sz_buf);
        cur_val += sz_rd;
        REQUIRE(RBUF_MAX(prb)*5/6 == RBUF_SIZE(prb));
        REQUIRE(RBUF_MAX(prb)*1/6 == RBUF_SPARE(prb));

        // read all of them, after that size()=0, spare()=FULL
        sz_buf = RBUF_MAX(prb) + 1;
        CIUT_LOG("rbuf_read(sz=%d)", sz_buf);
        sz_rd = RBUF_PEEK(prb, 0, buffer2, sz_buf);
        sz_rd = RBUF_READ(prb, buffer, sz_buf);
        CIUT_LOG("rbuf_read() return %d", sz_rd);
        CIUT_LOG("rbuf_size()=%d, rbuf_spare()=%d, rbuf_max()=%d", RBUF_SIZE(prb), RBUF_SPARE(prb), RBUF_MAX(prb));
        CIUT_LOG("buf=%s", buffer);
        REQUIRE(sz_rd > 0);
        REQUIRE(0 == RBUF_SIZE(prb));
        REQUIRE(RBUF_MAX(prb) == RBUF_SPARE(prb));
        REQUIRE(0 == RBUF_SIZE(prb));
        rbuf_fill_test_buffer_macro(buffer_comp, cur_rd, sizeof(buffer_comp)/sizeof(ITEM_T));
        REQUIRE(0 == memcmp(buffer, buffer_comp, sizeof(ITEM_T) * sz_rd));
        REQUIRE(0 == memcmp(buffer2, buffer_comp, sizeof(ITEM_T) * sz_rd));
        cur_rd += sz_rd;

        CHECK_BOUNDARY(boundary, prb);
        CHECK_BOUNDARY((char *)(prb) + RBUF_OCCUPIED_BYTES(MAX_SIZE+1, sizeof(ITEM_T)), ((char *)boundary + sizeof(boundary)));
        CIUT_LOG("boundary sz=%d", sizeof(boundary));
        hex_dump_to_fp(stderr, boundary, sizeof(boundary));
        CIUT_LOG("prb sz=%d", RBUF_OCCUPIED_BYTES(MAX_SIZE+1, sizeof(ITEM_T)));
        hex_dump_to_fp(stderr, prb, RBUF_OCCUPIED_BYTES(MAX_SIZE+1, sizeof(ITEM_T)));

        RBUF_RESET(prb);
        CHECK_BOUNDARY(boundary, prb);
        CHECK_BOUNDARY((char *)(prb) + RBUF_OCCUPIED_BYTES(MAX_SIZE+1, sizeof(ITEM_T)), ((char *)boundary + sizeof(boundary)));
    }
}

#endif /* CIUT_ENABLED */


