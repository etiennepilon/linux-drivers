#ifndef CIRC_BUFFER_H
#define CIRC_BUFFER_H
#ifdef __KERNEL__
    #include<linux/slab.h>
#endif
#ifndef NULL
    #define NULL ((void*) 0)
#endif

typedef struct circ_buf_t circ_buf_t;
typedef circ_buf_t* cbuf_handle_t;

// -- API Calls --
cbuf_handle_t cbuf_init(char* buf, unsigned int size);
void cbuf_free(cbuf_handle_t cbuf);
void cbuf_clear(cbuf_handle_t cbuf);
// cbuf_put
// return: 0 if full - 1 if not
char cbuf_put(cbuf_handle_t cbuf, char byte);
//char cbuf_put_block(cbuf_handle_t cbuf, char* bytes, unsigned int block_size);
// cbuf_pop
// return: byte count
char cbuf_pop(cbuf_handle_t cbuf, char* bytes);
//char cbuf_pop_block(cbuf_handle_t cbuf, char* bytes, unsigned int block_size);
char cbuf_is_empty(cbuf_handle_t cbuf);
char cbuf_is_full(cbuf_handle_t cbuf);

unsigned int cbuf_current_size(cbuf_handle_t cbuf);
unsigned int cbuf_max_capacity(cbuf_handle_t cbuf);

#endif
