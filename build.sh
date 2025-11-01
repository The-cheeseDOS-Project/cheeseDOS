#!/bin/sh

CC = gcc
# add others...

gcc src/build/build.c -o build -Ofast -march=native -mtune=native -g0 -std=c99 # add more flags...
./build
