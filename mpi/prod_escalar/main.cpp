// Escreva um programa para calcular o produto escalar de dois vetores.
// Utilize rotinas MPI_Send e MPI_Recv para comunicação entre os processos.
// Considere cada vetor com N posições e divida a operação entre P processos
// distintos. Considere que a divisão de N por P não tem resto

#include <iostream>
#include <memory>
#include <mpi.h>
#include <vector>

typedef long long int ll;

void usage() { std::cout << "<executavel> <tamanho do vetor>" << std::endl; }

void getInput(size_t &n, int rank, int size, MPI_Comm comm, int argc,
              char *argv[]) {
  if (rank == 0) {
    if (argc < 2) {
      usage();
      MPI_Abort(comm, -1);
    }

    n = std::atoi(argv[1]);

    if (n % size != 0) {
      std::cerr << "N não divisivel por P" << std::endl;
      MPI_Abort(comm, -1);
    }
  }

  MPI_Bcast(&n, 1, MPI_INT, 0, comm);
}

ll reduction(int size, int rank, MPI_Comm comm, int local_result) {
  ll global_result = 0;

  ll local_ll_result = local_result;

  MPI_Reduce(&local_ll_result, &global_result, 1, MPI_LONG_LONG, MPI_SUM, 0,
             comm);

  return global_result;
}

int main(int argc, char *argv[]) {
  int size, rank;
  size_t local_n, n;
  ll local_result = 0, global_result = 0;
  size_t my_first_i, my_last_i;
  ll prova = 0;
  double start, end, local_time, global_time;
  MPI_Comm comm = MPI_COMM_WORLD;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(comm, &rank);
  MPI_Comm_size(comm, &size);

  MPI_Barrier(comm);
  start = MPI_Wtime();

  getInput(n, rank, size, comm, argc, argv);
  local_n = n / size;

  std::unique_ptr<int[]> vec_a(new int[n]());
  std::unique_ptr<int[]> vec_b(new int[n]());

  for (size_t i = 0; i < n; i++) {
    vec_a[i] = i + 100;
    vec_b[i] = i + 100;
    prova += vec_a[i] * vec_b[i];
  }

  my_first_i = rank * local_n;
  my_last_i = my_first_i + local_n;
  for (size_t i = my_first_i; i < my_last_i; i++) {
    local_result += vec_a[i] * vec_b[i];
  }

  global_result = reduction(size, rank, comm, local_result);

  if (rank == 0) {
    std::cout << "Produto Escalar: " << global_result << std::endl;
#ifdef DEBUG
    std::cout << "Resultado esperado: " << prova << std::endl;
#endif
  }

  end = MPI_Wtime();
  local_time = end - start;
  MPI_Reduce(&local_time, &global_time, 1, MPI_DOUBLE, MPI_MAX, 0, comm);

  if (rank == 0) {
    std::cout << "Elapsed Time: " << global_time << std::endl;
  }

  MPI_Finalize();
  return 0;
}