#ifndef COMMON_H_
#define COMMON_H_

#include <efi.h>
#include <wchar.h>

#include "time.h"

void hcf();
void die(const wchar_t *msg, EFI_STATUS status);
void die_statusless(const wchar_t *msg);

float clamp_f(float min, float max, float value);

#endif // COMMON_H_
