#include <efi.h>
#include <efilib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>

#define STATUS_N_SET 35
#define STATUS_N_CLEAR 8

void set_debug_marker(EFI_HANDLE image_handle);
void hcf();
void die(const wchar_t *msg, EFI_STATUS status);
EFI_STATUS read_key(EFI_INPUT_KEY *key);

EFI_STATUS EFIAPI efi_main(
    EFI_HANDLE image_handle,
    EFI_SYSTEM_TABLE *system_table
) {
    InitializeLib(image_handle, system_table);
    set_debug_marker(image_handle);

    EFI_STATUS status;

    Print(L"Welcome to UEFI Breakout!\n");
    Print(L"Press any key to begin.\n");

    EFI_INPUT_KEY key;
    status = read_key(&key);
    if (EFI_ERROR(status)) {
        die(L"Failed to read key!", status);
    }

    Print(L"Type a line and I'll echo it back\n");
    wchar_t buf[256];
    wchar_t end[2] = L"\n";
    memset(buf, 0, sizeof(wchar_t) * 256);
    int i = 0;

    do {
        status = read_key(&key);
        if (EFI_ERROR(status))
            die(L"Failed to read key!", status);

        buf[i++] = key.UnicodeChar;
        Print(L"%c", key.UnicodeChar);
    } while (key.UnicodeChar != CHAR_CARRIAGE_RETURN);

    Print(L"\n> %s\n", buf);

    hcf();
    return EFI_SUCCESS;
}

#ifdef DEBUG_ENABLED
void set_debug_marker(EFI_HANDLE image_handle) {
    EFI_LOADED_IMAGE loaded_image;
    EFI_STATUS status = gBS->HandleProtocol(
        image_handle,
        &LoadedImageProtocol,
        (void**) &loaded_image
    );

    // Set a basic marker for debugging to pick up
    volatile uint64_t *marker_ptr = (uint64_t *)0x10000;
    volatile uint64_t *image_base_ptr = (uint64_t *)0x10008;
    *image_base_ptr = (uint64_t)loaded_image.ImageBase;
    *marker_ptr = 0xDEADBEEF;
}
#else
void set_debug_marker(EFI_HANDLE image_handle) {}
#endif

void hcf() {
    Print(L"Entering halt & catch fire...\n");
    while(1);
}

void die(const wchar_t *msg, EFI_STATUS status) {
    Print(L"%s ERR: %r\n", msg, status);

    hcf();
}

/**
 * Read a single key.
 *
 * Note: there is no timeout.
 *
 * @returns EFI_SUCCESS unless there is an error in which case, the error is
 * returned to the caller.
 */
EFI_STATUS read_key(EFI_INPUT_KEY *key) {
    EFI_STATUS status;

    status = WaitForSingleEvent(gST->ConIn->WaitForKey, 0);
    if (EFI_ERROR(status))
        return status;

    status = uefi_call_wrapper(gST->ConIn->ReadKeyStroke, 2, gST->ConIn, key);

    return status;
}
