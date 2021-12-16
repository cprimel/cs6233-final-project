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
#include <iterator>
#include <climits>
#include <sys/stat.h>
#include <string>


bool NO_FS_CACHE = false;

extern "C" {
extern const char func_type;
extern const char *func_desc;
extern const char *func_name;
extern const size_t min_block;
extern void write_to_file(int fd, size_t size, size_t block_size, char *buf);
extern unsigned int read_from_file(int fd, size_t size, size_t block_size, char *buf);
}

unsigned int xorbuf(unsigned int *buffer, int size);

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
    FILE *file = fdopen(fd, "w+");
    size_t n = 0;
    while (n < size) {
        size_t w = fwrite(buf, 1, sizeof(buf), file);
        n += w;
    }
}

unsigned int reference_read(int fd, size_t size, size_t block_size, char *buf) {
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

size_t filesize(int fd) {
    struct stat s;
    int r = fstat(fd, &s);
    if (r >= 0 && S_ISREG(s.st_mode) && s.st_size <= SSIZE_MAX) {
        return s.st_size;
    } else {
        return -1;
    }
}


void store_data(
        const std::string& func_id,
        std::vector<size_t> test_sizes,
        std::vector<Rates> per,
        std::vector<std::vector<double>> benchmark_data) {
    std::ofstream file;
    std::string cache_flag = NO_FS_CACHE ? "nocache" : "cache";
    file.open(func_id + "_" + func_type + "_" + cache_flag + "_" + std::to_string(std::time(nullptr)) + ".csv");
    for (int i = 0; i < test_sizes.size(); ++i) {
        file << test_sizes[i] << ",";
        std::copy(benchmark_data[i].begin(), benchmark_data[i].end(), std::ostream_iterator<double>(file, ","));
        file << per[i].min << "," << per[i].avg << "," << per[i].max;
        file << std::endl;
    }
    file.close();

}

// Source: https://github.com/GregoryConrad/pBar
class pBar {
public:
    void update(double newProgress) {
        currentProgress += newProgress;
        amountOfFiller = (int)((currentProgress / neededProgress)*(double)pBarLength);
    }
    void print() {
        currUpdateVal %= pBarUpdater.length();
        std::cout << "\r" //Bring cursor to start of line
             << firstPartOfpBar; //Print out first part of pBar
        for (int a = 0; a < amountOfFiller; a++) { //Print out current progress
            std::cout << pBarFiller;
        }
        std::cout << pBarUpdater[currUpdateVal];
        for (int b = 0; b < pBarLength - amountOfFiller; b++) { //Print out spaces
            std::cout << " ";
        }
        std::cout << lastPartOfpBar //Print out last part of progress bar
             << " (" << (int)(100*(currentProgress/neededProgress)) << "%)" //This just prints out the percent
             << std::flush;
        currUpdateVal += 1;
    }
    std::string firstPartOfpBar = "[", //Change these at will (that is why I made them public)
    lastPartOfpBar = "]",
            pBarFiller = "|",
            pBarUpdater = "/-\\|";
private:
    int amountOfFiller,
            pBarLength = 50, //I would recommend NOT changing this
    currUpdateVal = 0; //Do not change
    double currentProgress = 0, //Do not change
    neededProgress = 100; //I would recommend NOT changing this
};


void run_write_benchmark(const char *filename, size_t file_size, size_t block_size) {

    std::vector<size_t> test_sizes;
    if (block_size == 0) {
        test_sizes.resize(15);
        std::generate(test_sizes.begin(), test_sizes.end(), [n = 16]() mutable { return n <<= 1; });
    } else {
        test_sizes.push_back(block_size);
    }

    std::vector<Rates> per;
    std::vector<std::vector<double>> benchmark_data;

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
        benchmark_data.push_back(rates);
    }

    if (test_sizes.size() > 1) {
        auto result = std::max_element(per.begin(), per.end());
        auto argmax = std::distance(per.begin(), result);


        std::cout << "Best block size: " << test_sizes[argmax] << " kB" << std::endl;
        std::cout << "Minimum performance: " << per[argmax].min << " MiB/s" << std::endl;
        std::cout << "Average performance: " << per[argmax].avg << " MiB/s" << std::endl;
        std::cout << "Maximum performance: " << per[argmax].max << " MiB/s" << std::endl;

    } else {
        std::cout << "Block size: " << test_sizes[0] << " kB" << std::endl;
        std::cout << "Minimum write speed: " << per[0].min << " MiB/s" << std::endl;
        std::cout << "Average write speed: " << per[0].avg << " MiB/s" << std::endl;
        std::cout << "Maximum write speed: " << per[0].max << " MiB/s" << std::endl;

    }
    store_data(func_name, test_sizes, per, benchmark_data);

}

