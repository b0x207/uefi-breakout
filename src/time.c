#include <efi.h>
#include <efierr.h>
#include <efilib.h>

#include "common.h"

EFI_TIME get_time() {
    EFI_TIME t;
    EFI_STATUS status = uefi_call_wrapper(gRT->GetTime, 2, &t, NULL);

    if (EFI_ERROR(status))
        die(L"Failed to call GetTime!", status);

    return t;
}

void print_time() {
    EFI_TIME time = get_time();
    Print(L"%d:%d:%d\n", time.Hour, time.Minute, time.Second);
}

void sleep(uint64_t duration) {
    EFI_EVENT timer = NULL;
    EFI_STATUS status;

    status = uefi_call_wrapper(
        gBS->CreateEvent,
        5,
        EFI_EVENT_TIMER,
        TPL_NOTIFY,
        NULL,
        NULL,
        &timer
    );
    if (EFI_ERROR(status))
        die(L"Failed to create main graphics timer event!", status);

    status = uefi_call_wrapper(
        gBS->SetTimer,
        3,
        timer,
        TimerRelative,
        duration * DURATION_1MS
    );
    if (EFI_ERROR(status))
        die(L"Failed to set main graphics timer duration!", status);

    WaitForSingleEvent(timer, 0);
}

double get_delta(EFI_TIME *since, bool overwrite) {
    if (!since)
        die(L"Cannot call get_delta with NULL for since!", EFI_INVALID_PARAMETER);

    EFI_TIME now = get_time();
    double hours = (now.Hour - since->Hour) * 60.0 * 60.0 * 1000.0;
    double minutes = (now.Minute - since->Minute) * 60.0 * 1000.0;
    double seconds = (now.Second - since->Second) * 1000.0;
    double milliseconds = (now.Nanosecond - since->Nanosecond) * 1000.0;

    if (overwrite)
        *since = now;

    return hours + minutes + seconds + milliseconds;
}
