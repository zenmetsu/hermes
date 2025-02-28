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
#include "font_Arial.h"

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

uint32_t startTime;
float lastMarkSecond = -1.0;

void setup() {
  Serial.begin(115200);
  delay(100);  // Give Serial time to initialize
  Serial.println("Starting setup...");
  audioManager.begin();
  Serial.println("Audio initialized");
  displayManager.begin();
  Serial.println("Display initialized");
  utils.begin();
  Serial.println("Utils initialized");
  startTime = millis();
  utils.debugPrint("Hermes initialized");

  status.update();
  Serial.println("Status updated");
  status.render();
  Serial.println("Status rendered");

  indicator.clear();
  Serial.println("Indicator cleared");
  indicator.render();
  Serial.println("Indicator rendered");

  input.update();
  Serial.println("Input updated");
  input.render();
  Serial.println("Input rendered");

  output.update();
  Serial.println("Output updated");
  output.render();
  Serial.println("Output rendered");
}

void loop() {
  audioManager.updateBuffer();

  // Update and render visualizer with full-screen update
  if (audioManager.isFFTAvailable() && visualizer.isReadyToUpdate()) {
    visualizer.update(audioManager.getFFT());
    visualizer.render();
    Serial.println("Visualizer updated and rendered");
  }

  // Update status periodically (e.g., every second) with regional update
  static uint32_t lastStatusUpdate = 0;
  if (millis() - lastStatusUpdate >= 1000) {
    status.update();
    status.render();
    lastStatusUpdate = millis();
    Serial.println("Status loop update");
  }

  // Check RTC for JS8 timing marks and demodulation triggers
  if (utils.isRTCActive()) {
    float seconds = second() + (millis() % 1000) / 1000.0;
    float markSeconds[] = {0.0, 15.0, 30.0, 45.0};
    for (int i = 0; i < 4; i++) {
      if (seconds >= markSeconds[i] && seconds < markSeconds[i] + 0.01 &&
          abs(lastMarkSecond - markSeconds[i]) > 0.5) {
        utils.debugPrint("======= MARK =======");
        lastMarkSecond = markSeconds[i];
        break;
      }
    }

    if ((millis() - startTime) >= (JS8_SEARCH_DURATION * 1000)) {
      if ((seconds >= 2.36 && seconds < 2.37) ||
          (seconds >= 17.36 && seconds < 17.37) ||
          (seconds >= 32.36 && seconds < 32.37) ||
          (seconds >= 47.36 && seconds < 47.37)) {
        utils.debugPrint("Starting JS8 Normal mode processing");
        signalProcessor.processNormal(audioManager.getAudioBuffer(),
                                     audioManager.getBufferPos());
        indicator.render();
        Serial.println("Indicator updated and rendered");
      }
    }
  }
}