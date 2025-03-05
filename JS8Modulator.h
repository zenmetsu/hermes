#ifndef JS8_MODULATOR_H
#define JS8_MODULATOR_H

#include "Buffers.h"
#include <Audio.h>

class JS8Modulator {
public:
    JS8Modulator(AudioPlayMemory& playMem);
    void generateWaveform(const char* message);
    void startPlayback();
    bool isPlaying();
private:
    AudioPlayMemory& playMemRef;
    static const uint32_t bufferDuration; // 30s
    static const uint32_t sampleRate;     // 44.1 kHz
};

#endif