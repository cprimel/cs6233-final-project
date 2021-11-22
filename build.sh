#!/bin/bash
mkdir -p release
cd release || exit
cmake ..
cmake --build