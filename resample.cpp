#include "resample.h"
#include "polyphase_coeffs.h"

PolyphaseResampler::PolyphaseResampler() : phase(0), write_pos(0) {
    memset(history, 0, sizeof(history));
}

void PolyphaseResampler::resample(int16_t* input, int16_t* output, uint32_t* out_samples) {
    uint32_t out_idx = 0;
    for (int i = 0; i < INPUT_BLOCK_SIZE; i++) {
        memmove(history, &history[1], (TAPS_PER_BRANCH - 2) * sizeof(int16_t));
        history[TAPS_PER_BRANCH - 2] = input[i];
        while (phase < L) {
            float sum = 0.0f;
            const float* coeffs = polyphase_coeffs[phase];
            for (int k = 0; k < TAPS_PER_BRANCH - 1; k++) {
                sum += history[k] * coeffs[k];
            }
            sum += input[i] * coeffs[TAPS_PER_BRANCH - 1];
            int16_t out_sample = (int16_t)(sum * 32767.0f);
            output[out_idx++] = out_sample;
            phase += M;
        }
        phase -= L;
    }
    *out_samples = out_idx;
}