#include "Input.h"

Input::Input(DisplayManager& disp)
  : display(disp) {
  // Initialize buffer to black
  memset(buffer, 0, sizeof(buffer));
}

void Input::update() {
  // Placeholder: Clear buffer for now
  for (int x = 0; x < 20; x++) {
    for (int y = 0; y < SCREEN_WIDTH; y++) {
      buffer[x][y] = COLOR_BLACK;
    }
  }
}

void Input::render() {
  // Render input region (y=108 to y=127) using regional update
  display.renderRegion(0, 108, SCREEN_WIDTH, 20, (uint16_t*)buffer);
}