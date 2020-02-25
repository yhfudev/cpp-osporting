/**
 * \file    ringbuffer.h
 * \brief   Ring buffer for write/read
 * \author  Yunhui Fu <yhfudev@gmail.com>
 * \version 1.0
 */

#ifndef _RING_BUFFER_H
#define _RING_BUFFER_H 1

#include "osporting.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


////////////////////////////////////////////////////////////////////////////////
// Function version of ring buffer: supports only byte

// we use ring buffers to sync the data between
// the sender and receiver, to avoid expose API

typedef struct _ring_buffer_t {
    size_t volatile pos_read;  // the read position
    size_t volatile pos_write; // the write position
    size_t sz_buf;  // byte size of buffer

    unsigned char * buf1; // the buffer
} ring_buffer_t;

#define rbuf_occupied_bytes(data_size) (sizeof(ring_buffer_t) + 1 + (data_size))


/**
 * init a ring buffer structure
 * \param prb the pointer to the start of a memory buffer for ring buffer structure
 * \param byte_size the byte size of the whole buffer
 * \return 0 on success; -1 on error
 */
int rbuf_init(void *prb, size_t byte_size);
//#define rbuf_init(prb, byte_size) (((byte_size) <= sizeof(size_t)*3)?(memset((prb), 0, sizeof(ring_buffer_t)), ((prb)->pos_write = 1), ((prb)->sz_buf = (byte_size) - sizeof(size_t)*3), 0):-1)

/**
 * \brief get the capacity of the ring buffer
 * \param prb the ring buffer structure
 * \return the capacity of the ring buffer
 */
#define rbuf_max(prb) (((ring_buffer_t *)(prb))->sz_buf - 1)

//size_t rbuf_size(ring_buffer_t *prb);
/**
 * \brief get data size in ring buffer
 * \param prb the ring buffer structure
 * \return the data size in ring buffer
 */
#define rbuf_size(prb) ((((ring_buffer_t *)(prb))->pos_write + (((ring_buffer_t *)(prb))->sz_buf) - ((ring_buffer_t *)(prb))->pos_read - 1) % (((ring_buffer_t *)(prb))->sz_buf))

// return the spare size of buffer
//size_t rbuf_spare(ring_buffer_t *prb);
#define rbuf_spare(prb) (rbuf_max(prb) - rbuf_size(prb))

/**
 * \brief callback for filling target buffer from ring-buffer
 * \param userdata the pointer of user defined structure
 * \param sz_max the total size of data
 * \param off_target the offset of the data in the buffer
 * \param buf the data from ring buffer
 * \param sz_buf the size of data
 * \return the size of data processed; -1 on error which need to skip to next message; 0 on reach to end of buffer which need to retry again
 */
typedef ssize_t (* rbuf_callback_write_t) (void * userdata, size_t sz_max, size_t off_target, uint8_t * buf, size_t sz_buf);
ssize_t rbuf_peek_cb(void *prb, size_t offset, size_t sz, void * userdata, rbuf_callback_write_t cb_write);

ssize_t rbuf_peek(void *prb, size_t offset, uint8_t * buf, size_t sz);
ssize_t rbuf_read(void *prb, uint8_t * buf, size_t sz);
ssize_t rbuf_write(void *prb, uint8_t * buf, size_t sz);
ssize_t rbuf_forward(void *prb, size_t sz);


#define rbuf_reset(prb) rbuf_init(((ring_buffer_t *)(prb)), ((ring_buffer_t *)(prb))->sz_buf + sizeof(size_t)*3)


////////////////////////////////////////////////////////////////////////////////
// Macro version of ring buffer: supports user specified length of items

/// get the read position index value
#define RBUF_POS_RD(prb)    *((int *)(prb) + 0)
/// get the write position index value
#define RBUF_POS_WR(prb)    *((int *)(prb) + 1)
/// get the byte size of one item
#define RBUF_ITEM_SIZE(prb) *((int *)(prb) + 2)
/// get the max number of items can be stored, including the one used for barrier rd/wr
#define RBUF_MAX_ITEMS(prb) *((int *)(prb) + 3)
/// get the address of item index x
#define RBUF_ITEM_ADDR(prb, x) (((char *)((int *)(prb) + 4)) + RBUF_ITEM_SIZE(prb) * (x))

