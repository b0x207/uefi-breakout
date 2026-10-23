#include <efi.h>
#include <efierr.h>
#include <efilib.h>
#include <stdbool.h>
#include <wchar.h>

#include "common.h"
#include "efiprot.h"
#include "graphics.h"

#define STATUS_N_SET 35
#define STATUS_N_CLEAR 8

void print_efi_info();
void set_debug_marker(EFI_HANDLE image_handle);
EFI_STATUS read_key(EFI_INPUT_KEY *key);

EFI_STATUS EFIAPI efi_main(
    EFI_HANDLE image_handle,
    EFI_SYSTEM_TABLE *system_table
) {
    InitializeLib(image_handle, system_table);
    set_debug_marker(image_handle);
    print_efi_info();

    EFI_STATUS status;

    Print(L"Welcome to UEFI Breakout!\n");
    Print(L"Press any key to begin (or q to quit).\n");

    EFI_INPUT_KEY key;
    status = read_key(&key);
    if (EFI_ERROR(status))
        die(L"Failed to read key!", status);
    if (key.UnicodeChar == L'q')
        return EFI_SUCCESS;

    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    status = acquire_gop(&gop);
    if (EFI_ERROR(status))
        die(L"Failed to setup GOP!", status);

    set_graphics_context(gop);

    Print(
        L"Graphics Initialized: %upx by %upx display\n",
        graphics_width(),
        graphics_height()
    );

    // Draw a square
    graphics_rect(0, 0, 500, 500, 0xFF008800);
    graphics_rect(100, 100, 100, 20, 0x00FF8800);
    graphics_rect(300, 100, 100, 20, 0x00FF8800);
    graphics_rect(100, 300, 300, 20, 0x00FF8800);

    status = read_key(&key);
    if (EFI_ERROR(status))
        die(L"Failed to read key!", status);

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
    if (!graphics_context_valid())
        Print(L"Entering halt & catch fire...\n");
    while(1);
}

void die(const wchar_t *msg, EFI_STATUS status) {
    Print(L"%s ERR: %r\n", msg, status);
    hcf();
}

void die_statusless(const wchar_t *msg) {
    Print(L"%s\n", msg);
    hcf();
}

void print_efi_info() {
    Print(
        L"Running on UEFI from %s at revision 0x%x\n",
        gST->FirmwareVendor,
        gST->FirmwareRevision
    );
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
