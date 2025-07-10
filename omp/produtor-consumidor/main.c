#include "fila.h"
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void produtor(Fila *fila, int *produtores, int rank) {
  FILE *arquivo;
  char nome[15];
  char *linha = NULL, *lido;
  size_t tamanho = 0;

  snprintf(nome, sizeof(nome), "input-%d.txt", rank);
  arquivo = fopen(nome, "rt");

  if (arquivo == NULL) {
    fprintf(stderr, "Erro ao abrir arquivo: %s", nome);

#pragma omp atomic
    (*produtores)--;

    return;
  }

  while (getline(&linha, &tamanho, arquivo) != -1) {
    lido = strdup(linha);
    if (lido != NULL) {
      insere_fila(fila, lido);
    }
  }

  free(linha);
  fclose(arquivo);

#pragma omp atomic
  (*produtores)--;
}

void consumidor(Fila *fila, int *n_tokens, int rank, int *produtores) {
  char *linha = NULL;

  while (linha || *produtores != 0) {
    linha = remove_fila(fila);

    if (linha != NULL) {
      char *token, *ptr;
      token = strtok_r(linha, " \n", &ptr);

      while (token != NULL) {
        printf("Thread %d consumiu o token: %s\n", rank, token);

#pragma omp atomic
        (*n_tokens)++;

        token = strtok_r(NULL, " \n", &ptr);
      }
      free(linha);
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage:\n");
    printf("./run.sh <quantidade_ranks>");
  }

  int n_ranks, n_tokens = 0, produtores, rank;
  Fila *fila;
  n_ranks = atoi(argv[1]);
  fila = cria_fila();

  produtores = (n_ranks / 2) + (n_ranks & 1);

#pragma omp parallel num_threads(n_ranks) default(none)                        \
    shared(produtores, fila, n_tokens) private(rank)
  {
    rank = omp_get_thread_num();

    if (rank < produtores)
      produtor(fila, &produtores, rank);
    else
      consumidor(fila, &n_tokens, rank, &produtores);
  }

  printf("Tokens lidos: %d\n", n_tokens);

  libera_fila(fila);

  return 0;
}
