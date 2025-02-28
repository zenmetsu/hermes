#ifndef INPUT_H
#define INPUT_H

#include "DisplayManager.h"
#include "Config.h"

// Class to manage the Input window (tentative 320x20 region)
// Purpose: Displays keyboard input buffer
class Input {
public:
  Input(DisplayManager& display);  // Constructor
  void update();  // Update input buffer (placeholder)
  void render();  // Render input region (y=108 to y=127 tentative)

private:
  DisplayManager& display;  // Reference to display manager
  uint16_t buffer[20][SCREEN_WIDTH];  // Input pixel buffer (320x20 tentative)
};

#endif