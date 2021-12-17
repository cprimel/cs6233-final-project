# Final Project for CS6233 Fall 2021

Or, making disks go brrrr.

## Instructions

Requires `cmake` and `gcc.` Some of the benchmark programs use intrinsics and, therefore, are architecture specific.

To build all the files in the project:
```shell
$ ./build.sh
```

For Part 1:
```shell
$ ./release/run <filename> [-r|-w] <block_size> <block_count>
```

For Part 2:
```shell
$ ./release/run2 <filename> <block_size>
```

For Part 5:
```shell
$ ./release/run5 <filename>
```

For Part 6:
```shell
$ ./release/fast <filename>
```