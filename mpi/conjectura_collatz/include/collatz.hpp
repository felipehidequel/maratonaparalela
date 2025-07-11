#pragma once

#include <cstdint>

struct CollatzResult {
  uint64_t n;
  uint32_t steps;
};

uint32_t collatz_steps(uint64_t n);