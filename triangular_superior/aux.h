#ifndef AUX_H
#define AUX_H

#define EPSILON 1e-4

float **fabricar_matriz_triangular_superior(int n);
void imprimir_matriz(float **A, int n);

void imprimir_vetor(float *v, int n, const char *nome);

void liberar_matriz(float **A, int n);

void calcular_b(float **A, float *x, float *b, int n);

#endif