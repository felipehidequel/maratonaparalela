#!/bin/bash

CXX=mpirun

make

$CXX ./escalar $1
