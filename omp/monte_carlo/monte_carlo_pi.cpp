#include <iostream>
#include <omp.h>

typedef long long int ll;

double monte_carlo(ll num_lancamentos) {
  ll qtd_no_circulo = 0;
  double distancia_quadrada;
  double x, y;

#pragma omp parallel private(x, y) shared(num_lancamentos)
#pragma omp for reduction(+ : qtd_no_circulo)
  for (ll lancamento = 0; lancamento < num_lancamentos; lancamento++) {

    unsigned rank = omp_get_thread_num();

    x = rand_r(&rank) / (double)RAND_MAX * 2 - 1;
    y = rand_r(&rank) / (double)RAND_MAX * 2 - 1;

    distancia_quadrada = x * x + y * y;

    if (distancia_quadrada <= 1)
      qtd_no_circulo++;
  }

  return 4 * qtd_no_circulo / ((double)num_lancamentos);
}

int main(int argc, char *argv[]) {
  ll num_lancamentos = std::atoi(argv[1]);
  double s, e;
  double saida;

  s = omp_get_wtime();
  saida = monte_carlo(num_lancamentos);
  std::cout << "Estimativa de pi: " << saida << std::endl;
  e = omp_get_wtime();

  std::cout << "Elapsed time: " << e - s << std::endl;
  return 0;
}