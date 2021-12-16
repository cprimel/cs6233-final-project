cmake_minimum_required(VERSION 3.20)
project(cs6233_project C CXX)

add_library(benchmark OBJECT src/benchmark.cc)
target_compile_features(benchmark PRIVATE cxx_std_11)

foreach(BENCHMARK IN ITEMS read_mmapavx read_mmapavx2 read_mmapavx512 read_osblock read_mmapbyte read_mmapblock read_stdioblock read_pmmapblock write_osblock write_mmapbyte write_mmapblock write_stdioblock)
    add_executable(benchmark_${BENCHMARK} src/${BENCHMARK}.c)
    target_link_libraries(benchmark_${BENCHMARK} PRIVATE benchmark pthread)
    target_compile_features(benchmark_${BENCHMARK} PRIVATE c_std_11 c_restrict)
    target_compile_options(benchmark_${BENCHMARK} PRIVATE -Wall -O2 -funroll-loops -march=native -mavx512dq -msse4.1)
endforeach()

add_executable(run src/run.c)
add_executable(run2 src/run2.cc)