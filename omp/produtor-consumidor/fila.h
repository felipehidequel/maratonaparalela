// Fila.h
#ifndef FILA_H
#define FILA_H

typedef struct no {
  char *linha;
  struct no *proximo;
} No;

typedef struct fila {
  No *inicio;
  No *fim;
  int tamanho;
} Fila;

Fila *cria_fila();

void insere_fila(Fila *f, char *linha);

char *remove_fila(Fila *f);

void libera_fila(Fila *f);

#endif // FILA_H