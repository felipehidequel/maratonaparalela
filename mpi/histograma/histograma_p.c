/* File:      histogram.c
 * Purpose:   Build a histogram from some random data using MPI and OpenMP.
 *
 * Compile:   mpicc -g -Wall -fopenmp -o histogram_p histograma_p.c
 * Run:       mpirun -n <number_of_processes> ./histogram_p <bin_count>
 * <min_meas> <max_meas> <data_count>
 *
 * Input:     None
 * Output:    A histogram with X's showing the number of measurements
 * in each bin
 *
 * Notes:
 * 1.  Actual measurements y are in the range min_meas <= y < max_meas
 * 2.  bin_counts[i] stores the number of measurements x in the range
 * 3.  bin_maxes[i-1] <= x < bin_maxes[i] (bin_maxes[-1] = min_meas)
 * 4.  DEBUG compile flag gives verbose output
 * 5.  The program will terminate if either the number of command line
 * arguments is incorrect or if the search for a bin for a
 * measurement fails.
 *
 * IPP:  Section 2.7.1 (pp. 66 and ff.)
 */
#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

void Usage(char prog_name[]);

void Get_args(char *argv[] /* in  */, int size /* in */, int rank /* in */,
              int *bin_count_p /* out */, float *min_meas_p /* out */,
              float *max_meas_p /* out */, int *data_count_p /* out */,
              int *local_data_count /* out */);

void Gen_data(float min_meas /* in  */, float max_meas /* in  */,
              float data[] /* out */, int data_count /* in  */);

void Gen_bins(float min_meas /* in  */, float max_meas /* in  */,
              float bin_maxes[] /* out */, int bin_counts[] /* out */,
              int bin_count /* in  */);

int Which_bin(float data /* in */, float bin_maxes[] /* in */,
              int bin_count /* in */, float min_meas /* in */);

void Print_histo(float bin_maxes[] /* in */, int bin_counts[] /* in */,
                 int bin_count /* in */, float min_meas /* in */);

int main(int argc, char *argv[]) {
  int bin_count, i;
  float min_meas, max_meas;
  float *bin_maxes;
  int *local_bin_counts, *global_bin_counts;
  int data_count, local_data_count;
  float *data, *local_data;
  MPI_Comm comm = MPI_COMM_WORLD;
  int size, rank;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(comm, &size);
  MPI_Comm_rank(comm, &rank);

  if (rank == 0) {
    if (argc != 5) {
      Usage(argv[0]);
    }
  }

  MPI_Barrier(comm);
  if (argc != 5) {
    MPI_Finalize();
    exit(-1);
  }

  Get_args(argv, size, rank, &bin_count, &min_meas, &max_meas, &data_count,
           &local_data_count);

  bin_maxes = malloc(bin_count * sizeof(float));
  local_bin_counts = malloc(bin_count * sizeof(int));
  local_data = malloc(local_data_count * sizeof(float));

  if (rank == 0) {
    data = malloc(data_count * sizeof(float));
    Gen_data(min_meas, max_meas, data, data_count);
    global_bin_counts = malloc(bin_count * sizeof(int));
  }

  int *sendcounts = malloc(size * sizeof(int));
  int *displs = malloc(size * sizeof(int));
  int rest = data_count % size;
  int sum = 0;
  for (int i = 0; i < size; i++) {
    sendcounts[i] = data_count / size;
    if (i < rest)
      sendcounts[i]++;
    displs[i] = sum;
    sum += sendcounts[i];
  }

  MPI_Scatterv(data, sendcounts, displs, MPI_FLOAT, local_data,
               local_data_count, MPI_FLOAT, 0, comm);

  Gen_bins(min_meas, max_meas, bin_maxes, local_bin_counts, bin_count);

#pragma omp parallel for reduction(+ : local_bin_counts[:bin_count])
  for (i = 0; i < local_data_count; i++) {
    int bin = Which_bin(local_data[i], bin_maxes, bin_count, min_meas);
    local_bin_counts[bin]++;
  }

  MPI_Reduce(local_bin_counts, global_bin_counts, bin_count, MPI_INT, MPI_SUM,
             0, comm);

  if (rank == 0)
    Print_histo(bin_maxes, global_bin_counts, bin_count, min_meas);

  free(bin_maxes);
  free(local_data);
  free(local_bin_counts);
  if (rank == 0) {
    free(global_bin_counts);
    free(data);
  }

  free(sendcounts);
  free(displs);

  MPI_Finalize();

  return 0;
}

/*---------------------------------------------------------------------
 * Function:  Usage
 * Purpose:   Print a message showing how to run program and quit
 * In arg:    prog_name:  the name of the program from the command line
 */
void Usage(char prog_name[] /* in */) {
  fprintf(stderr, "usage: mpirun -n <p> %s ", prog_name);
  fprintf(stderr, "<bin_count> <min_meas> <max_meas> <data_count>\n");
} /* Usage */

/*---------------------------------------------------------------------
 * Function:  Get_args
 * Purpose:   Get the command line arguments
 * In arg:    argv:  strings from command line
 * Out args:  bin_count_p:   number of bins
 * min_meas_p:    minimum measurement
 * max_meas_p:    maximum measurement
 * data_count_p:  number of measurements
 */
