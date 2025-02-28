// Main sketch for the Hermes project (JS8 Teensy Transceiver)
// Purpose: Coordinates audio input, display output, and signal processing

#include "AudioManager.h"
#include "DisplayManager.h"
#include "Visualizer.h"
#include "Indicator.h"
#include "Status.h"
#include "Input.h"
#include "Output.h"
#include "SignalProcessor.h"
#include "Operator.h"
#include "Utils.h"
#include "Config.h"
#include "font_Arial.h"  // Include Arial font

// Global instances
AudioManager audioManager;
DisplayManager displayManager;
Visualizer visualizer(displayManager);
Indicator indicator(displayManager);
Utils utils;
Status status(displayManager, utils);
Input input(displayManager);
Output output(displayManager);
SignalProcessor signalProcessor(utils, indicator);

uint32_t startTime;  // Track startup time
float lastMarkSecond = -1.0;  // Track last mark to avoid repeats

void setup() {
  Serial.begin(115200);  // For debugging
  audioManager.begin();
  displayManager.begin();
  utils.begin();
  startTime = millis();  // Record startup time
  utils.debugPrint("Hermes initialized");

  // Initial status update
  status.update();
  status.render();
}

void loop() {
  audioManager.updateBuffer();  // Continuously update PSRAM buffer

  // Update and render visualizer when FFT is available and 0.5s has passed
  if (audioManager.isFFTAvailable() && visualizer.isReadyToUpdate()) {
    visualizer.update(audioManager.getFFT());
    visualizer.render();
  }

  // Update status periodically (e.g., every second)
  static uint32_t lastStatusUpdate = 0;
  if (millis() - lastStatusUpdate >= 1000) {
    status.update();
    status.render();
    lastStatusUpdate = millis();
  }

  // Check RTC for JS8 timing marks and demodulation triggers
  if (utils.isRTCActive()) {
    float seconds = second() + (millis() % 1000) / 1000.0;

    // Print timing mark at 0, 15, 30, 45 seconds
    float markSeconds[] = {0.0, 15.0, 30.0, 45.0};
    for (int i = 0; i < 4; i++) {
      if (seconds >= markSeconds[i] && seconds < markSeconds[i] + 0.01 &&
          abs(lastMarkSecond - markSeconds[i]) > 0.5) {  // Avoid rapid repeats
        utils.debugPrint("======= MARK =======");
        lastMarkSecond = markSeconds[i];
        break;
      }
    }

    // Trigger demodulation after initial 17.36s delay
    if ((millis() - startTime) >= (JS8_SEARCH_DURATION * 1000)) {
      if ((seconds >= 2.36 && seconds < 2.37) ||
          (seconds >= 17.36 && seconds < 17.37) ||
          (seconds >= 32.36 && seconds < 32.37) ||
          (seconds >= 47.36 && seconds < 47.37)) {
        utils.debugPrint("Starting JS8 Normal mode processing");
        signalProcessor.processNormal(audioManager.getAudioBuffer(),
                                     audioManager.getBufferPos());
        indicator.render();  // Render tone indicators after detection
      }
    }
  }
}