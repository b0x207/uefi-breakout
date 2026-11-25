#ifndef COMMON_H_
#define COMMON_H_

#include <efi.h>
#include <wchar.h>

#include "time.h"

void hcf();
void die(const wchar_t *msg, EFI_STATUS status);
void die_statusless(const wchar_t *msg);

#endif // COMMON_H_
