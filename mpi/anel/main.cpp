// [Silva et al., 2022] Considere um anel com num_procs processos. Escreva um
// programa utilizando rotinas MPI_Send e MPI_Recv para comunicação entre os
// processos que faça circular uma mensagem contendo um inteiro positivo ao
// longo desse canal. O processo com ranque igual a 0 é quem inicia a
// transmissão e cada vez que a mensagem passa por ele novamente o valor contido
// na mensagem deve ser decrementado de um até chegar ao valor 0. Quando um
// processo receber a mensagem com valor 0 ele deverá passá-la adiante e então
// terminar a sua execução.

#include <iostream>
#include <mpi.h>

int main(int argc, char *argv[]) {
  int size, rank, valor = -1;
  int prev, next;
  MPI_Comm comm = MPI_COMM_WORLD;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(comm, &rank);
  MPI_Comm_size(comm, &size);

  prev = (rank == 0) ? (size - 1) : (rank - 1);
  next = (rank + 1) % size;
  if (rank == 0) {
    valor = 10;
    MPI_Send(&valor, 1, MPI_INT, next, 0, comm);
  }

  while (valor != 0) {
    MPI_Recv(&valor, 1, MPI_INT, prev, 0, comm, MPI_STATUS_IGNORE);

    if (rank == 0) {
      valor--;
      std::cout << "Processo 0 decrementou o valor para " << valor << std::endl;
    } else {
      std::cout << "Processo " << rank << " recebeu valor " << valor
                << std::endl;
    }

    MPI_Send(&valor, 1, MPI_INT, next, 0, comm);
  }

  MPI_Finalize();
  return 0;
}