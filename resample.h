#ifndef RESAMPLE_H
#define RESAMPLE_H

#include <Arduino.h>

// Polyphase filter parameters
#define L 441  // Interpolation factor
#define M 128  // Decimation factor
#define TAPS_PER_BRANCH 16
#define TOTAL_TAPS (L * TAPS_PER_BRANCH)
#define INPUT_BLOCK_SIZE 128

class PolyphaseResampler {
private:
    int16_t history[TAPS_PER_BRANCH - 1];
    uint32_t phase;
    uint32_t write_pos;
public:
    PolyphaseResampler();
    void resample(int16_t* input, int16_t* output, uint32_t* out_samples);
};

#endif