#ifndef TIME_H_
#define TIME_H_

#include <efi.h>
#include <stdbool.h>

// A 1 millisecond duration expressed in units of 100 nanoseconds.
// This is useful for calls to `SetTimer`
#define DURATION_1MS 10000

EFI_TIME get_time();
void print_time();

/**
 * Return the delta time in milliseconds. If `overwrite` is true, then also set
 * the value of `since` to the current time.
 *
 * NOTE: the current resolution is from hours down to seconds.
 * TODO: explore ways to improve the resolution
 */
double get_delta(EFI_TIME *since, bool overwrite);

/**
 * Sleep for the given duration in milliseconds.
 */
void sleep(uint64_t duration);

#endif // TIME_H_
