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
#include "JS8.h"
#include "RadioControl.h"

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
AudioOutputI2S audioOutput;
AudioPlayMemory playMemory;
AudioConnection patchCord1(audioInput, 0, fft1024, 0);
AudioConnection patchCord2(audioInput, 0, queue1, 0);
AudioConnection patchCordTx(playMemory, 0, audioOutput, 1);

bool transmitEnabled = false;
bool isTransmitting = false;

JS8 js8;
AudioManager audioManager;
JS8Demodulator js8Demod;
JS8Modulator js8Mod(playMemory);       // Moved up
UIManager uiManager(tft, js8, js8Mod); // Now after js8Mod
RadioControl radioControl;
JS8Scheduler js8Scheduler(js8, js8Demod, js8Mod, radioControl);
JS8Resampler js8Resampler;
JS8Responder js8Responder;

void setup() {
    Serial.begin(115200);
    AudioMemory(60);

    setSyncProvider(getTeensy3Time);
    if (timeStatus() != timeSet) setTime(0, 0, 0, 1, 1, 2025);

    usbHost.begin();
    keyboard.attachPress(OnPress);
    keyboard.attachRawPress(OnRawPress);

    transmitEnabled = true;

    sgtl5000.enable();
    sgtl5000.volume(0.5);
    // No begin() needed for AudioPlayMemory

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