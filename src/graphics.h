#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include <efi.h>
#include <efiprot.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PIXEL_WIDTH 4

typedef __attribute__((aligned(4))) struct {
    uint8_t _reserved;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} PixelBGR;

/**
 * A graphics context representation which stores access to the GOP and frame
 * buffer along with a back framebuffer to enable double buffered output.
 */
typedef struct {
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    EFI_PHYSICAL_ADDRESS fb;
    PixelBGR* back_fb;
} GraphicsContext;

// Since x86 is little endian, the binary format is rRGB for BGR 8-bit
#define COLOR_BLACK 0x00000000
#define COLOR_WHITE 0x00FFFFFF
#define COLOR_RED 0x00FF0000
#define COLOR_BLUE 0x000000FF
#define COLOR_GREEN 0x0000FF00

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
EFI_STATUS graphics_acquire(EFI_GRAPHICS_OUTPUT_PROTOCOL **gop);

/**
 * Allocate memory for the backing framebuffer. This is a required step to get
 * double buffering working.
 *
 * @retval EFI_SUCEESS Success
 * @retval EFI_INVALID_PARAMETER `gop` was NULL
 * @retval EFI_OUT_OUF_RESOURCES Failed to allocate due to a lock of free memory
 */
EFI_STATUS graphics_allocate_backing(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop);

/**
 * Set the global graphics context singleton.
 *
 * This must be called before any functions prefixed with `graphics_`
 */
void graphics_set_context(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop);

/**
 * Confirm that the global singleton is valid
 */
bool graphics_context_valid();

/**
 * Present the back framebuffer by blitting it onto the main GOP framebuffer.
 *
 * @retval EFI_SUCCESS Success
 * @retval EFI_DEVICE_ERROR The graphics device had an error
 */
EFI_STATUS graphics_present();

size_t graphics_width();
size_t graphics_height();

/**
 * Sets the pixel located at (x, y) to the specified value
 */
void graphics_set(size_t x, size_t y, uint32_t pixel);

void graphics_clear(uint32_t color);
void graphics_rect(
    size_t x,
    size_t y,
    size_t width,
    size_t height,
    uint32_t color
);

#endif // GRAPHICS_H_
