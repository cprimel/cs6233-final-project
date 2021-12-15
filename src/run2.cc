#include <cstdlib>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <chrono>
#include <fstream>
#include <cmath>

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

int main(int argc, char *argv[]) {

    const size_t block_size = strtol(argv[1], nullptr, 10);

    srand(time(nullptr));
    char buf[block_size];
    memset(buf, 0, block_size);
    char alphanumeric[] = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    const int l = sizeof(alphanumeric);
    for (int i = 0; i < block_size; ++i) {
        buf[i] = alphanumeric[rand() % l];
    }

    const char *filename = "test_read";
    std::ofstream{filename};

    memset(buf, 0, block_size);

    double seconds = 0.0;
    double timeout = 0.5;
    size_t file_size = block_size * 1024;
    size_t n = 0;

    while (seconds < timeout) {
        FILE *wfile = fopen(filename, "w+");
        while (n < file_size) {
            size_t w = fwrite(buf, 1, sizeof(buf), wfile);
            n += w;
        }
        fclose(wfile);
        n = 0;
        memset(buf, 0, block_size);

        FILE *rfile = fopen(filename, "r+");
        auto start = std::chrono::steady_clock::now();
        while (n < file_size) {
            size_t r = fread(buf, 1, block_size, rfile);
            if (r != block_size) {
                break;
            }
            n += r;
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> diff = end - start;
        seconds = diff.count();
        fclose(rfile);
        file_size *= 2;
    }



    printf("File size: %.2f GB , Time: %.2f seconds\n", n / pow(1000,3), seconds);

    return 0;
}