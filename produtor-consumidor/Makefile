all: main

fila.o: fila.c fila.h
	gcc -Wall -fopenmp -c fila.c -o fila.o

main: main.c fila.o
	gcc -O3 -fopenmp main.c fila.o -o main

clean:
	rm -f *.o main