void run_read_benchmark(const char *filename, size_t block_count, size_t block_size) {

    int fd = open(filename, O_RDONLY);
    size_t file_size = filesize(fd);
    if (block_size == 0 && block_count > 0) {
        size_t mib_blocks = 1024 * 1024 * block_count;
        file_size = mib_blocks < file_size ? mib_blocks : file_size; // 1 MiB-sized blocks
    }

    char buf[4096];

    memset(buf, 0, sizeof(buf));
    unsigned int ref_xor = reference_read(fd, file_size, sizeof(buf), buf);
    close(fd);
    std::cout << "Reference XOR: " << ref_xor << std::endl;


//    for(int i = 0; i < bigger_than_cachesize; i++)
//    {
//        p[i] = rand();
//    }

    std::vector<size_t> test_sizes;
    if (block_size == 0) {
        test_sizes.resize(12);
        test_sizes[0] = min_block;
        std::generate(test_sizes.begin() + 1, test_sizes.end(), [n = min_block ]() mutable { return n <<= 1; });
    } else {
        test_sizes.push_back(block_size);
    }

    std::vector<Rates> per;
    std::vector<std::vector<double>> benchmark_data;
    for (size_t block: test_sizes) {
        std::vector<double> rates;
        size_t num_iterations = 25;

        std::cout << "Block size: " << block << std::endl; // For manual debugging

        for (size_t trial = 0; trial < num_iterations; ++trial) {
            std::cout << "Trial #" << trial << "..." << std::endl; // For manual debugging
            alignas(64) char test_buf[block];
            memset(test_buf, 0, sizeof(test_buf));

            // O_DIRECT has huge effect here
            fd = open(filename, O_RDONLY | O_DIRECT);
//            fd = open(filename, O_RDONLY);

//            for(int i = 0; i < bigger_than_cachesize; i++)
//            {
//                p[i] = rand();
//            }
            if (NO_FS_CACHE) {
                sync();
                std::ofstream ofs("/proc/sys/vm/drop_caches");
                ofs << "3" << std::endl;
            }


            double seconds;
            auto start = std::chrono::steady_clock::now();
            unsigned int test_xor = read_from_file(fd, file_size, block, test_buf);
            auto end = std::chrono::steady_clock::now();

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
        benchmark_data.push_back(rates);
    }

    if (test_sizes.size() > 1) {
        auto result = std::max_element(per.begin(), per.end());
        auto argmax = std::distance(per.begin(), result);


        std::cout << "Best block size: " << test_sizes[argmax] << " kB" <<  std::endl;
        std::cout << "Minimum read speed: " << per[argmax].min << " MiB/s" << std::endl;
        std::cout << "Average read speed: " << per[argmax].avg << " MiB/s" << std::endl;
        std::cout << "Maximum read speed: " << per[argmax].max << " MiB/s" << std::endl;

    } else {
        std::cout << "Block size: " << test_sizes[0] << " kB" << std::endl;
        std::cout << "Minimum read speed: " << per[0].min << " MiB/s" << std::endl;
        std::cout << "Average read speed: " << per[0].avg << " MiB/s" << std::endl;
        std::cout << "Maximum read speed: " << per[0].max << " MiB/s" << std::endl;

    }
    store_data(func_name,test_sizes,per, benchmark_data);
}


int arg_to_int(const std::string& arg) {
    int parsed_arg = -1;
    try {
        std::size_t pos;
        parsed_arg = std::stoi(arg, &pos);
        if (pos < arg.size()) {
            std::cerr << "Trailing characters after number: " << arg << '\n';
        }
    } catch (std::invalid_argument const &ex) {
        std::cerr << "Invalid number: " << arg << '\n';
    } catch (std::out_of_range const &ex) {
        std::cerr << "Number out of range: " << arg << '\n';
    }
    return parsed_arg;
}

/*
 * * ./bench_{BENCH_NAME} <filename> [cache_flag-y|-n]  <block_size> <block_count>
 */

int main(int argc, char **argv) {


    if (argc < 3) {
        std::cout << argc << " arguments passed, expected >= 3 in form ./bench_{BENCH_NAME} <filename> [-cache|-nocache]  <block_size> <block_count>" <<std::endl;
    }
    const char* filename = argv[1];
    const char cache_flag = argv[2][1];
    if (cache_flag == 'n') {
        NO_FS_CACHE = true;
    }
    size_t block_size = arg_to_int(argv[3]);
    size_t block_count = arg_to_int(argv[4]);


    std::cout << "Description:\t" << func_desc << std::endl << std::endl;
    std::cout << std::fixed << std::setprecision(2);



    std::cout << "Block count (0 = file size): " << block_count << ", Block size (0 = find best): " << block_size << std::endl;
    std::cout << "Cache? " << cache_flag << std::endl;
    std::cout << "File name: " << filename << std::endl;
    switch (func_type) {
        case 'r':
            run_read_benchmark(filename, block_count, block_size);
            break;
        case 'w':
            run_write_benchmark(filename, block_size * block_count, block_size);
            break;
        default:
            std::cout << "Operation type not specified. Please check that .c file includes func_type." << std::endl;
    }

    return 0;
}
