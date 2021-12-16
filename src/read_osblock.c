#include <stdlib.h>
#include <unistd.h>

const char func_type = 'r';
const char *func_name = "osblock";
const char *func_desc = "Read from file using OS system call, variable block size.";
const size_t min_block =64;


unsigned int xorbuf(const unsigned int *buffer, int size) {
    unsigned int result = 0;
    for (int i = 0; i < size; ++i) {
        unsigned int val = buffer[i];
        result ^= val;
    }
    return result;
}

unsigned int read_from_file(int fd, size_t size, size_t block_size, char* buf) {
    unsigned int xor_res = 0;

    size_t n = 0;
    while (n < size) {
        ssize_t r = read(fd, buf, block_size);

        xor_res ^= xorbuf((unsigned int *) buf, r / 4);
        if (r != (ssize_t) block_size) {
            break;
        }
        n += r;
    }
    return xor_res;
}

void write_to_file(int fd, size_t size, size_t block_size, char* buf) {};