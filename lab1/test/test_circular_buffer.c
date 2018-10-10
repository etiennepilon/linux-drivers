#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../src/circularBuffer.h"

#define BUF_SIZE 8

int main(void)
{

    char* buf = malloc(BUF_SIZE * sizeof(char));
    char test_value = 8, buffer = 0;
    cbuf_handle_t cbuf = cbuf_init(buf, BUF_SIZE);
    // -- Test proper init --
    assert(cbuf);
    // -- Test helper functions --
    assert(cbuf_is_empty(cbuf));
    assert(!cbuf_is_full(cbuf));
    assert(cbuf_current_size(cbuf) == 0);
    assert(cbuf_max_capacity(cbuf) == BUF_SIZE);
    // -- Test circular functions --
    // -- Should put 1 value, and remove it properly
    assert(cbuf_put(cbuf, test_value));
    assert(cbuf_current_size(cbuf) == 1);
    assert(cbuf_pop(cbuf, &buffer));
    assert(buffer == test_value);
    assert(cbuf_current_size(cbuf) == 0);
    assert(cbuf_is_empty(cbuf));
    // -- Should be full after BUF_SIZE --
    for(int i = 0; i < BUF_SIZE; i++) {
        assert(cbuf_put(cbuf, test_value));
        assert(cbuf_current_size(cbuf) == i + 1);
    }
    assert(!cbuf_put(cbuf, test_value));
    assert(cbuf_is_full(cbuf));
    // -- Should clear space for new bytes --
    assert(cbuf_pop(cbuf, &buffer));
    assert(!cbuf_is_full(cbuf));
    assert(test_value == buffer);
    assert(cbuf_put(cbuf, test_value));
    assert(cbuf_current_size(cbuf) == BUF_SIZE);
    // -- Should empty the buffer fully --
    for(unsigned int i = 0; i < BUF_SIZE; i++) {
        assert(cbuf_current_size(cbuf) == BUF_SIZE - i);
        assert(cbuf_pop(cbuf, &buffer));
        assert(cbuf_current_size(cbuf) == BUF_SIZE - i - 1);
    }
    assert(cbuf_is_empty(cbuf)); 
    // -- Frees memory --
    cbuf_free(cbuf);
    free(buf);
}
