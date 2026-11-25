#ifndef COMMON_H_
#define COMMON_H_

#include <efi.h>
#include <wchar.h>

// A 1 millisecond duration expressed in units of 100 nanoseconds.
// This is useful for calls to `SetTimer`
#define DURATION_1MS 10000

void hcf();
void die(const wchar_t *msg, EFI_STATUS status);
void die_statusless(const wchar_t *msg);
EFI_TIME get_time();
void print_time();

/**
 * Sleep for the given duration in milliseconds.
 */
void sleep(uint64_t duration);

#endif // COMMON_H_
