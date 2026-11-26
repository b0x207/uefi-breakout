#include <efi.h>
#include <efierr.h>
#include <efilib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <wchar.h>

#include "common.h"
#include "graphics.h"
#include "rand.h"

#define STATUS_N_SET 35
#define STATUS_N_CLEAR 8

void print_efi_info();
void set_debug_marker(EFI_HANDLE image_handle);
EFI_STATUS read_key(EFI_INPUT_KEY *key);
EFI_STATUS check_key(EFI_INPUT_KEY *key, bool clear_on_exist);
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

    main_loop();

    Print(L"Done!\n");

    EFI_INPUT_KEY key;
    status = read_key(&key);
    if (EFI_ERROR(status))
        die(L"Failed to read key!", status);

    return EFI_SUCCESS;
}

void main_loop() {
    size_t width = graphics_width();
    size_t height = graphics_height();
    size_t size = 20;
    size_t paddle_width = 30;
    size_t paddle_size = 100;
    size_t paddle_offset = 50;

    EFI_TIME seed_time = get_time();
    srand(seed_time.Second + seed_time.Minute);

    double x = (double) width / 2 - (double) size / 2;
    double y = (double) height / 2 - (double) size / 2;
    double vx = (rand() > RAND_HALF ? -1 : 1) * 10;
    double vy = (rand() > RAND_HALF ? -1 : 1) * 10;

    double paddle_left_y = (double) height / 2 - (double) paddle_size / 2;
    double paddle_right_y = (double) height / 2 - (double) paddle_size / 2;
    int paddle_left_move = 0;
    int paddle_right_move = 0;
    double PADDLE_SPEED = 10.0;

    EFI_STATUS status;
    EFI_INPUT_KEY key;

    while (1) {
        status = check_key(&key, true);
        if (status != EFI_NOT_READY && EFI_ERROR(status))
            die(L"Failed to check key state!", status);

        if (status != EFI_NOT_READY) {
            switch (key.UnicodeChar) {
                case L'q':
                    paddle_left_move = -1;
                    break;
                case L'e':
                    paddle_left_move = 1;
                    break;
                case L'w':
                    paddle_left_move = 0;
                    break;
                case L'i':
                    paddle_right_move = -1;
                    break;
                case L'p':
                    paddle_right_move = 1;
                    break;
                case L'o':
                    paddle_right_move = 0;
                    break;
            }
        }

        paddle_left_y += paddle_left_move * PADDLE_SPEED;
        paddle_right_y += paddle_right_move * PADDLE_SPEED;

        graphics_clear(COLOR_BLACK);
        graphics_rect(0, 0, width, height, COLOR_RED);
        graphics_rect((size_t) x, (size_t) y, size, size, COLOR_WHITE);
        graphics_rect(
            paddle_offset,
            (size_t) paddle_left_y,
            paddle_width,
            paddle_size,
            COLOR_WHITE
        );
        graphics_rect(
            width - paddle_offset - paddle_width,
            (size_t) paddle_right_y,
            paddle_width,
            paddle_size,
            COLOR_WHITE
        );

        x += vx;
        y += vy;

        if (x + size >= width || x <= 0) {
            x = x <= 0 ? 0 : width - size;
            vx *= -1;
        }

        if (y + size >= height || y <= 0) {
            y = y <= 0 ? 0 : height - size;
            vy *= -1;
        }

        status = graphics_present();
        if (EFI_ERROR(status))
            die(L"Fatal error! Could not present frame!", status);

        sleep(16);
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

/**
 * Check if any particular key is being pressed.
 *
 * If there is data, `key` will be set to it. If `clear_on_exist` is also true,
 * then the input device will be reset.
 *
 * @retval EFI_SUCCESS Success
 * @retval EFI_NOT_READY No key has been pressed yet
 * @retval * A forwarded error
 */
EFI_STATUS check_key(EFI_INPUT_KEY *key, bool clear_on_exist) {
    EFI_STATUS status = EFI_SUCCESS;

    status = uefi_call_wrapper(gBS->CheckEvent, 1, gST->ConIn->WaitForKey);
    if (status != EFI_SUCCESS)
        return status;

    status = uefi_call_wrapper(gST->ConIn->ReadKeyStroke, 2, gST->ConIn, key);
    if (status != EFI_SUCCESS)
        return status;

    status = uefi_call_wrapper(gST->ConIn->Reset, 2, gST->ConIn, false);

    return status;
}
