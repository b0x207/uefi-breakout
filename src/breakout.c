#include <efi.h>
#include <efierr.h>
#include <efilib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <wchar.h>

#include "common.h"
#include "graphics.h"

#define STATUS_N_SET 35
#define STATUS_N_CLEAR 8

void print_efi_info();
void set_debug_marker(EFI_HANDLE image_handle);
EFI_STATUS read_key(EFI_INPUT_KEY *key);
void main_loop();
void EFIAPI timer_callback(EFI_EVENT event, void *context);

EFI_STATUS EFIAPI efi_main(
    EFI_HANDLE image_handle,
    EFI_SYSTEM_TABLE *system_table
) {
    InitializeLib(image_handle, system_table);
    print_efi_info();
    set_debug_marker(image_handle);

    EFI_STATUS status;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    status = graphics_acquire(&gop);
    if (EFI_ERROR(status))
        die(L"Failed to setup GOP!", status);

    status = graphics_allocate_backing(gop);
    if (EFI_ERROR(status))
        die(L"Failed to allocate graphics backing!", status);

    graphics_set_context(gop);

    Print(
        L"Graphics Initialized: %upx by %upx display\n",
        graphics_width(),
        graphics_height()
    );

    Print(L"Welcome to UEFI Breakout!\n");
    Print(L"Press any key to begin (or q to quit).\n");

    EFI_INPUT_KEY key;
    status = read_key(&key);
    if (EFI_ERROR(status))
        die(L"Failed to read key!", status);
    if (key.UnicodeChar == L'q')
        return EFI_SUCCESS;

    main_loop();

    Print(L"Done!\n");

    status = read_key(&key);
    if (EFI_ERROR(status))
        die(L"Failed to read key!", status);

    return EFI_SUCCESS;
}

void main_loop() {
    ssize_t x = 0;
    ssize_t y = 0;
    ssize_t vx = 5;
    ssize_t vy = 5;
    size_t size = 20;

    size_t width = graphics_width();
    size_t height = graphics_height();

    EFI_TIME last_frame_time = get_time();
    EFI_STATUS status;

    while (1) {
        graphics_clear(COLOR_BLACK);
        graphics_rect(0, 0, width, height, COLOR_RED);
        graphics_rect(x, y, size, size, COLOR_WHITE);

        x += vx;
        y += vy;

        if (x + size >= width || x <= 0)
            vx *= -1;
        if (y + size >= height || y <= 0)
            vy *= -1;

        status = graphics_present();
        if (EFI_ERROR(status))
            die(L"Fatal error! Could not present frame!", status);

        sleep(33);
    }
}

#ifdef DEBUG_ENABLED
void set_debug_marker(EFI_HANDLE image_handle) {
    EFI_LOADED_IMAGE *loaded_image = NULL;
    EFI_GUID guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;

    EFI_STATUS status = uefi_call_wrapper(
        gBS->HandleProtocol,
        3,
        image_handle,
        &guid,
        (void**) &loaded_image
    );

    if (EFI_ERROR(status))
        die(L"Failed to acquire image handle for debugging", status);

    Print(
        L"[dbg] Setting image base at 0x10008 to %p\n",
        loaded_image->ImageBase
    );
    // Set a basic marker for debugging to pick up
    volatile uint64_t *marker_ptr = (uint64_t*)0x10000;
    volatile uint64_t *image_base_ptr = (uint64_t*)0x10008;
    *image_base_ptr = (uint64_t)loaded_image->ImageBase;
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

#ifdef QUICK_DIE
    uefi_call_wrapper(
        gRT->ResetSystem,
        4,
        EfiResetShutdown,
        EFI_SUCCESS,
        0,
        NULL
    );
#endif

    hcf();
}

void die_statusless(const wchar_t *msg) {
    Print(L"%s\n", msg);

#ifdef QUICK_DIE
    uefi_call_wrapper(
        gRT->ResetSystem,
        4,
        EfiResetShutdown,
        EFI_SUCCESS,
        0,
        NULL
    );
#endif

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
