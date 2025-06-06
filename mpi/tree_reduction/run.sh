#!/bin/bash

set -e

make

mpirun -n $1 main