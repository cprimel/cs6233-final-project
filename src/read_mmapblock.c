#include <sys/mman.h>
#include <string.h>


const char func_type = 'r';
const char *func_name = "mmapblock";
const char *func_desc = "Read from file using mmap OS system call, variable block size.";
const size_t min_block = 64;

unsigned int xorbuf(const unsigned int *buffer, int size);


unsigned int read_from_file(int fd, size_t size, size_t block_size, char* buf) {

    char* file_data = (char*) mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    unsigned int xor_res = 0;

    size_t n = 0;
    while (n < size) {
        size_t nw;
        if (block_size < size - n) {
            nw = block_size;
        } else {
            nw = size - n;
        }
        memcpy(buf, &file_data[n], nw);
        xor_res ^= xorbuf((unsigned int*)buf, nw / 4);
        n += nw;
    }
    munmap(file_data, size);
    return xor_res;
}

unsigned int xorbuf(const unsigned int *buffer, int size) {
    unsigned int result = 0;
    for (int i = 0; i < size; ++i) {
        unsigned int val = buffer[i];
        result ^= val;
    }
    return result;
}

void write_to_file(int fd, size_t size, size_t block_size, char* buf) {};