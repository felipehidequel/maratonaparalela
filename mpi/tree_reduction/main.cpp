/*
Escreva um programa MPI que calcule uma soma global estruturada em árvore.
Primeiro, escreva seu programa para o caso especial em que comm_sz é uma
potência de dois. Depois que esta versão estiver funcionando, modifique seu
programa para que ele possa lidar com qualquer comm_sz.
*/
#include <iostream>
#include <mpi.h>

/*
    div = 2
    salto = 1
    parceiro;

    for(salto; salto < size; salto*=2)
    {
        if(rank%salto==0){
            if(rank%div==0){
                parceiro=rank+1;
                receber soma local;
            }else{
                parceiro = rank-1;
                enviar soma;
            }
        }
        div*=2;
    }

*/
int reduction(int size, int rank, MPI_Comm comm, int &soma_local) {
  int div = 2;
  int salto = 1;
  int parceiro;
  int soma_global = soma_local;

  for (salto; salto < size; salto *= 2) {
    if (rank % salto == 0) {
      if (rank % div == 0) {
        parceiro = rank + salto;
        if (parceiro < size) {
          int recebido;
          MPI_Recv(&recebido, 1, MPI_INT, parceiro, 0, comm, MPI_STATUS_IGNORE);
          soma_global += recebido;
        }
      } else {
        parceiro = rank - salto;
        MPI_Send(&soma_global, 1, MPI_INT, parceiro, 0, comm);
      }
    }
    div <<= 1;
  }
  return soma_global;
}

int main(int argc, char *argv[]) {
  int comm_sz, rank;
  int valor_local;
  int soma_global;
  double start, end, local_time, global_time;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

  valor_local = rank + 1;

  std::cout << "Eu sou o rank " << rank << " e vou somar " << valor_local
            << std::endl;

  MPI_Barrier(MPI_COMM_WORLD);
  start = MPI_Wtime();
  soma_global = reduction(comm_sz, rank, MPI_COMM_WORLD, valor_local);
  end = MPI_Wtime();

  local_time = end - start;

  MPI_Reduce(&local_time, &global_time, 1, MPI_DOUBLE, MPI_MAX, 0,
             MPI_COMM_WORLD);

  if (rank == 0) {
    std::cout << "Soma Global: " << soma_global << std::endl;
    std::cout << "Tempo Decorrido: " << global_time << std::endl;
  }

  MPI_Finalize();
}