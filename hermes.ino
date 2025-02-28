// Main sketch for the Hermes project (JS8 Teensy Transceiver)
// Purpose: Coordinates audio input, display output, and signal processing

#include "AudioManager.h"
#include "DisplayManager.h"
#include "Waterfall.h"
#include "SignalProcessor.h"
#include "Utils.h"
#include "Config.h"

// Global instances
AudioManager audioManager;
DisplayManager displayManager;
Waterfall waterfall(displayManager);
Utils utils;
SignalProcessor signalProcessor(utils);

uint32_t startTime;  // Track startup time
float lastMarkSecond = -1.0;  // Track last mark to avoid repeats

void setup() {
  Serial.begin(115200);  // For debugging
  audioManager.begin();
  displayManager.begin();
  utils.begin();
  startTime = millis();  // Record startup time
  utils.debugPrint("Hermes initialized");
}

void loop() {
  audioManager.updateBuffer();  // Continuously update PSRAM buffer

  // Update and render waterfall only when FFT is available and 0.5s has passed
  if (audioManager.isFFTAvailable() && waterfall.isReadyToUpdate()) {
    waterfall.update(audioManager.getFFT());
    waterfall.render();
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
      }
    }
  }
}