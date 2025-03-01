#include "DisplayManager.h"
#include <Arduino.h>  // For Serial

DisplayManager::DisplayManager()
  : tft(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCK, TFT_MISO) {
}

void DisplayManager::begin() {
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(COLOR_BLACK);
  tft.useFrameBuffer(true);
  Serial.println("DisplayManager begin completed");
}

void DisplayManager::drawPixel(int16_t x, int16_t y, uint16_t color) {
  tft.drawPixel(x, y, color);
}

void DisplayManager::updateScreenAsync() {
  tft.updateScreenAsync();
}

void DisplayManager::renderRegion(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t* buffer) {
  Serial.print("Rendering region: x="); Serial.print(x);
  Serial.print(", y="); Serial.print(y);
  Serial.print(", w="); Serial.print(w);
  Serial.print(", h="); Serial.println(h);
  tft.writeRect(x, y, w, h, buffer);
  Serial.println("renderRegion completed");
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
  tft.setFont();
}