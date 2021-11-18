#include <stdlib.h>
#include <unistd.h>

const char func_type = 'r';
const char *func_desc = "Read from file using OS system call, variable block size.";



size_t read_from_file(int fd, size_t size, size_t block_size, char* buf) {

    size_t n = 0;
    while (n < size) {
        ssize_t r = read(fd, buf, block_size);
        if (r != (ssize_t) block_size) {
            break;
        }
        n += r;
        buf += r;
    }
    return n;
}

void write_to_file(int fd, size_t size, size_t block_size, char* buf) {};