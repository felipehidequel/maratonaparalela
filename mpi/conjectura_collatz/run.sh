#!/bin/bash

make

mpirun -n $1 collatz_parallel