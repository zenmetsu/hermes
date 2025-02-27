#include <Arduino.h>
#include "signal_proc.h"
#include <vector>
#include <arm_math.h>
#include "utils.h"

EXTMEM int16_t audio_buffer[SLOT_DURATION_MAX * SAMPLE_RATE_NATIVE]; // 5.17 MB, 16-bit, 44.1 kHz
EXTMEM int16_t resampled_buffer[RESAMPLED_BUFFER_SIZE]; // ~444 KB, 16-bit, 12.8 kHz
volatile size_t write_idx = 0;
float tone_frequencies[NUM_TONES];

// FFT instance moved to demodulate_normal.cpp
static uint32_t init_start_time, init_end_time;
static uint32_t peripheral_start_time, peripheral_end_time;

void init_signal_proc() {
    init_start_time = micros();
    for (int k = 0; k < NUM_TONES; k++) {
        tone_frequencies[k] = BASE_FREQUENCY + k * TONE_SPACING;
    }

    write_idx = 0;

    memset(audio_buffer, 0, sizeof(int16_t) * SLOT_DURATION_MAX * SAMPLE_RATE_NATIVE);
    memset(resampled_buffer, 0, sizeof(int16_t) * RESAMPLED_BUFFER_SIZE);
    init_end_time = micros();
    char msg[128];
    print_timestamp(msg, sizeof(msg));
    sprintf(msg + strlen(msg), "Program initialized in %lu us", init_end_time - init_start_time);
    Serial.println(msg);
}

void report_buffer_write_speed() {
    peripheral_start_time = micros();
    size_t test_samples = 44100;
    uint32_t start_time = micros();
    for (size_t i = 0; i < test_samples; i++) {
        audio_buffer[i] = 0;
    }
    uint32_t end_time = micros();
    float elapsed_seconds = (end_time - start_time) / 1000000.0f;
    float write_speed = test_samples / elapsed_seconds / 1000.0f;
    peripheral_end_time = micros();
    char msg[128];
    print_timestamp(msg, sizeof(msg));
    sprintf(msg + strlen(msg), "Peripheral (buffer write) initialized in %lu us, speed: %.2f kSamples/sec", 
            peripheral_end_time - peripheral_start_time, write_speed);
    Serial.println(msg);
}

void transfer_to_audio_buffer() {
    if (audio_available()) {
        int16_t* block = get_audio_block();
        if (block) {
            for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
                audio_buffer[write_idx] = block[i];
                write_idx = (write_idx + 1) % (SLOT_DURATION_MAX * SAMPLE_RATE_NATIVE);
            }
            queue_in.freeBuffer();
        }
    }
}