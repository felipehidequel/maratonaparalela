#include <iostream>
#include <mpi.h>

// int reducao_global(MPI_Comm comm, int size, int rank, int &valor) {
//   int soma_local = 0;
//   int salto = 1, div = 2;
//   int parceiro;
//   while (salto < size) {
//     if (rank % salto == 0) {
//       int recebido;
//       MPI_Send(&valor, 1, MPI_INT, rank == 0 ? size - 1 : rank + salto, 0,
//                comm);
//       MPI_Recv(&recebido, 1, MPI_INT, rank + salto, 0, comm,
//       MPI_STATUS_IGNORE);
//     }
//   }
// }

int reducao_global(MPI_Comm comm, int size, int rank, int &valor) {
  int soma_local = valor;
  int salto = 1;
  int parceiro;

  while (salto < size) {
    parceiro = rank ^ salto;

    if (parceiro < size) {

      MPI_Sendrecv(&soma_local, 1, MPI_INT, parceiro, 0, &valor, 1, MPI_INT,
                   parceiro, 0, comm, MPI_STATUS_IGNORE);
      soma_local += valor;
    }
    salto <<= 1;
  }

  return soma_local;
}

int reducao_global_2(MPI_Comm comm, int size, int rank, int &valor) {
  int soma_local = valor;
  int salto, rest;
  int parceiro;

  salto = rest = 1;
  while (rest < size)
    rest <<= 1;
  rest >>= 1;

  parceiro = rank ^ rest;
  if (rank >= rest) {
    MPI_Send(&soma_local, 1, MPI_INT, parceiro, 0, comm);
  }
  if (rank < (size - rest)) {
    MPI_Recv(&valor, 1, MPI_INT, parceiro, 0, comm, MPI_STATUS_IGNORE);
    soma_local += valor;
  }

  while (salto < rest) {
    parceiro = rank ^ salto;

    if (parceiro < rest) {
      MPI_Sendrecv(&soma_local, 1, MPI_INT, parceiro, 0, &valor, 1, MPI_INT,
                   parceiro, 0, comm, MPI_STATUS_IGNORE);
      soma_local += valor;
    }
    salto <<= 1;
  }

  parceiro = rank ^ rest;
  if (rank < (size - rest)) {
    MPI_Send(&soma_local, 1, MPI_INT, parceiro, 0, comm);
  }
  if (rank >= rest) {
    MPI_Recv(&soma_local, 1, MPI_INT, parceiro, 0, comm, MPI_STATUS_IGNORE);
  }

  return soma_local;
}

int main(int argc, char *argv[]) {
  int rank, size, valor, soma;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  valor = rank + 1;

  std::cout << "O Rank " << rank << " tem valor inicial: " << valor
            << std::endl;

  soma = reducao_global_2(MPI_COMM_WORLD, size, rank, valor);

  std::cout << "O Rank " << rank << " tem " << soma << std::endl;

  MPI_Finalize();

  return 0;
}