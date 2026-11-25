#include <efi.h>
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
