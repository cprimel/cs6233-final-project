#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

const char func_type = 'w';
const char *func_desc = "Write to file using C standard library, variable block size.";

void write_to_file(int fd, size_t size, size_t block_size, char* buf) {
    FILE *file = fdopen(fd, "w");

    size_t n = 0;
    while (n < size) {
        size_t nw;
        if (block_size < size - n) {
            nw = block_size;
        } else {
            nw = size - n;
        }
        size_t r = fwrite(buf, 1, nw, file);
        n += r;
        buf += r;
    }
}

size_t read_from_file(int fd, size_t size, size_t block_size, char* buf) { return -1;}