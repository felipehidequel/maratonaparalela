#include "Master.hpp"
#include "collatz.hpp"
#include "utils.hpp"
#include <cstdint>
#include <iostream>
#include <mpi.h>
#include <vector>
// const int TAG_TASK = 1;
// const int TAG_RESULT = 2;
// const int TAG_TERMINATE = 3;
// const uint64_t MAX_N = 100000000;
// const int BLOCK_SIZE = 1000;

Master::Master(int num_procs)
    : num_procs(num_procs), max_steps(0), next_n(1), max_n(0) {}

void Master::run() {
  int num_workers = num_procs - 1;
  std::vector<MPI_Request> requests(num_workers);
  std::vector<CollatzResult> results(num_workers);
  std::vector<int> sources(num_workers);
  int tasks_in_flight = 0;

  for (int i = 1; i < num_procs && next_n <= MAX_N; ++i) {
    send_block(i);
    MPI_Irecv(&results[i - 1], sizeof(CollatzResult), MPI_BYTE, i, TAG_RESULT,
              MPI_COMM_WORLD, &requests[i - 1]);
    sources[i - 1] = i;
    tasks_in_flight++;
  }

  while (tasks_in_flight > 0) {
    int index;
    MPI_Waitany(num_workers, requests.data(), &index, MPI_STATUS_IGNORE);

    int source = sources[index];
    CollatzResult &result = results[index];

    if (result.steps > max_steps) {
      max_steps = result.steps;
      max_n = result.n;
    }

    if (next_n <= MAX_N) {
      send_block(source);
      MPI_Irecv(&results[index], sizeof(CollatzResult), MPI_BYTE, source,
                TAG_RESULT, MPI_COMM_WORLD, &requests[index]);
    } else {
      send_terminate(source);
      requests[index] = MPI_REQUEST_NULL;
      tasks_in_flight--;
    }
  }

  std::cout << "Número com mais passos: " << max_n << " (" << max_steps
            << " passos)\n";
}

void Master::send_block(int dest) {
  std::vector<uint64_t> block;
  for (int i = 0; i < BLOCK_SIZE && next_n <= MAX_N; i++, next_n++) {
    block.push_back(next_n);
  }

  int size = block.size();

  MPI_Send(&size, 1, MPI_INT, dest, TAG_TASK, MPI_COMM_WORLD);
  MPI_Send(block.data(), size, MPI_UINT64_T, dest, TAG_TASK, MPI_COMM_WORLD);
}

void Master::send_terminate(int dest) {
  int dummy = 0;
  MPI_Send(&dummy, 1, MPI_INT, dest, TAG_TERMINATE, MPI_COMM_WORLD);
}