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
#include <cstring>
#include <algorithm>

extern "C" {
    extern const char func_type;
    extern const char* func_desc;
    extern void write_to_file(int fd, size_t size, size_t block_size, char* buf);
    extern size_t read_from_file(int fd, size_t size, size_t block_size, char* buf);
}

const size_t bigger_than_cachesize = 10 * 1024 * 1024;
long *p = new long[bigger_than_cachesize];

void reference_write(int fd, size_t size, char* buf) {
    FILE *file = fdopen(fd, "w");
    fwrite(buf, 1, size, file);
}

size_t reference_read(int fd, size_t size, char* buf) {
    FILE *file = fdopen(fd, "r");
    size_t r = fread(buf,1, size,file);
    return r;
}

unsigned int xorbuf(unsigned int *buffer, int size) {
    unsigned int result = 0;
    for (int i = 0; i < size; ++i) {
        result ^= buffer[i];
    }
    return result;
}

ssize_t get_block_size(int fd)
{
    struct statfs st;
    assert(fstatfs(fd, &st) != -1);
    return (ssize_t) st.f_bsize;
}


void run_write_benchmark(size_t file_size = 1024000) {
    srand(time(nullptr));
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

    for(int i = 0; i < bigger_than_cachesize; i++)
    {
        p[i] = rand();
    }

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

    for(int i = 0; i < bigger_than_cachesize; i++)
    {
        p[i] = rand();
    }

    /*
     * ~4096 seems to be a plateau for write()
     * ~1024 plateau for mmap
     */

//    size_t block_size = get_block_size(fd) * 32;
    size_t block_size = 4096;
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
}

void run_read_benchmark(size_t file_size = 1024000) {

    int fd = open("write_reference", O_RDONLY);
    char buf[file_size];
    memset(buf, 0, sizeof(buf));

    for(int i = 0; i < bigger_than_cachesize; i++)
    {
        p[i] = rand();
    }

    double seconds;
    auto start = std::chrono::steady_clock::now();
    reference_read(fd, file_size, buf);
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end - start;
    seconds = diff.count();
    double reference_rate = file_size / pow(10, 6)  /seconds;

    std::cout << "Reference - MiB/s: " << reference_rate << std::endl;

    close(fd);

    unsigned int ref_xor = xorbuf(reinterpret_cast<unsigned int *>(buf), file_size / 4);

    char test_buf[file_size];
    memset(test_buf, 0, sizeof(test_buf));
    // O_DIRECT has huge effect here
    fd = open("write_naive", O_RDONLY | O_DIRECT);
//    fd = open("write_naive", O_RDONLY);

    for(int i = 0; i < bigger_than_cachesize; i++)
    {
        p[i] = rand();
    }

    size_t block_size = get_block_size(fd) * 32;
//    size_t block_size = 4096;
    std::cout << "Block size: " << block_size << std::endl;

    start = std::chrono::steady_clock::now();
    read_from_file(fd, file_size, block_size, test_buf);
    end = std::chrono::steady_clock::now();

    unsigned int test_xor = xorbuf(reinterpret_cast<unsigned int *>(test_buf), file_size / 4);


    if (ref_xor != test_xor) {
        std::cout << "XORs do not match: " << ref_xor << " vs. " << test_xor << std::endl;
    }

    diff = end - start;
    seconds = diff.count();
    double test_rate = file_size / pow(10, 6)  /seconds;
    std::cout << "Test - MiB/s: " << test_rate << std::endl;

    std::cout << "% speed-up: " << (test_rate - reference_rate) / reference_rate * 100 << "%" << std::endl;

    close(fd);

}

/*
 * * ./run <filename> [-r|-w] <block_size> <block_count>
 */

int main(int argc, char **argv) {

    std::cout << "Description:\t" << func_desc << std::endl << std::endl;
    std::cout << std::fixed << std::setprecision(2);

    size_t file_size = 1024000; // ~ 1MB

    switch (func_type) {
        case 'r':
            run_read_benchmark(file_size);
            break;
        case 'w':
            run_write_benchmark(file_size);
            break;
        default:
            std::cout << "Operation type not specified. Please provide -r or -w arg to command." << std::endl;
    }

    return 0;
}
