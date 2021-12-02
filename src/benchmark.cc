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
#include <numeric>

extern "C" {
extern const char func_type;
extern const char *func_desc;
extern void write_to_file(int fd, size_t size, size_t block_size, char *buf);
extern size_t read_from_file(int fd, size_t size, size_t block_size, char *buf);
}

struct Rates {
    Rates(double min, double avg, double max) : min(min), avg(avg), max(max) {};
    double min{}, avg{}, max{};

    bool operator<(const Rates &r) const {
        if (avg < r.avg) {
            return true;
        }
        return false;
    }
};

const size_t bigger_than_cachesize = 10 * 1024 * 1024;
long *p = new long[bigger_than_cachesize];

void reference_write(int fd, size_t size, char *buf) {
    FILE *file = fdopen(fd, "w");
    fwrite(buf, 1, size, file);
}

size_t reference_read(int fd, size_t size, char *buf) {
    FILE *file = fdopen(fd, "r");
    size_t r = fread(buf, 1, size, file);
    return r;
}

unsigned int xorbuf(unsigned int *buffer, int size) {
    unsigned int result = 0;
    for (int i = 0; i < size; ++i) {
        result ^= buffer[i];
    }
    return result;
}

[[maybe_unused]] ssize_t get_block_size(int fd) {
    struct statfs st;
    assert(fstatfs(fd, &st) != -1);
    return (ssize_t) st.f_bsize;
}


void run_write_benchmark(const char *filename, size_t file_size, size_t block_size) {

    std::vector<size_t> test_sizes;
    if (block_size == 0) {
        test_sizes.resize(15);
        std::generate(test_sizes.begin(), test_sizes.end(), [n = 16]() mutable { return n <<= 1; });
    } else {
        test_sizes.push_back(block_size);
    }

    std::vector<Rates> per;


    srand(time(nullptr));
    const char alphanumeric[] = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    const int l = sizeof(alphanumeric);


    for (size_t block: test_sizes) {
        char rand_arr[file_size];

        std::vector<double> rates;
        for (int num_iterations = 0; num_iterations < 100; ++num_iterations) {

            for (int i = 0; i < file_size; ++i) {
                rand_arr[i] = alphanumeric[rand() % l];
            }

            std::ofstream{filename};
            /*
             * O_DIRECT does not seem to make that much difference.
             */
//            int fd = open(filename, O_RDWR | O_APPEND | O_TRUNC | O_DIRECT);
            int fd = open(filename, O_RDWR | O_APPEND | O_TRUNC);
            double seconds;
            auto start = std::chrono::steady_clock::now();
            write_to_file(fd, file_size, block, rand_arr);
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> diff = end - start;
            seconds = diff.count();
            double test_rate = file_size / pow(10, 6) / seconds;
            rates.push_back(test_rate);

            close(fd);
        }

        auto avg = std::reduce(rates.begin(), rates.end()) / rates.size();
        const auto[min, max] = std::minmax_element(rates.begin(), rates.end());
        per.emplace_back(*min, avg, *max);

    }

    if (test_sizes.size() > 1) {
        auto result = std::max_element(per.begin(), per.end());
        auto argmax = std::distance(per.begin(), result);


        std::cout << "Best block size: " << test_sizes[argmax] / 1024 << " kB" << std::endl;
        std::cout << "Minimum performance: " << per[argmax].min << " MiB/s" << std::endl;
        std::cout << "Average performance: " << per[argmax].avg << " MiB/s" << std::endl;
        std::cout << "Maximum performance: " << per[argmax].max << " MiB/s" << std::endl;

    } else {
        std::cout << "Block size: " << test_sizes[0] / 1024 << " kB" << std::endl;
        std::cout << "Minimum write speed: " << per[0].min << " MiB/s" << std::endl;
        std::cout << "Average write speed: " << per[0].avg << " MiB/s" << std::endl;
        std::cout << "Maximum write speed: " << per[0].max << " MiB/s" << std::endl;

    }


}

