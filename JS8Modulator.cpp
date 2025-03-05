#include "JS8Modulator.h"
#include <Arduino.h>

const uint32_t JS8Modulator::bufferDuration = 30;
const uint32_t JS8Modulator::sampleRate = 44100;

JS8Modulator::JS8Modulator(AudioPlayMemory& playMem) : playMemRef(playMem) {
    Serial.println("JS8Modulator initialized");
}

void JS8Modulator::generateWaveform(const char* message) {
    const float frequency = 1000.0;
    const float duration = 12.6;
    const float amplitude = 0.5;
    const uint32_t numSamples = duration * sampleRate;

    Serial.print("Generating waveform: ");
    Serial.println(message);
    for (uint32_t i = 0; i < numSamples; i++) {
        float t = (float)i / sampleRate;
        transmitBuffer[i] = (int16_t)(amplitude * 32767.0 * sin(2.0 * PI * frequency * t));
    }
    Serial.print("Generated ");
    Serial.print(numSamples);
    Serial.println(" samples");
}

void JS8Modulator::startPlayback() {
    Serial.println("Starting playback");
    playMemRef.play((const unsigned int*)transmitBuffer);
    if (!playMemRef.isPlaying()) {
        Serial.println("Playback failed to start");
    }
}

bool JS8Modulator::isPlaying() {
    bool playing = playMemRef.isPlaying();
    if (playing) {
        Serial.println("Playing...");
    }
    return playing;
}