#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

size_t read_from_file(int fd, size_t file_size, size_t block_size, char* buf) {

    size_t n = 0;
    while ( n < file_size) {
        ssize_t r = read(fd, buf, block_size);
        if (r != (ssize_t) block_size) {
            break;
        }
        n += r;
        buf += r;
    }
    return n;
}

size_t write_to_file(int fd, size_t size, size_t block_size, char* buf) {

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
    return n;
}
unsigned int xorbuf(unsigned int *buffer, size_t size) {
    unsigned int result = 0;
    for (int i = 0; i < size; ++i) {
        result ^= buffer[i];
    }
    return result;
}

int main(int argc, char **argv) {

    char * filename = argv[1];
    char ops = argv[2][1];
    const size_t block_size = strtol(argv[3], NULL, 10);
    const size_t block_count = strtol(argv[4], NULL, 10);

    srand(time(NULL));
    const size_t file_size = block_size * block_count;
    char buf[file_size];
    memset(buf, 0, file_size);
    char alphanumeric[] = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    const int l = sizeof(alphanumeric);
    int fd;

    switch(ops) {
        case 'w':
            for (int i = 0; i < file_size; ++i) {
                buf[i] = alphanumeric[rand() % l];
            }
            fd = open(filename, O_RDWR | O_CREAT);
            size_t bytes_written = write_to_file(fd, file_size, block_size, buf);
            close(fd);
            break;
        case 'r':
            fd = open(filename, O_RDONLY);
            size_t n = read_from_file(fd, file_size, block_size, buf);
            unsigned int xor_res = xorbuf((unsigned int *) buf, n / 4);
            printf("XOR: %u", xor_res);
            close(fd);
            break;
        default:
            printf("%c is not a valid option. Please see run --help for options.", ops);
    }
    return 0;
}