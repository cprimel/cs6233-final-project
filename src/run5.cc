#include <unistd.h>
#include <chrono>
#include <fcntl.h>
#include <iostream>
#include <iomanip>
#include <cmath>


int main(int argc, const char **argv) {

    const char *filename = argv[1];
    size_t block_count = 1000000;
    char * buf[block_count];

    int fd = open(filename, O_RDONLY);
    double seconds;
    auto start = std::chrono::steady_clock::now();
    size_t n = 0;
    while (n < block_count) {
        read(fd, buf, 1);
        n += 1;
    }
    auto end = std::chrono::steady_clock::now();
    close(fd);

    std::chrono::duration<double> diff = end - start;
    seconds = diff.count();
    double reads_per_second = n / seconds;

    std::cout << "Read call rate: "<<reads_per_second << " B/s" <<std::endl;
    std::cout << "Read call rate: "<<reads_per_second / 1000 << " B/ms" <<std::endl;

    std::ostringstream tmp;
    tmp << std::fixed;
    tmp << std::setprecision(1);
    double mebi = pow(1024,2);

    tmp << n / seconds / mebi;
    std::string read_rate =  tmp.str() + " MiB/s";
    std::cout << "Read rate for byte-by-byte: " << read_rate << std::endl;

    fd = open(filename, O_RDONLY);
    seconds = 0;
    start = std::chrono::steady_clock::now();
     n = 0;
    while (n < block_count) {
        lseek(fd,1, SEEK_SET);
        n += 1;
    }
    end = std::chrono::steady_clock::now();
    close(fd);

    diff = end - start;
    seconds = diff.count();
    double seeks_per_second = n / seconds;
    std::cout << "Seek call rate: " << seeks_per_second << " B/s" <<std::endl;
    std::cout << "Seek call rate: " << seeks_per_second / 1000 << " B/ms" <<std::endl;


    return 0;
}