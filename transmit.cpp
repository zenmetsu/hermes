#include <Arduino.h>
#include "transmit.h"
#include "signal_proc.h"
#include <vector>

static std::vector<int> tx_symbols;
static size_t tx_sym_idx = 0;
static float tx_time = 0.0f;
static bool transmitting = false;

void start_transmission(const char* message) {
    for (size_t i = 0; i < strlen(message); i++) {
        tx_symbols.push_back(message[i] % NUM_TONES);
    }
    if (tx_symbols.empty()) {
        tx_symbols = {0, 1, 2, 3, 4, 5, 6, 7};
    }
    tx_sym_idx = 0;
    tx_time = 0.0f;
    transmitting = true;
}

bool is_transmitting() {
    return transmitting;
}

void generate_tx_audio(int16_t* block) {
    const float sample_period = 1.0f / static_cast<float>(SAMPLE_RATE_NATIVE);
    const float symbol_period = 1.0f / SYMBOL_RATE_NORMAL;
    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
        if (tx_sym_idx < tx_symbols.size()) {
            int tone_idx = tx_symbols[tx_sym_idx];
            float frequency = tone_frequencies[tone_idx];
            block[i] = static_cast<int16_t>(sinf(2.0f * PI * frequency * tx_time) * 32767.0f);
            tx_time += sample_period;
            if (tx_time >= symbol_period) {
                tx_time -= symbol_period;
                tx_sym_idx++;
            }
        } else {
            block[i] = 0;
            transmitting = false;
        }
    }
}