#!/bin/bash

if [ -f "anel" ]; then
  make clean
fi

make 

mpirun anel