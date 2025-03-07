#include "JS8Scheduler.h"
#include <Arduino.h>

extern bool transmitEnabled;
extern bool isTransmitting;
extern time_t getTeensy3Time();

time_t JS8Scheduler::lastCycleTime = 0;
const uint16_t JS8Scheduler::cycleDuration = 15;

JS8Scheduler::JS8Scheduler(JS8& js8, JS8Demodulator& demod, JS8Modulator& mod, RadioControl& radio, AudioPlayMemory& playMem)
    : js8Ref(js8), demodRef(demod), modRef(mod), radioRef(radio), playMemRef(playMem), 
      waveformGenerated(false), txState(TxState::IDLE) {}

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

    // State machine for transmission
    switch (txState) {
        case TxState::IDLE:
            if (js8Ref.hasPendingMessage() && !waveformGenerated) {
                Serial.println("Detected pending message, generating waveform");
                modRef.generateWaveform(js8Ref.getOutputBuffer());
                waveformGenerated = true;
            }
            if (isWindowStart && (currentTime - lastCycleTime >= cycleDuration)) {
                lastCycleTime = currentTime;
                if (transmitEnabled && js8Ref.hasPendingMessage() && waveformGenerated) {
                    Serial.println("Transmission window detected");
                    txState = TxState::PREPARING; // Move to preparing state
                } else {
                    triggerDemodulation();
                }
            }
            if (!isTransmitting && js8Ref.hasPendingMessage() && waveformGenerated && !isWindowStart) {
                Serial.println("Waiting for next transmission window...");
            }
            break;

        case TxState::PREPARING:
            startTransmission();
            txState = TxState::TRANSMITTING; // Move to transmitting state
            break;

        case TxState::TRANSMITTING:
            if (!playMemRef.isPlaying()) {
                txState = TxState::STOPPING; // Playback done, move to stopping
            }
            break;

        case TxState::STOPPING:
            stopTransmission();
            js8Ref.clearOutputBuffer();
            waveformGenerated = false;
            txState = TxState::IDLE; // Back to idle
            break;
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
        isTransmitting = false;
    } else {
        isTransmitting = true;
    }
}

void JS8Scheduler::stopTransmission() {
    Serial.println("Stopping transmission");
    radioRef.stopTransmission();
    isTransmitting = false;
}