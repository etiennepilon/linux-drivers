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
    unsigned int cursor_in;
    unsigned int cursor_out;
    unsigned int max_capacity;
    char is_full;
};

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
    kfree(cbuf->buffer);
    kfree(cbuf);
#else
    free(cbuf->buffer);
    free(cbuf);
#endif
}

void cbuf_clear(cbuf_handle_t cbuf){
    if (cbuf == NULL) return;
    cbuf->cursor_in = 0;
    cbuf->cursor_out = 0;
    cbuf->is_full = 0;
}
char cbuf_is_full(cbuf_handle_t cbuf){
    if (cbuf == NULL) return 0;
    return cbuf->is_full;
}
char cbuf_is_empty(cbuf_handle_t cbuf){
    if (cbuf == NULL) return 0;
    return (!cbuf->is_full && cbuf->cursor_in == cbuf->cursor_out);
}
unsigned int cbuf_current_size(cbuf_handle_t cbuf){
    if (cbuf == NULL) return 0;
    if (cbuf == NULL) return 0;
    unsigned int current_size = 0;
    if (cbuf->is_full) {
        current_size = cbuf->max_capacity;
    } else if (cbuf->cursor_in >= cbuf->cursor_out) {
        current_size = cbuf->cursor_in - cbuf->cursor_out; 
    } else {
        current_size = cbuf->max_capacity + cbuf->cursor_in - cbuf->cursor_out;
    }
    return current_size;
}
unsigned int cbuf_max_capacity(cbuf_handle_t cbuf){
    if (cbuf == NULL) return 0;
    return cbuf->max_capacity;
}

static void move_cursor_in(cbuf_handle_t cbuf){
    cbuf->cursor_in = (cbuf->cursor_in + 1) % cbuf->max_capacity;
    cbuf->is_full = cbuf->cursor_out == cbuf->cursor_in ? 1 : 0;
    return;
}

char cbuf_put(cbuf_handle_t cbuf, char byte){
    if (cbuf == NULL || cbuf->is_full) return 0;
    cbuf->buffer[cbuf->cursor_in] = byte;
    move_cursor_in(cbuf);
    return 1;
}

static void move_cursor_out(cbuf_handle_t cbuf, unsigned int size){
    if (cbuf_is_empty(cbuf)) return;
    cbuf->cursor_out = (cbuf->cursor_out + size) % cbuf->max_capacity;
    cbuf->is_full = 0;
    return;
}

char cbuf_pop(cbuf_handle_t cbuf, char* buffer){
   if (cbuf == NULL || cbuf_is_empty(cbuf)) return 0;
   *buffer = cbuf->buffer[cbuf->cursor_out];
   move_cursor_out(cbuf, 1);
   return 1;
}

