#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../src/circularBuffer.h"

#define BUF_SIZE 10

int main(void)
{

    char* buf = malloc(BUF_SIZE * sizeof(char));
    cbuf_handle_t cbuf = cbuf_init(buf, BUF_SIZE);
    // -- Test proper init --
    assert(cbuf);

    // -- Frees memory --
    cbuf_free(cbuf);
    free(buf);
}
