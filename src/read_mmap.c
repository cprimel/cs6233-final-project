#include <sys/mman.h>
#include <string.h>


const char func_type = 'r';
const char *func_name = "mmap";
const char *func_desc = "Read from file using mmap OS system call, entire file.";



size_t read_from_file(int fd, size_t size, size_t block_size, char* buf) {
    char* file_data = (char*) mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    memcpy(buf, file_data, size);
    return size;
}

void write_to_file(int fd, size_t size, size_t block_size, char* buf) {};