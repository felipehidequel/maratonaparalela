#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "aux.h"

/* Resolve o sistema triangular superior Ax = b sequencial
   A: matriz triangular superior
   b: lado direito do sistema
   x: vetor solução
*/
float *triangular_superior_linhas(float **A, float *B, int n) {
  float *x = (float *)malloc(n * sizeof(float));
  for (int lin = n - 1; lin >= 0; lin--) {
    x[lin] = B[lin];
    for (int col = lin + 1; col < n; col++) {
      x[lin] -= A[lin][col] * x[col];
    }
    x[lin] /= A[lin][lin];
  }
  return x;
}

/* Resolve o sistema triangular superior Ax = b paralelo
   A: matriz triangular superior
   b: lado direito do sistema
   x: vetor solução
*/
float *triangular_superior_linhas_p(float **A, float *B, int n) {
  float *x = (float *)malloc(n * sizeof(float));
  int col, lin;
  float soma;

  for (lin = n - 1; lin >= 0; lin--) {
    soma = 0.0F;

#pragma omp parallel for default(none) reduction(+ : soma) shared(A,B,x,n,lin) private(col) schedule(runtime)
    for (col = lin + 1; col < n; col++) {
      soma += A[lin][col] * x[col];
    }

    x[lin] = (B[lin] - soma) / A[lin][lin];
  }

  return x;
}

int main(int argc, char *argv[]) {
  int n;

  if (argc != 2) {
    printf("Uso: %s <n>\n", argv[0]);
    return 1;
  }

  n = atoi(argv[1]);
  if (n <= 0) {
    printf("A ordem da matriz deve ser maior que zero.\n");
    return 1;
  }

  srand(42);

  float **A = fabricar_matriz_triangular_superior(n);
  float **A_copy = fabricar_matriz_triangular_superior(n);
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      // A_copy[i * n + j] = A[i][j];
      A_copy[i][j] = A[i][j];
    }
  }

  float *x_esperado = (float *)malloc(n * sizeof(float));
  for (int i = 0; i < n; i++) {
    x_esperado[i] = rand() % 9 + 1;
  }

  float *b = (float *)malloc(n * sizeof(float));
  calcular_b(A, x_esperado, b, n);

  double start_time = omp_get_wtime();
  float *x_obtido = triangular_superior_linhas(A, b, n);
  double end_time = omp_get_wtime();

  double start_time_p = omp_get_wtime();
  float *x_obtido_p = triangular_superior_linhas_p(A_copy, b, n);
  double end_time_p = omp_get_wtime();

  // printf("\nMatriz A (triangular superior):\n");
  // imprimir_matriz(A, n);
  // imprimir_vetor(x_esperado, n, "\nx_esperado");
  // imprimir_vetor(b, n, "b");
  // imprimir_vetor(x_obtido, n, "x_obtido");
  printf("Tempo de execução: %f segundos\n", end_time - start_time);
  printf("Tempo de execução paralelo: %f segundos\n",
         end_time_p - start_time_p);

  int ok = 1;
  for (int i = 0; i < n; i++) {
    if (fabs(x_esperado[i] - x_obtido_p[i]) > EPSILON) {
      ok = 0;
      break;
    }
  }
  printf("\nResultado: %s\n", ok ? "CORRETO" : "INCORRETO");

  // Liberação
  liberar_matriz(A, n);
  liberar_matriz(A_copy, n);
  free(x_esperado);
  free(b);
  free(x_obtido);

  return 0;
}
