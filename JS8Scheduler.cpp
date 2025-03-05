#include "JS8Scheduler.h"
#include <Arduino.h>

extern bool transmitEnabled;
extern bool isTransmitting;
extern time_t getTeensy3Time();

time_t JS8Scheduler::lastCycleTime = 0;
const uint16_t JS8Scheduler::cycleDuration = 15;

JS8Scheduler::JS8Scheduler(JS8& js8, JS8Demodulator& demod, JS8Modulator& mod, RadioControl& radio)
    : js8Ref(js8), demodRef(demod), modRef(mod), radioRef(radio) {}

void JS8Scheduler::begin() {
    setSyncProvider(getTeensy3Time);
    if (timeStatus() != timeSet) {
        return;
    }
    lastCycleTime = now();
}

void JS8Scheduler::update() {
    time_t currentTime = now();
    int seconds = second(currentTime);
    int windowPosition = seconds % cycleDuration;
    bool isWindowStart = (windowPosition == 0);

    if (isWindowStart && (currentTime - lastCycleTime >= cycleDuration)) {
        lastCycleTime = currentTime;
        if (transmitEnabled && js8Ref.hasPendingMessage()) {
            Serial.println("Transmission window detected");
            triggerTransmission();
        } else {
            triggerDemodulation();
        }
    }
    isTransmitting = modRef.isPlaying();
    if (!isTransmitting && js8Ref.hasPendingMessage()) {
        Serial.println("Playback ended, clearing buffer");
        js8Ref.clearOutputBuffer();
    }
}

void JS8Scheduler::triggerDemodulation() {
    isTransmitting = false;
    demodRef.demodulate();
}

void JS8Scheduler::triggerTransmission() {
    Serial.println("Triggering transmission");
    radioRef.startTransmission();
    modRef.startPlayback();
}