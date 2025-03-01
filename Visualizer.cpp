#include "Visualizer.h"
#include <Arduino.h>

Visualizer::Visualizer(DisplayManager& disp)
  : display(disp), maxMagnitude(0.0), gainFactor(1.0), lastUpdateTime(0) {
  memset(buffer, 0, sizeof(buffer));
}

void Visualizer::update(AudioAnalyzeFFT1024& fft) {
  if (!fft.available()) return;

  static uint32_t lastDebug = 0;
  if (millis() - lastDebug >= 1000) {
    Serial.println("Visualizer update called");
    lastDebug = millis();
  }

  for (int x = 0; x < WATERFALL_HEIGHT - 1; x++) {
    memcpy(buffer[x], buffer[x+1], SCREEN_WIDTH * sizeof(uint16_t));
  }

  float binHz = SAMPLE_RATE / 1024.0;
  int minBin = MIN_FREQ / binHz;
  int maxBin = MAX_FREQ / binHz;
  int binRange = maxBin - minBin;

  float localMax = 0.0;
  for (int y = 0; y < SCREEN_WIDTH; y++) {
    int bin = minBin + (y * binRange) / SCREEN_WIDTH;
    float mag = fft.read(bin);
    if (mag > localMax) localMax = mag;
  }

  if (millis() - lastDebug >= 1000) {
    Serial.print("Local max magnitude: "); Serial.println(localMax);
  }
  if (localMax > maxMagnitude) maxMagnitude = localMax;
  else maxMagnitude *= GAIN_DECAY;
  gainFactor = maxMagnitude > 0 ? 1.0 / maxMagnitude : 1.0;

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

void Visualizer::render() {
  display.renderRegion(0, 0, SCREEN_WIDTH, WATERFALL_HEIGHT, (uint16_t*)buffer);
  lastUpdateTime = millis();
}

bool Visualizer::isReadyToUpdate() {
  return (millis() - lastUpdateTime) >= UPDATE_INTERVAL;
}