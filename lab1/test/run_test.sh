#!/bin/bash

MAIN_DIR=$PWD/..
#gcc $MAIN_DIR/test/test_circular_buffer.c $MAIN_DIR/src/circularBuffer.c -o test > /dev/null
#./test
#rm test


gcc $MAIN_DIR/test/test_char_driver.c -o test_char > /dev/null
./test_char
rm test_char
sleep 1
gcc $MAIN_DIR/test/test_handle_1.c -o test_char > /dev/null
./test_char
rm test_char
