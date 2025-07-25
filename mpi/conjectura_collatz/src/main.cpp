// [Silva et al., 2022] A Conjectura de Collatz foi desenvolvida pelo matemático
// alemão Lothar Collatz. Nela, escolhendo-se um número natural inicial N, onde
// N > 0, os seguintes critérios serão obedecidos: Se N for par o seu sucessor
// será a metade e se N for ímpar o seu sucessor será o triplo mais um, gerando
// então um novo número. Esse processo repete-se até que eventualmente se atinja
// o número 1. Use o modelo “saco de tarefa” para resolver esse problema em
// paralelo e verificar se os números inteiros no intervalo de 1 a 100.000.000
// atendem a esses critérios e o maior número de passos necessários para chegar
// até o número 1. Atribua a um dos processos o papel de “mestre” (gerencia a
// distribuição de tarefas) e aos demais processos o papel de “trabalhadores”
// (executam a tarefa de avaliar se um número está dentro da conjectura).
// Utilize mensagens de envio e recepção não bloqueantes de forma a paralelizar
// a computação com o envio de mensagens.

#include "Master.hpp"
#include "Worker.hpp"
#include <iostream>
#include <mpi.h>

int main(int argc, char **argv) {
  double start, end;

  MPI_Init(&argc, &argv);
  MPI_Barrier(MPI_COMM_WORLD);
  start = MPI_Wtime();

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (rank == 0) {
    Master m(size);
    m.run();
  } else {
    Worker w;
    w.run();
  }

  end = MPI_Wtime();
  double local_time = end - start;
  double final_time;

  MPI_Reduce(&local_time, &final_time, 1, MPI_DOUBLE, MPI_MAX, 0,
             MPI_COMM_WORLD);

  if (rank == 0)
    std::cout << "ELapsed Time: " << final_time << std::endl;

  MPI_Finalize();
  return 0;
}
