#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>

const char *func_desc = "Write to file using mmap system call, variable block size.";

void write_to_file(int fd, size_t size, size_t block_size, char* buf) {

    int t = ftruncate(fd, size);
    if (t < 0) {
        perror("ftruncate");
        exit(1);
    }
    char* data = (char *) mmap(NULL, size,PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == (char*) MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    size_t n = 0;
    while (n < size) {
        size_t nw;
        if (block_size < size - n) {
            nw = block_size;
        } else {
            nw = size - n;
        }
        memcpy(&data[n], buf, nw);
        n += nw;
    }
}