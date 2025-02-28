#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <SPI.h>
#include <ILI9341_t3n.h>
#include "Config.h"

// Class to manage the ILI9341 display with DMA
// Purpose: Handles display initialization and pixel drawing
class DisplayManager {
public:
  DisplayManager();           // Constructor
  void begin();               // Initialize display
  void drawPixel(int16_t x, int16_t y, uint16_t color);  // Draw a pixel
  void updateScreenAsync();   // Update display using DMA

private:
  ILI9341_t3n tft;  // Display object
};

#endif
