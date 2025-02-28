#include "Waterfall.h"

Waterfall::Waterfall(DisplayManager& disp)
  : display(disp), maxMagnitude(0.0), gainFactor(1.0), lastUpdateTime(0) {
  // Initialize buffer to black
  memset(buffer, 0, sizeof(buffer));
}

void Waterfall::update(AudioAnalyzeFFT1024& fft) {
  // Shift waterfall left
  for (int x = 0; x < WATERFALL_HEIGHT - 1; x++) {
    memcpy(buffer[x], buffer[x+1], SCREEN_WIDTH * sizeof(uint16_t));
  }

  // Calculate new rightmost column with gain control
  float binHz = SAMPLE_RATE / 1024.0;  // Hz per bin (43.066Hz)
  int minBin = MIN_FREQ / binHz;       // ~7
  int maxBin = MAX_FREQ / binHz;       // ~84
  int binRange = maxBin - minBin;      // ~77 bins

  // Find maximum magnitude for this frame
  float localMax = 0.0;
  for (int y = 0; y < SCREEN_WIDTH; y++) {
    int bin = minBin + (y * binRange) / SCREEN_WIDTH;
    float mag = fft.read(bin);
    if (mag > localMax) localMax = mag;
  }

  // Update global max with decay
  if (localMax > maxMagnitude) maxMagnitude = localMax;
  else maxMagnitude *= GAIN_DECAY;
  gainFactor = maxMagnitude > 0 ? 1.0 / maxMagnitude : 1.0;

  // Fill new column
  for (int y = 0; y < SCREEN_WIDTH; y++) {
    int bin = minBin + (y * binRange) / SCREEN_WIDTH;
    float magnitude = fft.read(bin) * gainFactor * 5.0;

    if (magnitude > 0.75) {
      buffer[WATERFALL_HEIGHT-1][y] = COLOR_WHITE;
    } else if (magnitude > 0.5) {
      buffer[WATERFALL_HEIGHT-1][y] = COLOR_LIGHTGREY;
    } else if (magnitude > 0.25) {
      buffer[WATERFALL_HEIGHT-1][y] = COLOR_GREY;
    } else if (magnitude > 0.1) {
      buffer[WATERFALL_HEIGHT-1][y] = COLOR_DARKGREY;
    } else if (magnitude > 0.05) {
      buffer[WATERFALL_HEIGHT-1][y] = COLOR_SEAFOAM;
    } else {
      buffer[WATERFALL_HEIGHT-1][y] = COLOR_BLACK;
    }
  }
}

void Waterfall::render() {
  // Render waterfall to display
  for (int x = 0; x < WATERFALL_HEIGHT; x++) {
    for (int y = 0; y < SCREEN_WIDTH; y++) {
      display.drawPixel(y, WATERFALL_HEIGHT-1-x, buffer[x][y]);
    }
  }
  display.updateScreenAsync();
  lastUpdateTime = millis();  // Update timestamp after rendering
}

bool Waterfall::isReadyToUpdate() {
  // Check if 0.5s has elapsed since last render
  return (millis() - lastUpdateTime) >= UPDATE_INTERVAL;
}