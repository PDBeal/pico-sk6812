/*!
 * \file sk6812.cpp
 *
 * \mainpage SK6812 Library
 *
 * \section intro_sec Introduction
 *
 * Example to control SK6812-based RGBW LED Modules in a strand or strip with RP2040 SDK only
 *
 * \section author Author
 *
 * Written by PBeal
 * \section license License
 * MIT license
 */
#include "sk6812.h"

SK6812::SK6812(uint16_t num, uint8_t pin, PIO pio, int sm) {
    alloc(num);

    pixelSm = sm;
    pixelPio = pio;
}

SK6812::SK6812(uint16_t num, uint8_t pin) {
    alloc(num);
    
    PIO pio = pio0;
    int sm;
    // Find a free SM on one of the PIO's
    sm = pio_claim_unused_sm(pio, false); // don't panic
    // Try pio1 if SM not found
    if (sm < 0) {
        pio = pio1;
        sm = pio_claim_unused_sm(pio, true); // panic if no SM is free
    }
    
    pixelSm = sm;
    pixelPio = pio;
    pixelGpio = pin;
}

void SK6812::begin() {
    uint offset = pio_add_program(pixelPio, &sk6812_program);
    sk6812_program_init(pixelPio, pixelSm, offset, pixelGpio, 800000, true);
}

// Allocate 4 bytes per pixel, init to RGBW 'off' state:
void SK6812::alloc(uint16_t num)
{
    numLEDs = ((pixels = (uint8_t *)calloc(num, 4)) != NULL) ? num : 0;
}

// Release memory (as needed):
SK6812::~SK6812(void)
{
    if (pixels)
        free(pixels);
}

// Set pixel color from separate 8-bit R, G, B components:
void SK6812::setPixelColor(uint16_t led, uint8_t red, uint8_t green, uint8_t blue, uint8_t white)
{
    if (led < numLEDs)
    {
        uint8_t *p = &pixels[led * 4];
        *p++ = red;
        *p++ = green;
        *p++ = blue;
        *p++ = white;
    }
}

// Set pixel color from 'packed' 32-bit RGBW value:
void SK6812::setPixelColor(uint16_t led, uint32_t color)
{
    if (led < numLEDs)
    {
        uint8_t *p = &pixels[led * 4];
        *p++ = color >> 24;  // Red
        *p++ = color >> 16;  // Green
        *p++ = color >> 8;   // Blue
        *p++ = color;        // White
    }
}

// Clear all pixels
void SK6812::clear()
{
    if (pixels != NULL)
    {
        memset(pixels, 0, numLEDs * 4);
    }
}

// Update the SK6812 pixels
void SK6812::show(void) {
    for (uint16_t i = 0; i < numLEDs; i++) 
    {
        uint8_t redPtr = this->pixels[i*4];
        uint8_t greenPtr = this->pixels[(i*4)+1];
        uint8_t bluePtr = this->pixels[(i*4)+2];
        uint8_t whitePtr = this->pixels[(i*4)+3];
        uint32_t colorData = ((uint32_t)(redPtr) << 16) | ((uint32_t)(greenPtr) << 24) | ((uint32_t)(bluePtr) << 8) | (uint32_t)(whitePtr);

        pio_sm_put_blocking(pixelPio, pixelSm, colorData); //<< 8u);
    }
}

void SK6812::fillPixelColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t white)
{
    // fill all the neopixels in the buffer with the specified rgbw values.
    for (uint16_t i = 0; i < numLEDs; i++)
    {    
        uint8_t *p = &pixels[i * 4];
        *p++ = red;
        *p++ = green;
        *p++ = blue;
        *p++ = white;
    }
}

void SK6812::updateLength(uint16_t num)
{
    if (pixels != NULL)
        free(pixels); // Free existing data (if any)
    // Allocate new data -- note: ALL PIXELS ARE CLEARED
    numLEDs = ((pixels = (uint8_t *)calloc(num, 4)) != NULL) ? num : 0;
}

uint16_t SK6812::numPixels(void) { return numLEDs; }

uint32_t SK6812::getPixelColor(uint16_t led)
{
    if (led < numLEDs)
    {
        uint16_t ofs = led * 4;
        // To keep the show() loop as simple & fast as possible, the
        // internal color representation is native to different pixel
        // types.  For compatibility with existing code, 'packed' RGBW
        // values passed in or out are always 0xRRGGBBWW order.
        return ((uint32_t)pixels[ofs] << 24) | ((uint16_t)pixels[ofs + 1] << 16) | ((uint16_t)pixels[ofs + 2] << 8) | ((uint16_t)pixels[ofs + 3]);
    }

    return 0; // Pixel # is out of bounds
}
