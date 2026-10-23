#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include <efi.h>
#include <efiprot.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PIXEL_WIDTH 4

typedef struct {
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    EFI_PHYSICAL_ADDRESS fb;
} GraphicsContext;

/**
 * A singleton to provide easy global access to the current graphics context
 *
 * Since there can only ever be a single active graphics context, this is a
 * reasonable means of access.
 */
extern GraphicsContext GRAPHICS_CONTEXT;

/**
 * Acquire access to the graphics output protocol
 *
 * @returns either EFI_SUCCESS or a forwarded error
 */
EFI_STATUS acquire_gop(EFI_GRAPHICS_OUTPUT_PROTOCOL **gop);

/**
 * Set the global graphics context singleton.
 *
 * This must be called before any functions prefixed with `graphics_`
 */
void set_graphics_context(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop);

bool graphics_context_valid();

size_t graphics_width();
size_t graphics_height();

/**
 * Sets the pixel located at (x, y) to the specified value
 */
void graphics_set(size_t x, size_t y, uint32_t pixel);

void graphics_rect(
    size_t x,
    size_t y,
    size_t width,
    size_t height,
    uint32_t color
);

#endif // GRAPHICS_H_
