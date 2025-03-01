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

static uint32_t lastVisualizerUpdate = 0;

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("Starting setup...");
  audioManager.begin();
  Serial.println("Audio initialized");
  displayManager.begin();
  Serial.println("Display initialized");
  utils.begin();
  Serial.println("Utils initialized");
  startTime = millis();
  utils.debugPrint("Hermes initialized");

  // Force Status update and render
  status.update();
  status.render();
  Serial.println("Status rendered in setup");

  // Test Visualizer with forced update
  visualizer.update(audioManager.getFFT());
  visualizer.render();
  Serial.println("Visualizer rendered in setup");
}

void loop() {
  audioManager.updateBuffer();

  if (audioManager.isFFTAvailable() && (millis() - lastVisualizerUpdate >= 500)) {
    visualizer.update(audioManager.getFFT());
    visualizer.render();
    lastVisualizerUpdate = millis();
    Serial.println("Visualizer loop update");
  }

  static uint32_t lastStatusUpdate = 0;
  if (millis() - lastStatusUpdate >= 1000) {
    status.update();
    status.render();
    lastStatusUpdate = millis();
    Serial.println("Status loop update");
  }

  delay(1);
}