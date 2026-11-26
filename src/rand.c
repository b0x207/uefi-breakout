/**
 * Based on the version by Sebastiano Vigna (2015).
 */

#include <stdint.h>

#include "rand.h"

typedef struct {
	uint64_t s[4];
} Xoshiro256ssState;

static Xoshiro256ssState STATE;

static uint64_t rol64(uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}

static uint64_t splitmix64(uint64_t seed) {
    uint64_t z = (seed += 0x9e3779b97f4a7c15uLL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9uLL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebuLL;
    return z ^ (z >> 31);
}

static uint64_t xoshiro256ss(Xoshiro256ssState* state) {
	uint64_t* s = state->s;
	const uint64_t result = rol64(s[1] * 5, 7) * 9;
	const uint64_t t = s[1] << 17;

	s[2] ^= s[0];
	s[3] ^= s[1];
	s[1] ^= s[2];
	s[0] ^= s[3];

	s[2] ^= t;
	s[3] = rol64(s[3], 45);

	return result;
}

void srand(uint64_t seed) {
    STATE.s[0] = splitmix64(seed);
    STATE.s[1] = splitmix64(seed);
    STATE.s[2] = splitmix64(seed);
    STATE.s[3] = splitmix64(seed);
}

uint64_t rand() {
    return xoshiro256ss(&STATE);
}
