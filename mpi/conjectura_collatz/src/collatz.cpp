// Se N for par o seu sucessor
// será a metade e se N for ímpar o seu sucessor será o triplo mais um, gerando
// então um novo número. Esse processo repete-se até que eventualmente se atinja
// o número 1.

#include "../include/collatz.hpp"

#include <unordered_map>

uint32_t collatz_steps(uint64_t n) {
    uint32_t steps = 0;
    while (n != 1) {
        n = (n % 2 == 0) ? n / 2 : 3 * n + 1;
        ++steps;
    }
    return steps;
}
