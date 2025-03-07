#ifndef JS8_MODULATOR_H
#define JS8_MODULATOR_H

#include "Buffers.h"

class JS8Modulator {
public:
    JS8Modulator();
    void generateWaveform(const char* message); // Generates transmitBuffer
private:
    static const uint32_t bufferDuration; // 30s (not used directly now)
    static const uint32_t sampleRate;     // 22050 Hz
};

#endif