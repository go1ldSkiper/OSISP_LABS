#!/bin/sh
./generator 4096 file.bin
./sort_index 4096 64 16 file.bin
