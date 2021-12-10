#include <sys/mman.h>
#include <string.h>

#include <immintrin.h>

const char func_type = 'r';
const char *func_name = "mmapavx";
const char *func_desc = "Read from file using memory mapping and AVX instrinsics, entire file.";

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

size_t read_from_file(int fd, size_t size, size_t block_size, char *buf) {
    char *file_data = (char *) mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    size_t n = size;

    memcpy_handler(buf, file_data, size);

    munmap(file_data, n);

    return n;
}

void write_to_file(int fd, size_t size, size_t block_size, char *buf) {};