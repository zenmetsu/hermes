#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <SPI.h>
#include <ILI9341_t3n.h>
#include "Config.h"

// Class to manage the ILI9341 display with DMA
// Purpose: Handles display initialization, pixel drawing, and text rendering
class DisplayManager {
public:
  DisplayManager();           // Constructor
  void begin();               // Initialize display
  void drawPixel(int16_t x, int16_t y, uint16_t color);  // Draw a pixel
  void updateScreenAsync();   // Update full display using DMA
  void renderRegion(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t* buffer);  // Render specific region
  void setTextColor(uint16_t color);  // Set text color
  void setTextSize(uint8_t size);     // Set text size (1=smallest, 6x8 pixels)
  void setCursor(int16_t x, int16_t y);  // Set text cursor position
  void print(const char* text);       // Print text to display
  void setFont(const ILI9341_t3_font_t& font);  // Set custom font (reference)
  void setFont();                        // Reset to built-in font

private:
  ILI9341_t3n tft;  // Display object
};

#endif