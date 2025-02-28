#include "DisplayManager.h"

DisplayManager::DisplayManager()
  : tft(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCK, TFT_MISO) {
  // Constructor initializes ILI9341_t3n with pin definitions
}

void DisplayManager::begin() {
  tft.begin();
  tft.setRotation(3);  // 270° counterclockwise as requested
  tft.fillScreen(COLOR_BLACK);
  tft.useFrameBuffer(true);  // Enable DMA buffer
  // Note: DMA is used for efficient SPI transfers to display
}

void DisplayManager::drawPixel(int16_t x, int16_t y, uint16_t color) {
  // Draw a single pixel at (x,y) with specified color
  tft.drawPixel(x, y, color);
}

void DisplayManager::updateScreenAsync() {
  // Trigger asynchronous DMA update of the display
  tft.updateScreenAsync();
}

void DisplayManager::setTextColor(uint16_t color) {
  tft.setTextColor(color);
}

void DisplayManager::setTextSize(uint8_t size) {
  tft.setTextSize(size);
}

void DisplayManager::setCursor(int16_t x, int16_t y) {
  tft.setCursor(x, y);
}

void DisplayManager::print(const char* text) {
  tft.print(text);
}

void DisplayManager::setFont(const ILI9341_t3_font_t& font) {
  tft.setFont(font);
}

void DisplayManager::setFont() {
  tft.setFont();  // Reset to built-in font
}