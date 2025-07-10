#include <cstdlib>
#include <iostream>
#include <mpi.h>
#include <vector>

void block_compute(int displs[], int send_counts[], int n, int size) {
  int base = n / size;
  int rest = n % size;

  for (int rank = 0; rank < size; rank++) {
    displs[rank] = rank * base + std::min(rank, rest);
    send_counts[rank] = base + (rank < rest ? 1 : 0);
  }
}

void cyclic_compute(int displs[], int send_counts[], int n, int size) {
  for (int rank = 0; rank < size; rank++) {
    int count = 0;
    for (int i = rank; i < n; i += size)
      count++;
    send_counts[rank] = count;
    displs[rank] = rank;
  }
}

int get_input(int rank, char *argv[], MPI_Comm comm, int &local_n, int size) {
  int n;
  if (rank == 0) {
    n = std::stoi(argv[1]);
  }
  MPI_Bcast(&n, 1, MPI_INT, 0, comm);
  local_n = n / size + (rank < (n % size));
  return n;
}

int main(int argc, char *argv[]) {
  MPI_Init(&argc, &argv);

  int rank, comm_sz;
  MPI_Comm comm = MPI_COMM_WORLD;
  MPI_Comm_size(comm, &comm_sz);
  MPI_Comm_rank(comm, &rank);

  if (rank == 0 && argc < 2) {
    std::cerr << "Uso: " << argv[0] << " <n>" << std::endl;
    MPI_Abort(comm, -1);
  }

  int local_n;
  int n = get_input(rank, argv, comm, local_n, comm_sz);

  std::vector<int> block_data(local_n);
  for (int i = 0; i < local_n; i++) {
    block_data[i] = rank * 1000 + i;
  }

  std::vector<int> all_data;
  if (rank == 0) {
    all_data.resize(n);
  }

  std::vector<int> block_counts(comm_sz), block_displs(comm_sz);
  block_compute(block_displs.data(), block_counts.data(), n, comm_sz);

  std::vector<int> cyclic_counts(comm_sz), cyclic_displs(comm_sz);
  cyclic_compute(cyclic_displs.data(), cyclic_counts.data(), n, comm_sz);

  double t1 = MPI_Wtime();

  MPI_Gatherv(block_data.data(), local_n, MPI_INT, all_data.data(),
              block_counts.data(), block_displs.data(), MPI_INT, 0, comm);

  std::vector<int> cyclic_data(n);
  if (rank == 0) {
    for (int i = 0; i < n; i++) {
      cyclic_data[i] = all_data[i];
    }
  }

  std::vector<int> local_cyclic(cyclic_counts[rank]);
  MPI_Scatterv(cyclic_data.data(), cyclic_counts.data(), cyclic_displs.data(),
               MPI_INT, local_cyclic.data(), cyclic_counts[rank], MPI_INT, 0,
               comm);

  double t2 = MPI_Wtime();

  MPI_Gatherv(local_cyclic.data(), cyclic_counts[rank], MPI_INT,
              all_data.data(), cyclic_counts.data(), cyclic_displs.data(),
              MPI_INT, 0, comm);

  std::vector<int> block_back(local_n);
  MPI_Scatterv(all_data.data(), block_counts.data(), block_displs.data(),
               MPI_INT, block_back.data(), local_n, MPI_INT, 0, comm);

  double t3 = MPI_Wtime();

  bool data_ok = true;
  for (int i = 0; i < local_n; i++) {
    if (block_back[i] != block_data[i]) {
      std::cout << "Rank " << rank << " ERRO no índice " << i << ": "
                << block_back[i] << " != " << block_data[i] << std::endl;
      data_ok = false;
      break;
    }
  }

  int all_ok;
  MPI_Reduce(&data_ok, &all_ok, 1, MPI_INT, MPI_LAND, 0, comm);

  if (rank == 0 && all_ok)
    std::cout << "Verificação dos dados passou!" << std::endl;

  double cost_bc = t2 - t1;
  double cost_cb = t3 - t2;

  double max_bc, max_cb;
  MPI_Reduce(&cost_bc, &max_bc, 1, MPI_DOUBLE, MPI_MAX, 0, comm);
  MPI_Reduce(&cost_cb, &max_cb, 1, MPI_DOUBLE, MPI_MAX, 0, comm);

  if (rank == 0) {
    std::cout << "Tempo bloco → cíclico: " << max_bc << " s\n";
    std::cout << "Tempo cíclico → bloco: " << max_cb << " s\n";
  }

  MPI_Finalize();
  return 0;
}
