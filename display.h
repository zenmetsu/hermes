#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <ILI9341_t3n.h>
#include "config.h"

// Color definitions (RGB565)
#define COLOR_SEAFOAM_GREEN 0x7FF0 // Approx RGB(127, 255, 0)
#define COLOR_WHITE         0xFFFF
#define COLOR_GREY_LIGHT    0xC618
#define COLOR_GREY_DARK     0x4208
#define COLOR_BLACK         0x0000

extern ILI9341_t3n tft; // Declare global tft instance

void init_display(ILI9341_t3n* tft);
void update_display_state(const char* state, ILI9341_t3n* tft = nullptr); // Default to global tft
void draw_welcome_message(ILI9341_t3n* tft);
void update_display_dma(ILI9341_t3n* tft);

#endif