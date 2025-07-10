all: linhas colunas

aux.o: aux.c
	gcc -Wall -fopenmp -c aux.c -o aux.o

linhas: linhas.c aux.o
	gcc -Wall -fopenmp linhas.c aux.o -o linhas

colunas: colunas.c aux.o
	gcc -Wall -fopenmp colunas.c aux.o -o colunas

clean:
	rm -f *.o linhas colunas
