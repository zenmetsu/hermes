#ifndef INDICATOR_H
#define INDICATOR_H

#include "DisplayManager.h"
#include "Config.h"

// Class to manage the tone indicator region (320x8 below waterfall)
// Purpose: Displays tone detection markers
class Indicator {
public:
  Indicator(DisplayManager& display);  // Constructor takes display reference
  void update(float centerFreq);       // Update buffer with a tone indicator
  void clear();                        // Clear tone indicator region
  void render();                       // Render tone indicators (y=60 to y=67)

private:
  DisplayManager& display;  // Reference to display manager
  uint16_t buffer[8][SCREEN_WIDTH];  // Tone indicator pixel buffer
};

#endif