#!/bin/bash
mkdir -p release
cd release || exit
cmake ..
cmake --config Release
make