#include <sys/mman.h>
#include <string.h>

#include <immintrin.h>


const char func_type = 'r';
const char *func_name = "mmapavx";
const char *func_desc = "Read from file using memory mapping and AVX instrinsics, entire file.";
size_t min_block = 512;

unsigned int xorbuf(const unsigned int *buffer, int size);

// 512 bytes, 256 bit registers, aligned
void * memcpy_intrinsic(void *dest, const void *src, size_t len)
{
    const __m256i* s = (__m256i*)src;
    __m256i* d = (__m256i*)dest;

    while (len--)
    {
        _mm256_store_si256(d++, _mm256_load_si256(s++)); // 1
        _mm256_store_si256(d++, _mm256_load_si256(s++)); // 2
        _mm256_store_si256(d++, _mm256_load_si256(s++)); // 3
        _mm256_store_si256(d++, _mm256_load_si256(s++)); // 4
        _mm256_store_si256(d++, _mm256_load_si256(s++)); // 5
        _mm256_store_si256(d++, _mm256_load_si256(s++)); // 6
        _mm256_store_si256(d++, _mm256_load_si256(s++)); // 7
        _mm256_store_si256(d++, _mm256_load_si256(s++)); // 8
        _mm256_store_si256(d++, _mm256_load_si256(s++)); // 9
        _mm256_store_si256(d++, _mm256_load_si256(s++)); // 10
        _mm256_store_si256(d++, _mm256_load_si256(s++)); // 11
        _mm256_store_si256(d++, _mm256_load_si256(s++)); // 12
        _mm256_store_si256(d++, _mm256_load_si256(s++)); // 13
        _mm256_store_si256(d++, _mm256_load_si256(s++)); // 14
        _mm256_store_si256(d++, _mm256_load_si256(s++)); // 15
        _mm256_store_si256(d++, _mm256_load_si256(s++)); // 16
    }

    return dest;
}

void memcpy_handler(char* dest, char* src, size_t n_bytes) {

    while (n_bytes) {
        memcpy_intrinsic(dest, src, n_bytes >> 9);
        size_t offset = n_bytes & -512;
        dest = (char *)dest + offset;
        src = (char *)src + offset;
        n_bytes &= 511;
    }

}

unsigned int xorbuf(const unsigned int *buffer, int size) {
    unsigned int result = 0;
    for (int i = 0; i < size; ++i) {
        unsigned int val = buffer[i];
        result ^= val;
    }
    return result;
}

unsigned int read_from_file(int fd, size_t size, size_t block_size, char *buf) {
    char *file_data = (char *) mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    unsigned int xor_res = 0;

    size_t n = 0;
    while (n < size) {
        size_t nw;
        if (block_size < size - n) {
            nw = block_size;
        } else {
            nw = size - n;
        }
        memcpy_handler(buf, &file_data[n], nw);
        xor_res ^= xorbuf((unsigned int*)buf, nw / 4);
        n += nw;
    }
    munmap(file_data, size);
    return xor_res;
}

void write_to_file(int fd, size_t size, size_t block_size, char *buf) {};