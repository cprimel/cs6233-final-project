#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

unsigned int xorbuf(const unsigned int *buffer, size_t size);

unsigned int read_from_file(int fd, size_t block_size, char* buf) {
    unsigned int xor_res = 0;

    ssize_t r;
    while ((r = read(fd, buf, block_size))) {

        xor_res ^= xorbuf((unsigned int*)buf, r / 4);
        if (r != (ssize_t) block_size) {
            break;
        }
    }
    return xor_res;
}

unsigned int xorbuf(const unsigned int *buffer, size_t size) {
    unsigned int result = 0;
    for (int i = 0; i < size; ++i) {
        result ^= buffer[i];
    }
    return result;
}

int main(int argc, char *argv[]) {

    char * filename = argv[1];

    int fd = open(filename, O_RDONLY);
    size_t block_size = 131072;
    char buf[block_size];
    unsigned int xor_res = read_from_file(fd, block_size, buf);
    printf("XOR: %u", xor_res);
    close(fd);

    return 0;

}