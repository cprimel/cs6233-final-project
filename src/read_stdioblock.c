#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


const char func_type = 'r';
const char *func_name = "stdioblock";
const char *func_desc = "Read from file using standard C library, variable block size.";
const size_t min_block = 64;

unsigned int xorbuf(unsigned int *buffer, int size) {
    unsigned int result = 0;
    for (int i = 0; i < size; ++i) {
        result ^= buffer[i];
    }
    return result;
}

unsigned int read_from_file(int fd, size_t size, size_t block_size, char* buf) {
    unsigned int xor_res = 0;

    FILE *file = fdopen(fd, "r");
    size_t n = 0;
    while (n < size) {
        ssize_t r = fread(buf,1, block_size,file);
        xor_res ^= xorbuf((unsigned int *) buf, r / 4);
        if (r != (ssize_t) block_size) {
            break;
        }
        n += r;
    }
    return xor_res;
}

void write_to_file(int fd, size_t size, size_t block_size, char* buf) {};