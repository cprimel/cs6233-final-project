#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/stat.h>
#include <limits.h>

unsigned int xorbuf(const unsigned int *buffer, size_t size);

unsigned int read_from_file(int fd, size_t size, size_t block_size, char* buf) {
    unsigned int xor_res = 0;

    size_t n = 0;
    while ( n < size) {
        ssize_t r = read(fd, buf, block_size);

        xor_res ^= xorbuf((unsigned int*)buf, r / 4);
        if (r != (ssize_t) block_size) {
            break;
        }
        n += r;
    }
    return xor_res;
}

size_t write_to_file(int fd, size_t size, size_t block_size, char* buf) {

    char* tmp = buf;

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
        if (buf - tmp < block_size){
            buf += r;
        } else {
            buf = tmp;
        }
    }
    return n;
}
unsigned int xorbuf(const unsigned int *buffer, size_t size) {
    unsigned int result = 0;
    for (int i = 0; i < size; ++i) {
        result ^= buffer[i];
    }
    return result;
}

size_t filesize(int fd) {
    struct stat s;
    int r = fstat(fd, &s);
    if (r >= 0 && S_ISREG(s.st_mode) && s.st_size <= SSIZE_MAX) {
        return s.st_size;
    } else {
        return -1;
    }
}

int main(int argc, char *argv[]) {

    if (argc < 4) {
        printf("Missing arguments. Expected <filename> [-r|-w] <block_size> <block_count>");
        return 0;
    }

    char * filename = argv[1];
    char ops = argv[2][1];
    const size_t block_size = strtol(argv[3], NULL, 10);
    const size_t block_count = strtol(argv[4], NULL, 10);

    srand(time(NULL));
    size_t file_size = block_size * block_count;
    char buf[block_size];
    memset(buf, 0, block_size);
    char alphanumeric[] = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    const int l = sizeof(alphanumeric);
    int fd;

    switch(ops) {
        case 'w':
            for (int i = 0; i < block_size; ++i) {
                buf[i] = alphanumeric[rand() % l];
            }
            fd = open(filename, O_RDWR | O_CREAT | O_TRUNC);
            size_t bytes_written = write_to_file(fd, file_size, block_size, buf);

            double mega = pow(1000,2);
            double giga = pow(1000,3);
            if ((bytes_written / giga) >= 1.0) {
                printf("%.2f GB written to disk.\n", bytes_written / giga);
            } else if ((bytes_written / mega) > 1.0) {
                printf("%.2f MB written to disk.\n", bytes_written / mega);
            } else {
                printf("%.2f kB written to disk.\n", bytes_written / 1000.0);
            }

            close(fd);
            break;
        case 'r':
            if (file_size == 0){
                file_size = filesize(fd);
            }
            fd = open(filename, O_RDONLY);
            unsigned int xor_res = read_from_file(fd, file_size, block_size, buf);
            printf("XOR: %u", xor_res);
            close(fd);
            break;
        default:
            printf("-%c is not a valid option. Please enter either -r or -w.", ops);
    }
    return 0;
}