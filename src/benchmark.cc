#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <chrono>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <sys/statfs.h>
#include <cassert>

extern "C" {
    extern const char* func_desc;
    extern void write_to_file(int fd, size_t size, size_t block_size, char* buf);
}

void reference_write(int fd, size_t size, char* buf, size_t block_size = 1) {

    size_t n = 0;
    while (n < size) {
        write(fd, buf, block_size);
        ++n;
        ++buf;
    }

}

ssize_t get_block_size(int fd)
{
    struct statfs st;
    assert(fstatfs(fd, &st) != -1);
    return (ssize_t) st.f_bsize;
}


/*
 * * ./run <filename> [-r|-w] <block_size> <block_count>
 */

int main(int argc, char **argv) {
    std::cout << "Description:\t" << func_desc << std::endl << std::endl;
    std::cout << std::fixed << std::setprecision(2);


    srand(time(nullptr));
    size_t file_size = 1024000; // ~ 1MB
    const char alphanumeric[] = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    char rand_arr[file_size];
    const int l = sizeof(alphanumeric);

    for (int i = 0; i < file_size; ++i) {
        rand_arr[i] = alphanumeric[rand() % l];
    }

    std::ofstream  {"write_reference"};
    /*
     * O_SYNC turns off all caching by the OS; however, it ~0.0006 MiB/s
     * making it impractical for running multiple benchmarks.
     */
//    int fd = open("write_reference", O_WRONLY | O_APPEND | O_SYNC);
    int fd = open("write_reference", O_WRONLY | O_APPEND);


    double seconds;
    auto start = std::chrono::steady_clock::now();
    reference_write(fd, file_size, rand_arr);
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end - start;
    seconds = diff.count();
    double reference_rate = file_size / pow(10, 6)  /seconds;

    std::cout << "Reference - MiB/s: " << reference_rate << std::endl;
    close(fd);

    std::ofstream  {"write_naive"};
    /*
     * O_DIRECT does not seem to make that much difference.
     */
//    fd = open("write_naive", O_RDWR | O_APPEND | O_TRUNC | O_DIRECT);
    fd = open("write_naive", O_RDWR | O_APPEND | O_TRUNC);

    /*
     * ~4096 seems to be a plateau for write()
     * ~1024 plateau for mmap
     */

    size_t block_size = get_block_size(fd) * 32;
//    size_t block_size = 4096;
    std::cout << "Block size: " << block_size << std::endl;

    start = std::chrono::steady_clock::now();
    write_to_file(fd, file_size, block_size, rand_arr);
    end = std::chrono::steady_clock::now();
    diff = end - start;
    seconds = diff.count();
    double test_rate = file_size / pow(10, 6)  /seconds;
    std::cout << "Test - MiB/s: " << test_rate << std::endl;

    std::cout << "% speed-up: " << (test_rate - reference_rate) / reference_rate * 100 << "%" << std::endl;

    close(fd);

    return 0;
}
