#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>

const char func_type = 'w';
const char *func_name = "mmapbyte";
const char *func_desc = "Write to file using mmap system call, one byte at a time.";

void write_to_file(int fd, size_t size, size_t block_size, char* buf) {

    int r = ftruncate(fd, size);
    if (r < 0) {
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
        memcpy(&data[n], buf, 1);
        n += 1;
        buf += 1;
    }
}

size_t read_from_file(int fd, size_t size, size_t block_size, char* buf) { return -1;}