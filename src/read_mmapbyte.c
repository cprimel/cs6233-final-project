#include <sys/mman.h>
#include <string.h>


const char func_type = 'r';
const char *func_desc = "Read from file using mmap OS system call, byte by byte.";



size_t read_from_file(int fd, size_t size, size_t block_size, char* buf) {

    char* file_data = (char*) mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    size_t n = 0;
    while (n < size) {
        memcpy(buf, &file_data[n], 1);
        ++n;
        ++buf;
    }
    munmap(file_data, size);
    return n;
}

void write_to_file(int fd, size_t size, size_t block_size, char* buf) {};