#!/bin/bash

set -e

make

export OMP_NUM_THREADS=4

./main.o $1 >>log.out
