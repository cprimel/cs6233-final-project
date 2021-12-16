#include <sys/mman.h>
#include <string.h>
#include <immintrin.h>

const char func_type = 'r';
const char *func_name = "mmapavx512";
const char *func_desc = "Read from file using memory mapping and AVX512 instrinsics.";
const size_t min_block = 4096;

// 4096 bytes, or 4 kB
// m5d.metal instance runs on Xeon Scalable with AVX512 extension!!!
void * memcpy_intrinsics(void *dest, const void *src, size_t len)
{
    const __m512i* s = (__m512i*)src;
    __m512i* d = (__m512i*)dest;

    while (len--)
    {
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 1
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 2
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 3
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 4
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 5
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 6
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 7
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 8
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 9
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 10
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 11
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 12
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 13
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 14
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 15
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 16
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 17
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 18
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 19
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 20
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 21
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 22
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 23
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 24
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 25
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 26
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 27
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 28
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 29
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 30
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 31
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 32
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 1
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 2
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 3
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 4
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 5
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 6
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 7
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 8
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 9
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 10
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 11
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 12
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 13
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 14
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 15
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 16
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 17
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 18
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 19
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 20
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 21
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 22
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 23
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 24
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 25
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 26
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 27
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 28
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 29
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 30
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 31
        _mm512_store_si512(d++, _mm512_load_si512(s++)); // 32
    }

    return dest;
}


void memcpy_handler(char* dest, char* src, size_t n_bytes) {
    while (n_bytes) {
        memcpy_intrinsics(dest, src, n_bytes >> 11);
        size_t offset = n_bytes & -4096;
        dest = (char *)dest + offset;
        src = (char *)src + offset;
        n_bytes &= 4095;
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