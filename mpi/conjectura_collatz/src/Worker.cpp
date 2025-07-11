#include "Worker.hpp"
#include "collatz.hpp"
#include "utils.hpp"
#include <cstdint>
#include <mpi.h>
#include <omp.h>
#include <vector>

/*utils.hpp*/
// const int MASTER = 0;
// const int TAG_TASK = 1;
// const int TAG_RESULT = 2;
// const int TAG_TERMINATE = 3;
// const uint64_t MAX_N = 100000000;
// const int BLOCK_SIZE = 1000;

void Worker::run() {
  MPI_Request request;
  MPI_Status status;
  std::vector<uint64_t> block(BLOCK_SIZE);
  uint64_t local_n;
  uint32_t max_local_steps;

  while (true) {
    int block_size;
    MPI_Recv(&block_size, 1, MPI_INT, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD,
             &status);
    if (status.MPI_TAG == TAG_TERMINATE)
      break;

    block.resize(block_size);
    MPI_Recv(block.data(), block_size, MPI_UINT64_T, MASTER, TAG_TASK,
             MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    local_n = 0;
    max_local_steps = 0;

#pragma omp parallel
    {
      uint64_t thread_n = 0;
      uint32_t max_thread_steps = 0;

#pragma omp for nowait
      for (int i = 0; i < block_size; i++) {
        uint64_t n = block[i];
        uint32_t steps = collatz_steps(n);

        if (steps > max_thread_steps) {
          thread_n = n;
          max_thread_steps = steps;
        }
      }

#pragma omp critical
      if (max_thread_steps > max_local_steps) {
        local_n = thread_n;
        max_local_steps = max_thread_steps;
      }
    } // end OpeMP

    CollatzResult result = {local_n, max_local_steps};
    MPI_Isend(&result, sizeof(CollatzResult), MPI_BYTE, MASTER, TAG_RESULT,
              MPI_COMM_WORLD, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE);
  } // end while
}