void Get_args(char *argv[] /* in  */, int size /* in */, int rank /* in */,
              int *bin_count_p /* out */, float *min_meas_p /* out */,
              float *max_meas_p /* out */, int *data_count_p /* out */,
              int *local_data_count /* out */) {

  *bin_count_p = strtol(argv[1], NULL, 10);
  *min_meas_p = strtof(argv[2], NULL);
  *max_meas_p = strtof(argv[3], NULL);
  *data_count_p = strtol(argv[4], NULL, 10);

  int rest = *data_count_p % size;
  *local_data_count = *data_count_p / size;
  if (rank < rest)
    (*local_data_count)++;

#ifdef DEBUG
  if (rank == 0) {
    printf("bin_count = %d\n", *bin_count_p);
    printf("min_meas = %f, max_meas = %f\n", *min_meas_p, *max_meas_p);
    printf("data_count = %d\n", *data_count_p);
  }
#endif
} /* Get_args */

/*---------------------------------------------------------------------
 * Function:  Gen_data
 * Purpose:   Generate random floats in the range min_meas <= x < max_meas
 * In args:   min_meas:    the minimum possible value for the data
 * max_meas:    the maximum possible value for the data
 * data_count:  the number of measurements
 * Out arg:   data:        the actual measurements
 */
void Gen_data(float min_meas /* in  */, float max_meas /* in  */,
              float data[] /* out */, int data_count /* in  */) {
  int i;

  srandom(0);
  for (i = 0; i < data_count; i++)
    data[i] = min_meas + (max_meas - min_meas) * random() / ((double)RAND_MAX);

#ifdef DEBUG
  printf("data = ");
  for (i = 0; i < data_count; i++)
    printf("%4.3f ", data[i]);
  printf("\n");
#endif
} /* Gen_data */

/*---------------------------------------------------------------------
 * Function:  Gen_bins
 * Purpose:   Compute max value for each bin, and store 0 as the
 * number of values in each bin
 * In args:   min_meas:   the minimum possible measurement
 * max_meas:   the maximum possible measurement
 * bin_count:  the number of bins
 * Out args:  bin_maxes:  the maximum possible value for each bin
 * bin_counts: the number of data values in each bin
 */
void Gen_bins(float min_meas /* in  */, float max_meas /* in  */,
              float bin_maxes[] /* out */, int bin_counts[] /* out */,
              int bin_count /* in  */) {
  float bin_width;
  int i;

  bin_width = (max_meas - min_meas) / bin_count;

  for (i = 0; i < bin_count; i++) {
    bin_maxes[i] = min_meas + (i + 1) * bin_width;
    bin_counts[i] = 0;
  }

#ifdef DEBUG
  if (omp_get_thread_num() == 0) {
    printf("bin_maxes = ");
    for (i = 0; i < bin_count; i++)
      printf("%4.3f ", bin_maxes[i]);
    printf("\n");
  }
#endif
} /* Gen_bins */

/*---------------------------------------------------------------------
 * Function:  Which_bin
 * Purpose:   Use binary search to determine which bin a measurement
 * belongs to
 * In args:   data:       the current measurement
 * bin_maxes:  list of max bin values
 * bin_count:  number of bins
 * min_meas:   the minimum possible measurement
 * Return:    the number of the bin to which data belongs
 * Notes:
 * 1.  The bin to which data belongs satisfies
 *
 * bin_maxes[i-1] <= data < bin_maxes[i]
 *
 * where, bin_maxes[-1] = min_meas
 * 2.  If the search fails, the function prints a message and exits
 */
int Which_bin(float data /* in */, float bin_maxes[] /* in */,
              int bin_count /* in */, float min_meas /* in */) {
  int bottom = 0, top = bin_count - 1;
  int mid;
  float bin_max, bin_min;

  while (bottom <= top) {
    mid = (bottom + top) / 2;
    bin_max = bin_maxes[mid];
    bin_min = (mid == 0) ? min_meas : bin_maxes[mid - 1];
    if (data >= bin_max)
      bottom = mid + 1;
    else if (data < bin_min)
      top = mid - 1;
    else
      return mid;
  }

  fprintf(stderr, "Data = %f doesn't belong to a bin!\n", data);
  fprintf(stderr, "Quitting\n");
  MPI_Abort(MPI_COMM_WORLD, -1);
  return -1;
} /* Which_bin */

/*---------------------------------------------------------------------
 * Function:  Print_histo
 * Purpose:   Print a histogram.  The number of elements in each
 * bin is shown by an array of X's.
 * In args:   bin_maxes:   the max value for each bin
 * bin_counts:  the number of elements in each bin
 * bin_count:   the number of bins
 * min_meas:    the minimum possible measurment
 */
void Print_histo(float bin_maxes[] /* in */, int bin_counts[] /* in */,
                 int bin_count /* in */, float min_meas /* in */) {
  int i, j;
  float bin_max, bin_min;

  for (i = 0; i < bin_count; i++) {
    bin_max = bin_maxes[i];
    bin_min = (i == 0) ? min_meas : bin_maxes[i - 1];
    printf("%.3f-%.3f:\t", bin_min, bin_max);
    for (j = 0; j < bin_counts[i]; j++)
      printf("X");
    printf("\n");
  }
} /* Print_histo */