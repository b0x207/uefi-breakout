#include <efi.h>
#include <efierr.h>
#include <efilib.h>
#include <stdbool.h>
#include <stdint.h>

#include "common.h"
#include "efiprot.h"
#include "graphics.h"

GraphicsContext GRAPHICS_CONTEXT = { .gop = NULL };

EFI_STATUS acquire_gop(EFI_GRAPHICS_OUTPUT_PROTOCOL **gop) {
    EFI_STATUS status;

    status = uefi_call_wrapper(
        gBS->LocateProtocol,
        3,
        &gEfiGraphicsOutputProtocolGuid,
        NULL,
        (void**) gop
    );
    if (EFI_ERROR(status))
        return status;

    // The only supported format is 24-bit BGR
    EFI_GRAPHICS_PIXEL_FORMAT format = (*gop)->Mode->Info->PixelFormat;
    if (format != PixelBlueGreenRedReserved8BitPerColor) {
        Print(
            L"[err] GOP presented an unsupported pixel format: number %d\n",
            format
        );
        return EFI_UNSUPPORTED;
    }

    EFI_PIXEL_BITMASK bitmask = (*gop)->Mode->Info->PixelInformation;
    Print(L"[dbg] Bitmask:\n");
    Print(L"[dbg] B: 0x%x\n", bitmask.BlueMask);
    Print(L"[dbg] G: 0x%x\n", bitmask.GreenMask);
    Print(L"[dbg] R: 0x%x\n", bitmask.RedMask);
    Print(L"[dbg] r: 0x%x\n", bitmask.ReservedMask);

    return EFI_SUCCESS;
}

void set_graphics_context(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop) {
    GRAPHICS_CONTEXT.gop = gop;
    GRAPHICS_CONTEXT.fb = gop->Mode->FrameBufferBase;

    if (gop->Mode->Info->PixelFormat != PixelBlueGreenRedReserved8BitPerColor)
        die_statusless(
            L"Attempted to set the graphics context with the wrong format"
        );
}

bool graphics_context_valid() {
    return GRAPHICS_CONTEXT.gop != NULL;
}

size_t graphics_width() {
    if (!graphics_context_valid())
        die_statusless(L"Invalid/missing graphics context");

    return GRAPHICS_CONTEXT.gop->Mode->Info->HorizontalResolution;
}

size_t graphics_height() {
    if (!graphics_context_valid())
        die_statusless(L"Invalid/missing graphics context");

    return GRAPHICS_CONTEXT.gop->Mode->Info->VerticalResolution;
}

void graphics_set(size_t x, size_t y, uint32_t pixel) {
    if (!graphics_context_valid())
        die_statusless(L"Invalid/missing graphics context");

    uint32_t *p = (uint32_t*) GRAPHICS_CONTEXT.fb
                   + y * GRAPHICS_CONTEXT.gop->Mode->Info->PixelsPerScanLine
                   + x;
    *p = pixel;
}

void graphics_clear(uint32_t color) {
    for (size_t x = 0; x < graphics_width(); x++) {
        for (size_t y = 0; y < graphics_height(); y++) {
            graphics_set(x, y, color);
        }
    }
}

void graphics_rect(
    size_t x,
    size_t y,
    size_t width,
    size_t height,
    uint32_t color
) {
    for (int i = x; i < x + width; i++) {
        for (int j = y; j < y + height; j++) {
            graphics_set(i, j, color);
        }
    }
}
