#include "JS8Modulator.h"
#include <Arduino.h>

const uint32_t JS8Modulator::bufferDuration = 30;
const uint32_t JS8Modulator::sampleRate = 22050;

JS8Modulator::JS8Modulator() {
    Serial.println("JS8Modulator initialized");
}

void JS8Modulator::generateWaveform(const char* message) {
    const float frequency = 1000.0; // Placeholder—JS8 FSK to be implemented
    const float duration = 12.6;    // JS8 Normal mode duration
    const float amplitude = 0.5;
    const uint32_t numSamples = duration * sampleRate;       // 277,830 samples
    const uint32_t numPackedSamples = numSamples / 2;        // 138,915 32-bit words
    const uint32_t totalPackedSamples = 330750;              // 30s worth (661,500 / 2)

    Serial.print("Generating waveform: ");
    Serial.println(message);

    // 1) Set the 32-bit header
    uint32_t header = (0x82 << 24) | (numSamples & 0xFFFFFF); // 0x82 + sample count
    transmitBuffer[0] = header;

    // 2) Generate sine wave for 12.6s
    for (uint32_t i = 0; i < numPackedSamples; i++) {
        float t1 = (float)(2 * i) / sampleRate;     // First sample
        float t2 = (float)(2 * i + 1) / sampleRate; // Second sample
        int16_t sample1 = (int16_t)(amplitude * 32767.0 * sin(2.0 * PI * frequency * t1));
        int16_t sample2 = (int16_t)(amplitude * 32767.0 * sin(2.0 * PI * frequency * t2));
        
        transmitBuffer[i + 1] = ((uint32_t)(uint16_t)sample2 << 16) | ((uint32_t)(uint16_t)sample1 & 0xFFFF);
    }

    // 3) Fill the remainder with silence (zeros)
    for (uint32_t i = numPackedSamples; i < totalPackedSamples; i++) {
        transmitBuffer[i + 1] = 0;
    }

    Serial.print("Generated ");
    Serial.print(numSamples);
    Serial.println(" samples with header, padded with silence");
}