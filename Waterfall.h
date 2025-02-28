#ifndef WATERFALL_H
#define WATERFALL_H

#include "DisplayManager.h"
#include "Config.h"
#include <Audio.h>

// Class to manage waterfall display processing
// Purpose: Processes FFT data and renders spectral waterfall
class Waterfall {
public:
  Waterfall(DisplayManager& display);  // Constructor takes display reference
  void update(AudioAnalyzeFFT1024& fft);  // Update waterfall with new FFT data
  void render();                         // Render waterfall to display
  bool isReadyToUpdate();                // Check if 0.5s interval has elapsed

private:
  DisplayManager& display;  // Reference to display manager
  uint16_t buffer[WATERFALL_HEIGHT][SCREEN_WIDTH];  // Waterfall pixel buffer
  float maxMagnitude;       // For automatic gain control
  float gainFactor;         // Current gain factor
  const float GAIN_DECAY = 0.99;  // Decay rate for max magnitude
  uint32_t lastUpdateTime;  // Last time buffer was updated
  const uint32_t UPDATE_INTERVAL = 500;  // 0.5 seconds in milliseconds  
};

#endif