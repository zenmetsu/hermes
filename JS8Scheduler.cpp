#include "JS8Scheduler.h"
#include <Arduino.h>

extern bool transmitEnabled;
extern bool isTransmitting;
extern time_t getTeensy3Time();

time_t JS8Scheduler::lastCycleTime = 0;
const uint16_t JS8Scheduler::cycleDuration = 15;

JS8Scheduler::JS8Scheduler(JS8& js8, JS8Demodulator& demod, JS8Modulator& mod, RadioControl& radio, AudioPlayMemory& playMem)
    : js8Ref(js8), demodRef(demod), modRef(mod), radioRef(radio), playMemRef(playMem), waveformGenerated(false) {}

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

    if (js8Ref.hasPendingMessage() && !waveformGenerated) {
        Serial.println("Detected pending message, generating waveform");
        modRef.generateWaveform(js8Ref.getOutputBuffer());
        waveformGenerated = true;
    }

    if (isWindowStart && (currentTime - lastCycleTime >= cycleDuration)) {
        lastCycleTime = currentTime;
        if (transmitEnabled && js8Ref.hasPendingMessage() && waveformGenerated) {
            Serial.println("Transmission window detected");
            startTransmission();
        } else {
            triggerDemodulation();
        }
    }

    if (!isTransmitting && js8Ref.hasPendingMessage() && waveformGenerated && !isWindowStart) {
        Serial.println("Waiting for next transmission window...");
    }
}

void JS8Scheduler::triggerDemodulation() {
    isTransmitting = false;
    demodRef.demodulate();
}

void JS8Scheduler::startTransmission() {
    Serial.println("Starting transmission");
    radioRef.startTransmission();

    playMemRef.play((const unsigned int*)transmitBuffer);
    if (!playMemRef.isPlaying()) {
        Serial.println("Playback failed to start");
        radioRef.stopTransmission();
        return;
    }

    isTransmitting = true;

    while (playMemRef.isPlaying()) {
        delay(10);
    }

    stopTransmission();
    js8Ref.clearOutputBuffer();
    waveformGenerated = false;
}

void JS8Scheduler::stopTransmission() {
    Serial.println("Stopping transmission");
    radioRef.stopTransmission();
    isTransmitting = false;
}