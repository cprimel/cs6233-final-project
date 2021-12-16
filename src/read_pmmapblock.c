#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <pthread.h>
#include <string.h>

const char func_type = 'r';
const char *func_name = "pthread_mmapblock";
const char *func_desc = "Read from file using pthread + mmap, variable block size.";
const size_t min_block = 64;

struct arg_struct {
    char* file_data;
    char** buf;
    size_t stop_read;
    size_t block_size;
};

void *read_routine(void *args) {

    struct arg_struct *t_args = (struct arg_struct *)args;
    char* file_data = t_args->file_data;
    char** buf = t_args->buf;
    size_t stop_read = t_args->stop_read;
    size_t block_size = t_args->block_size;


    size_t n = 0;
    while (n < stop_read) {
        size_t nw;
        if (block_size < stop_read - n) {
            nw = block_size;
        } else {
            nw = stop_read - n;
        }

        memcpy(buf, &file_data[n], nw);

        n += nw;
        buf += nw;
    }

    pthread_exit((void* )buf);
}

unsigned int xorbuf(const unsigned int *buffer, int size) {
    unsigned int result = 0;
    for (int i = 0; i < size; ++i) {
        unsigned int val = buffer[i];
        result ^= val;
    }
    return result;
}

unsigned int read_from_file(int fd, size_t size, size_t block_size, char* buf) {
    unsigned int xor_res = 0;

    char* file_data = (char*) mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);

    pthread_t *threads;
    int num_threads = 4;
    threads = (pthread_t *) malloc(sizeof(pthread_t)*num_threads);
    char * thread_buf[num_threads];

    size_t offset = size / num_threads;
    for (int i = 0; i < num_threads; i++) {
        struct arg_struct *args = malloc(sizeof(struct arg_struct));
        args->stop_read = offset;
        args->block_size =  block_size;
        args->file_data = file_data + offset * i;;
        args->buf = &thread_buf[i];

        pthread_create(&threads[i], NULL, read_routine, (void *)args);
    }


    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], (void**) &thread_buf[i]);
        xor_res ^= xorbuf((const unsigned int *) thread_buf[i], block_size);
    }
    munmap(file_data, size);
    free(threads);
    return xor_res;
}

void write_to_file(int fd, size_t size, size_t block_size, char* buf) {};