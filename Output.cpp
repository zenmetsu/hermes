#include "Output.h"

Output::Output(DisplayManager& disp)
  : display(disp) {
  // Initialize buffer to black
  memset(buffer, 0, sizeof(buffer));
}

void Output::update() {
  // Placeholder: Clear buffer for now
  for (int x = 0; x < 112; x++) {
    for (int y = 0; y < SCREEN_WIDTH; y++) {
      buffer[x][y] = COLOR_BLACK;
    }
  }
}

void Output::render() {
  // Render output region (y=128 to y=239) using regional update
  display.renderRegion(0, 128, SCREEN_WIDTH, 112, (uint16_t*)buffer);
}