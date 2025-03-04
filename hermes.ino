#include <Audio.h>
#include <SPI.h>
#include <ILI9341_t3n.h>
#include <Wire.h>
#include <USBHost_t36.h>
#include <TimeLib.h>
#include "config.h"
#include "operator.h"
#include "AudioManager.h"
#include "UIManager.h"
#include "JS8Scheduler.h"
#include "JS8Resampler.h"
#include "JS8Demodulator.h"
#include "JS8Modulator.h"
#include "JS8Responder.h"
#include "Buffers.h"

// Hardware objects
ILI9341_t3n tft(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCK, TFT_MISO);
USBHost usbHost;
USBHIDParser hid(usbHost);
KeyboardController keyboard(usbHost);

// Audio objects
AudioControlSGTL5000 sgtl5000;
AudioInputI2S audioInput;
AudioAnalyzeFFT1024 fft1024;
AudioRecordQueue queue1;
AudioConnection patchCord1(audioInput, 0, fft1024, 0);
AudioConnection patchCord2(audioInput, 0, queue1, 0);

// Global managers
AudioManager audioManager;
UIManager uiManager(tft);
JS8Scheduler js8Scheduler;
JS8Resampler js8Resampler;
JS8Demodulator js8Demodulator;
JS8Modulator js8Modulator;
JS8Responder js8Responder;

void setup() {
  Serial.begin(115200);
  AudioMemory(60);
  
  setSyncProvider(getTeensy3Time);
  if (timeStatus() != timeSet) setTime(0, 0, 0, 1, 1, 2025);
  
  usbHost.begin();
  keyboard.attachPress(OnPress);
  keyboard.attachRawPress(OnRawPress);
  
  audioManager.begin();
  uiManager.begin();
  js8Scheduler.begin();
}

void loop() {
  usbHost.Task();
  audioManager.update();
  uiManager.update();
  js8Scheduler.update();
}

void OnPress(int key) {
  uiManager.handleKeyPress(key);
}

void OnRawPress(uint8_t keycode) {
  uiManager.handleRawKeyPress(keycode);
}

time_t getTeensy3Time() {
  return Teensy3Clock.get();
}