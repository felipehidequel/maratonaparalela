#!/bin/bash

set -e

make

export OMP_NUM_THREADS=4
ESCALONADORES="static dynamic guided"
echo ""
for ESCALONADOR in $ESCALONADORES; do
    echo ""
    export OMP_SCHEDULE=$ESCALONADOR
    echo "Executando Colunas com $OMP_NUM_THREADS threads e escalonador $ESCALONADOR"
    ./colunas $1
    echo ""
    echo "Executando Linhas com $OMP_NUM_THREADS threads e escalonador $ESCALONADOR"
    ./linhas $1
done
echo ""

# echo "Executando Colunas com $OMP_NUM_THREADS threads"
# ./colunas $1
# echo ""
# echo "Executando Linhas com $OMP_NUM_THREADS threads"
# ./linhas $1
