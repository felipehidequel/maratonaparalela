#!/bin/bash

make

mpirun -n 4 main $1 