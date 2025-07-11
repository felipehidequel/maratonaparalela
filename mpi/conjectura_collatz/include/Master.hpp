// Use o modelo “saco de tarefa” para resolver esse problema em
// paralelo e verificar se os números inteiros no intervalo de 1 a 100.000.000
// atendem a esses critérios e o maior número de passos necessários para chegar
// até o número 1. Atribua a um dos processos o papel de “mestre” (gerencia a
// distribuição de tarefas) e aos demais processos o papel de “trabalhadores”
// (executam a tarefa de avaliar se um número está dentro da conjectura).
// Utilize mensagens de envio e recepção não bloqueantes de forma a paralelizar
// a computação com o envio de mensagens.
#pragma once
#include <cstdint>

class Master {
private:
  int num_procs;
  uint32_t max_steps;
  uint64_t next_n;
  uint64_t max_n;

  void send_block(int dest);
  void send_terminate(int dest);

public:
  void run();
  Master(int num_procs);
};