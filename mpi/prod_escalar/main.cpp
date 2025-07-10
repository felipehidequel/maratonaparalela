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
  int salto = 1;
  int parceiro;
  int valor;
  ll global_result = local_result;
  while (salto < size) {
    parceiro = rank ^ salto;
    if ((rank & salto) == 0) {
      if (parceiro < size) {
        MPI_Recv(&valor, 1, MPI_INT, parceiro, 0, comm, MPI_STATUS_IGNORE);
        global_result += valor;
      }
    } else {
      MPI_Send(&local_result, 1, MPI_INT, parceiro, 0, comm);
    }
    salto <<= 1;
  }

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