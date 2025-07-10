/*

Escreva um programa que possa ser usado para determinar o custo de alterar a
distribuição de uma estrutura de dados distribuída. Quanto tempo leva para mudar
de uma distribuição em bloco de um vetor para uma distribuição cíclica? Quanto
tempo leva a redistribuição reversa?

*/

#include <iostream>
#include <mpi.h>

void imprime_vetor(int rank, int *v, int n) {
  std::cout << "Rank " << rank << std::endl;
  for (int i = 0; i < n; i++) {
    std::cout << v[i] << " ";
  }
  std::cout << std::endl;
}

void get_input(int &n, int rank, int size, MPI_Comm comm, int &local_n) {
  if (rank == 0) {
    std::cout << "Informe o valor de N" << std::endl;
    std::cin >> n;
  }

  MPI_Bcast(&n, 1, MPI_INT, 0, comm);
  local_n = (n / size) + (rank < (n % size));
}

int compara(const void *a, const void *b) {
  int fa = *(const int *)a;
  int fb = *(const int *)b;

  return (fa > fb) - (fa < fb);
}

int *merge(int *left, int *right, int n_left, int n_right) {
  int top_left = 0, top_right = 0;
  int n = n_left + n_right;
  int *vetor = new int[n];

  for (int i = 0; i < n; i++) {
    if (top_left >= n_left) {
      vetor[i] = right[top_right++];
    } else if (top_right >= n_right) {
      vetor[i] = left[top_left++];
    } else if (left[top_left] < right[top_right]) {
      vetor[i] = left[top_left++];
    } else {
      vetor[i] = right[top_right++];
    }
  }

  return vetor;
}

int *reduction(int size, int rank, MPI_Comm comm, int *vetor_local, int local_n,
               int n) {
  int salto, parceiro;
  int *vetor_global = nullptr;
  int *aux = vetor_local;

  for (salto = 1; salto < size; salto *= 2) {
    if (rank % salto == 0) {
      if ((rank % (2 * salto)) == 0) {
        parceiro = rank + salto;
        if (parceiro < size) {
          int count_parceiro;
          MPI_Recv(&count_parceiro, 1, MPI_INT, parceiro, 1, comm,
                   MPI_STATUS_IGNORE);

          int *recebido = new int[count_parceiro];
          MPI_Recv(recebido, count_parceiro, MPI_INT, parceiro, 0, comm,
                   MPI_STATUS_IGNORE);

          vetor_global = merge(aux, recebido, local_n, count_parceiro);

          delete[] recebido;
          if (aux != vetor_local)
            delete[] aux;

          aux = vetor_global;
          local_n += count_parceiro;
        }
      } else {
        parceiro = rank - salto;
        MPI_Send(&local_n, 1, MPI_INT, parceiro, 1, comm);
        MPI_Send(aux, local_n, MPI_INT, parceiro, 0, comm);

        if (aux != vetor_local)
          delete[] aux;

        break;
      }
    }
  }

  return (rank == 0) ? aux : nullptr;
}

int main(int argc, char *argv[]) {
  int rank, size, n, local_n;
  MPI_Comm comm = MPI_COMM_WORLD;
  int *vetor;
  int *vetor_local;
  double s, e;
  MPI_Init(&argc, &argv);

  MPI_Barrier(comm);
  s = MPI_Wtime();
  MPI_Comm_rank(comm, &rank);
  MPI_Comm_size(comm, &size);

  srand(rank + time(NULL));

  get_input(n, rank, size, comm, local_n);

  vetor_local = new int[local_n];
  for (int i = 0; i < local_n; i++) {
    vetor_local[i] = rand() % 100;
  }

  qsort(vetor_local, local_n, sizeof(int), compara);
  // MPI_Barrier(comm);

  if (rank != 0) {
    MPI_Send(&local_n, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    MPI_Send(vetor_local, local_n, MPI_INT, 0, 0, MPI_COMM_WORLD);
  }

  int par_n, *par_vec;
  if (rank == 0) {
    std::cout << std::endl;
    std::cout << "Vetores locais ordenados" << std::endl;
    imprime_vetor(rank, vetor_local, local_n);

    for (int r = 1; r < size; r++) {
      MPI_Recv(&par_n, 1, MPI_INT, r, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      par_vec = new int[par_n];
      MPI_Recv(par_vec, par_n, MPI_INT, r, 0, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);

      imprime_vetor(r, par_vec, par_n);
      delete[] par_vec;
    }
  }

  vetor = reduction(size, rank, comm, vetor_local, local_n, n);

  if (rank == 0) {
    std::cout << "Vetores mesclados" << std::endl;
    imprime_vetor(0, vetor, n);
    delete[] vetor;
  }

  delete[] vetor_local;

  e = MPI_Wtime();
  double local_elapsed = e - s;
  double elpased;

  MPI_Reduce(&local_elapsed, &elpased, 1, MPI_DOUBLE, MPI_MAX, 0, comm);
  MPI_Finalize();

  return 0;
}
