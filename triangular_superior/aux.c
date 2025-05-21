#include "aux.h"

#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define EPSILON 1e-4

float **fabricar_matriz_triangular_superior(int n) {
  float **A = (float **)malloc(n * sizeof(float *));
  for (int i = 0; i < n; i++) {
    A[i] = (float *)malloc(n * sizeof(float));
    for (int j = 0; j < n; j++) {
      if (i <= j) {
        A[i][j] = rand() % 9 + 1;
      } else {
        A[i][j] = 0.0;
      }
    }
  }
  return A;
}

void imprimir_matriz(float **A, int n) {
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      printf("%5.1f ", A[i][j]);
    }
    printf("\n");
  }
}

void imprimir_vetor(float *v, int n, const char *nome) {
  printf("%s = [", nome);
  for (int i = 0; i < n; i++) {
    printf(" %.2f", v[i]);
  }
  printf(" ]\n");
}

void liberar_matriz(float **A, int n) {
  for (int i = 0; i < n; i++) {
    free(A[i]);
  }
  free(A);
}

void calcular_b(float **A, float *x, float *b, int n) {
  for (int i = 0; i < n; i++) {
    b[i] = 0;
    for (int j = 0; j < n; j++) {
      b[i] += A[i][j] * x[j];
    }
  }
}