void run_read_benchmark(const char *filename, size_t file_size, size_t block_size) {

    int fd = open(filename, O_RDONLY);
    char buf[file_size];
    memset(buf, 0, sizeof(buf));
    reference_read(fd, file_size, buf);
    close(fd);

    unsigned int ref_xor = xorbuf(reinterpret_cast<unsigned int *>(buf), file_size / 4);

    for(int i = 0; i < bigger_than_cachesize; i++)
    {
        p[i] = rand();
    }

    std::vector<size_t> test_sizes;
    if (block_size == 0) {
        test_sizes.resize(15);
        std::generate(test_sizes.begin(), test_sizes.end(), [n = 16]() mutable { return n <<= 1; });
    } else {
        test_sizes.push_back(block_size);
    }

    std::vector<Rates> per;

    for (size_t block: test_sizes) {
        std::vector<double> rates;
        for (int num_iterations = 0; num_iterations < 100; ++num_iterations) {


            char test_buf[file_size];
            memset(test_buf, 0, sizeof(test_buf));

            // O_DIRECT has huge effect here
            fd = open(filename, O_RDONLY | O_DIRECT);
//            fd = open(filename, O_RDONLY);

            for(int i = 0; i < bigger_than_cachesize; i++)
            {
                p[i] = rand();
            }
            double seconds;
            auto start = std::chrono::steady_clock::now();
            read_from_file(fd, file_size, block, test_buf);
            auto end = std::chrono::steady_clock::now();

            unsigned int test_xor = xorbuf(reinterpret_cast<unsigned int *>(test_buf), file_size / 4);

            if (ref_xor != test_xor) {
                std::cout << "XORs do not match. Error reading file." << std::endl;
                return;
            }

            std::chrono::duration<double> diff = end - start;
            seconds = diff.count();
            double test_rate = file_size / pow(10, 6) / seconds;
            rates.push_back(test_rate);
            close(fd);
        }

        auto avg = std::reduce(rates.begin(), rates.end()) / rates.size();
        const auto[min, max] = std::minmax_element(rates.begin(), rates.end());
        per.emplace_back(*min, avg, *max);

    }

    if (test_sizes.size() > 1) {
        auto result = std::max_element(per.begin(), per.end());
        auto argmax = std::distance(per.begin(), result);


        std::cout << "Best block size: " << test_sizes[argmax] / 1024 << " kB" <<  std::endl;
        std::cout << "Minimum read speed: " << per[argmax].min << " MiB/s" << std::endl;
        std::cout << "Average read speed: " << per[argmax].avg << " MiB/s" << std::endl;
        std::cout << "Maximum read speed: " << per[argmax].max << " MiB/s" << std::endl;

    } else {
        std::cout << "Block size: " << test_sizes[0] / 1024 << " kB" << std::endl;
        std::cout << "Minimum read speed: " << per[0].min << " MiB/s" << std::endl;
        std::cout << "Average read speed: " << per[0].avg << " MiB/s" << std::endl;
        std::cout << "Maximum read speed: " << per[0].max << " MiB/s" << std::endl;

    }

}


/*
 * * ./run <filename> [-r|-w] <block_size> <block_count>
 */

int main(int argc, char **argv) {

    std::cout << "Description:\t" << func_desc << std::endl << std::endl;
    std::cout << std::fixed << std::setprecision(2);

    size_t file_size = 1024 * 1024; // ~ 1MB
    size_t block_size = 0;
    const char *filename = "write_test";

    switch (func_type) {
        case 'r':
            run_read_benchmark(filename, file_size, block_size);
            break;
        case 'w':
            run_write_benchmark(filename, file_size, block_size);
            break;
        default:
            std::cout << "Operation type not specified. Please provide -r or -w arg to command." << std::endl;
    }

    return 0;
}
