#ifndef RAND_H_
#define RAND_H_

#include <stdint.h>
#include <limits.h>

#define RAND_HALF (UINT64_MAX / 2)

/**
 * A simple integer random number generator based on the xoshiro256** algorithm
 */
uint64_t rand_next();

void seed_rand(uint64_t seed);

#endif // RAND_H_
