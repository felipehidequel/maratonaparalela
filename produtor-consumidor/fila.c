#include "fila.h"
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

Fila *cria_fila() {
  Fila *f = (Fila *)malloc(sizeof(Fila));
  f->inicio = NULL;
  f->fim = NULL;
  f->tamanho = 0;
  return f;
}

void insere_fila(Fila *f, char *linha) {
  No *novo = (No *)malloc(sizeof(No));
  novo->linha = linha;
  novo->proximo = NULL;

#pragma omp critical
  {
    if (f->tamanho == 0) {
      f->inicio = novo;
      f->fim = novo;
    } else {
      f->fim->proximo = novo;
      f->fim = novo;
    }
    f->tamanho++;
  }
}

char *remove_fila(Fila *f) {
  No *removido = NULL;
  char *linha = NULL;

#pragma omp critical
  {
    if (f->tamanho != 0) {
      removido = f->inicio;
      linha = removido->linha;

      f->inicio = removido->proximo;
      f->tamanho--;

      if (f->tamanho == 0) {
        f->fim = NULL;
      }
    }
  }

  if (removido != NULL) {
    free(removido);
  }

  return linha;
}

void libera_fila(Fila *f) {
  No *atual = f->inicio;
  while (atual != NULL) {
    No *proximo = atual->proximo;
    free(atual);
    atual = proximo;
  }
  free(f);
}
