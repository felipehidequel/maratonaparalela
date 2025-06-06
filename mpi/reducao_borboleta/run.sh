#!/bin/bash

set -e

make

if [ -f "hostfile" ]; then
    mpirun --hostfile hostfile -n $1 ./main
else
    mpirun -n $1 ./main
fi
