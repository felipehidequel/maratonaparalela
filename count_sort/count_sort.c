/*Author: Felipe Hidequel
  Date: 2023-10-02
  Description: Parallel implementation of counting sort using OpenMP.
  Execution: ./run.sh <size>
  Note: The array is filled with random numbers between 0 and 9.
  The array is sorted in ascending order.
*/

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void count_sort(int a[], int n) {
  int i, j, count;
  int *temp = malloc(n * sizeof(int));
  for (i = 0; i < n; i++) {
    count = 0;
    for (j = 0; j < n; j++)
      if (a[j] < a[i])
        count++;
      else if (a[j] == a[i] && j < i)
        count++;
    temp[count] = a[i];
  }
  memcpy(a, temp, n * sizeof(int));
  free(temp);
}

/*  count_sort_parallel - Parallel implementation of counting sort
  @a: array to be sorted
  @n: size of the array
*/
void count_sort_parallel(int a[], int n) {
  int i, j, count;
  int *temp = malloc(n * sizeof(int));
  int rest, id, chunksize;
  int start;

#pragma omp parallel default(none) private(i, j, count, start, id, chunksize)  \
    shared(a, temp, n, rest)
  {
#pragma omp for
    for (i = 0; i < n; i++) {
      count = 0;
      for (j = 0; j < n; j++)
        if (a[j] < a[i])
          count++;
        else if (a[j] == a[i] && j < i)
          count++;
      temp[count] = a[i];
    }

#pragma omp barrier
    rest = n % omp_get_num_threads();
    id = omp_get_thread_num();
    chunksize = n / omp_get_num_threads() + (id < rest);

    start = id * chunksize + (id < rest ? id : rest);
    memcpy(&a[start], &temp[start], chunksize * sizeof(int));
  }

  free(temp);
}

int compara(const void *a, const void *b) {
  int fa = *(const int *)a;
  int fb = *(const int *)b;

  if (fa < fb)
    return -1;
  else if (fa > fb)
    return 1;
  else
    return 0;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: <executavel> <tamanho do vetor>\n");
  }

  double start, end;
  int n = atoi(argv[1]);
  int a[n], b[n], c[n];

  for (int i = 0; i < n; i++) {
    a[i] = rand() % 10;
    b[i] = a[i];
    c[i] = a[i];
  }

  //   printf("Vetor Original\n");
  //   for (int i = 0; i < n; i++) {
  //     printf("%d\t", a[i]);
  //   }
  printf("\n");

  start = omp_get_wtime();
  count_sort_parallel(a, n);
  end = omp_get_wtime();

  printf("Tempo Paralelo: %f\n", end - start);

  start = omp_get_wtime();
  count_sort(b, n);
  end = omp_get_wtime();
  printf("Tempo Serial: %f\n", end - start);

  start = omp_get_wtime();
  qsort(c, n, sizeof(int), compara);
  end = omp_get_wtime();
  printf("Tempo qsort: %f\n", end - start);

  // printf("Vetor Ordenado\n");
  // for (int i = 0; i < n; i++) {
  //   printf("%d\t", a[i]);
  // }
  // printf("\n");

  return 0;
}