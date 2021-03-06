#include <stdlib.h>
#include <unistd.h>

const char func_type = 'w';
const char *func_name = "osblock";
const char *func_desc = "Write to file using OS system call, variable block size.";

void write_to_file(int fd, size_t size, size_t block_size, char* buf) {

    size_t n = 0;
    while (n < size) {
        size_t nw;
        if (block_size < size - n) {
            nw = block_size;
        } else {
            nw = size - n;
        }
        ssize_t r = write(fd, buf, nw);
        n += r;
        buf += r;
    }
}

size_t read_from_file(int fd, size_t size, size_t block_size, char* buf) { return -1;}