/// calculate the occupied byte size space for a giving ring buffer, including the header and data space
/// the items_in_buf includes the one used for barrier rd/wr
#define RBUF_OCCUPIED_BYTES(items_in_buf, item_size) (sizeof(int) * 4 + (item_size) * (items_in_buf))

/// calculate the max available item slots for a given byte length of buffer, including the header and data space.
/// the DATA_NUM includes the one used for barrier rd/wr
#define RBUF_CALC_DATA_NUM(byte_size, item_size) (((byte_size) - sizeof(int)*4) / (item_size))


/**
 * init a ring buffer structure
 * \param prb the ring buffer structure
 * \param byte_size_prb the byte size of the whole buffer
 * \param item_size the size of each item in the buffer
 */
#define RBUF_INIT(prb, byte_size_prb, item_size) \
    RBUF_POS_RD(prb) = 0; \
    RBUF_POS_WR(prb) = 1; \
    RBUF_ITEM_SIZE(prb) = (item_size); \
    RBUF_MAX_ITEMS(prb) = RBUF_CALC_DATA_NUM((byte_size_prb), (item_size))

ssize_t macro_rbuf_peek(void *prb, size_t offset, void * buf, size_t num_items);
ssize_t macro_rbuf_read(void *prb, void * buf, size_t num_items);
ssize_t macro_rbuf_write(void *prb, void * buf, size_t num_items);
ssize_t macro_rbuf_forward(void *prb, size_t num_items);

/**
 * \brief peek data from ring buffer and save to buf without advancing the inter read pointer
 * \param prb the ring buffer structure
 * \param offset the offset of the reading data from the current read position
 * \param buf the buffer to be filled by data from ring buffer
 * \param items_in_buf the number of items in the buffer
 * \return the number of items read to the buffer; -1 on error
 */
#define RBUF_PEEK(prb, offset, buf, items_in_buf)  macro_rbuf_peek((prb), (offset), (buf), (items_in_buf))

/**
 * \brief read data from ring buffer and save to buf
 * \param prb the ring buffer structure
 * \param buf the buffer to be filled by data from ring buffer
 * \param items_in_buf the number of items in the buffer
 * \return the number of items read to the buffer; -1 on error
 */
#define RBUF_READ(prb, buf, items_in_buf)  macro_rbuf_read((prb), (buf), (items_in_buf))

/**
 * \brief discard data and forword in ring buffer
 * \param prb the ring buffer structure
 * \param num_items the number of items to be discarded in the buffer
 * \return he number of items to be discarded in the buffer; -1 on error
 */
#define RBUF_FORWARD(prb, num_items)  macro_rbuf_forward((prb), (num_items))

/**
 * \brief write data to ring buffer
 * \param prb the ring buffer structure
 * \param buf the buffer to be writen
 * \param num_items the number of items in the buffer
 * \return the number of items written to the ring buffer; -1 on error
 */
#define RBUF_WRITE(prb, buf, items_in_buf) macro_rbuf_write((prb), (buf), (items_in_buf))

/**
 * \brief get number of items in ring buffer
 * \param prb the ring buffer structure
 * \return the number of items in ring buffer
 */
#define RBUF_SIZE(prb) ((RBUF_POS_WR(prb) + RBUF_MAX_ITEMS(prb) - RBUF_POS_RD(prb) - 1) % RBUF_MAX_ITEMS(prb))

/**
 * \brief get the number of spare item slots in ring buffer
 * \param prb the ring buffer structure
 * \return the number of spare item slots in ring buffer
 */
#define RBUF_SPARE(prb) (RBUF_MAX(prb) - RBUF_SIZE(prb))

/**
 * \brief get the max number of item slots in ring buffer
 * \param prb the ring buffer structure
 * \return the max number of item slots in ring buffer
 */
#define RBUF_MAX(prb) (RBUF_MAX_ITEMS(prb) - 1)

/**
 * \brief reset the ring buffer
 * \param prb the ring buffer structure
 */
#define RBUF_RESET(prb) RBUF_INIT((prb), RBUF_OCCUPIED_BYTES(RBUF_MAX_ITEMS(prb), RBUF_ITEM_SIZE(prb)), RBUF_ITEM_SIZE(prb))


#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* _RING_BUFFER_H */

