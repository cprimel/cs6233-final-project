#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <chrono>
#include <fstream>

size_t read_from_file(int fd, size_t file_size, size_t block_size, char *buf) {

    size_t n = 0;
    while (n < file_size) {
        ssize_t r = read(fd, buf, block_size);
        if (r != (ssize_t) block_size) {
            break;
        }
        n += r;
        buf += r;
    }
    return n;
}

int main(int argc, char **argv) {

    char *filename = argv[1];
    const size_t block_size = strtol(argv[2], nullptr, 10);

    srand(time(nullptr));
    char buf[block_size];
    memset(buf, 0, block_size);
    char alphanumeric[] = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    const int l = sizeof(alphanumeric);
    for (int i = 0; i < block_size; ++i) {
        buf[i] = alphanumeric[rand() % l];
    }

    std::ofstream{filename};
    FILE *file = fopen(filename, "a");

    size_t file_size = block_size * 1024;
    size_t n = 0;

    while (n < file_size) {
        n += fwrite(buf, 1, sizeof(buf), file);
    }

    fclose(file);
    memset(buf, 0, block_size);

    double seconds = 0.0;
    double timeout = 5;
    n = 0;
    file = fopen(filename, "r");
    while (seconds < timeout) {
        auto start = std::chrono::steady_clock::now();
        n += fread(buf, 1,  sizeof(buf), file);
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> diff = end - start;
        seconds += diff.count();
        if (n % file_size == 0) {
            rewind(file);
        }
    }
   fclose(file);



    printf("File size: %zu GB (%zu bytes or %d %zu-byte blocks), Time: %f seconds", n >> 30, n, n / 4096, 4096, seconds);

    return 0;
}