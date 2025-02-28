#include "Indicator.h"

Indicator::Indicator(DisplayManager& disp)
  : display(disp) {
  // Initialize buffer to black
  clear();
}

void Indicator::update(float centerFreq) {
  // Map center frequency (300-3600 Hz) to x-offset (0-319 pixels)
  const float freqRange = 3600.0f - 300.0f;  // 3300 Hz
  const float pixelsPerHz = SCREEN_WIDTH / freqRange;  // ~0.09697 pixels/Hz
  int xOffset = (int)((centerFreq - 300.0f) * pixelsPerHz);  // 300 Hz = 0, 3600 Hz = 319
  if (xOffset >= 0 && xOffset < SCREEN_WIDTH) {
    // Draw 1x8 yellow bar (RGB565: R=31, G=63, B=0)
    const uint16_t COLOR_YELLOW = 0xFFE0;
    for (int y = 0; y < 8; y++) {
      buffer[y][xOffset] = COLOR_YELLOW;
    }
  }
}

void Indicator::clear() {
  // Clear tone indicator region to black
  for (int x = 0; x < 8; x++) {
    for (int y = 0; y < SCREEN_WIDTH; y++) {
      buffer[x][y] = COLOR_BLACK;
    }
  }
}

void Indicator::render() {
  // Render indicator region (y=60 to y=67) using regional update
  display.renderRegion(0, 60, SCREEN_WIDTH, 8, (uint16_t*)buffer);
}