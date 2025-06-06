#!/bin/bash

set -e 

make

mpirun -np 4 ./main $1