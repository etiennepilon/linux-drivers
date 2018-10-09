/*
 * =====================================================================================
 *
 *       Filename:  circularBuffer.c
 *
 *    Description: 
 *
 *        Version:  1.0
 *        Created:  04/10/2018 14:50:27
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include "circularBuffer.h"

struct circ_buf_t{
    char* buffer;
    unsigned int cursor_head;
    unsigned int cursor_tail;
    unsigned int max_capacity;
    char is_full;
}
// ---------------------
// -------- API --------
// ---------------------
cbuf_handle_t cbuf_init(char* buf, unsigned int size){
    if (buf == NULL || size == 0) return NULL;
    cbuf_handle_t cbuf = NULL;
#ifdef __KERNEL__
    cbuf = (cbuf_handle_t) kmalloc(sizeof(circ_buf_t), GFP_KERNEL);
    if (cbuf == NULL) {
        kfree(cbuf);
        return NULL;
    }
#else
    cbuf = (cbuf_handle_t) malloc(sizeof(circ_buf_t));
    if (cbuf == NULL) {
        free(cbuf);
        return NULL;
    }
#endif
    cbuf->max_capacity = size;
    cbuf->buffer = buf;
    cbuf_clear(cbuf);
    return cbuf;
}

void cbuf_free(cbuf_handle_t cbuf){
#ifdef __KERNEL__
    kfree(cbuf);
#else
    free(cbuf);
#endif
}

void cbuf_clear(cbuf_handle_t cbuf){
    if (cbuf == NULL) return;
    cbuf->cursor_head = 0;
    cbuf->cursor_tail = 0;
    cbuf->is_full = 0;
}
/*  
// cbuf_put
// return: 0 if full - 1 if not
char cbuf_put(cbuf_handle_t cbuf, char byte);
// cbuf_pop
// return: byte count
char cbuf_pop(cbuf_handle_t cbuf, char* bytes);
char cbuf_is_empty(cbuf_handle_t cbuf);
char cbuf_is_full(cbuf_handle_t cbuf);

unsigned int cbuf_current_size(cbuf_handle_t cbuf);
unsigned int cbuf_max_capacity(cbuf_handle_t cbuf);
*/
