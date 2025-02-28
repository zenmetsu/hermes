#ifndef OUTPUT_H
#define OUTPUT_H

#include "DisplayManager.h"
#include "Config.h"

// Class to manage the Output window (tentative 320x112 region)
// Purpose: Displays decoded messages or requested data
class Output {
public:
  Output(DisplayManager& display);  // Constructor
  void update();  // Update output buffer (placeholder)
  void render();  // Render output region (y=128 to y=239 tentative)

private:
  DisplayManager& display;  // Reference to display manager
  uint16_t buffer[112][SCREEN_WIDTH];  // Output pixel buffer (320x112 tentative)
};

#endif