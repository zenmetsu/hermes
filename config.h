#ifndef CONFIG_H
#define CONFIG_H

constexpr const char* VERSION = "0.11"; // Program version

#define PTT_PIN 2
#define LED_STATUS_PIN 13
#define SAMPLE_RATE_NATIVE 44100  // Native sample rate (44.1 kHz)
#define SAMPLE_RATE_RESAMPLED 12800  // Resampled rate (12.8 kHz)
#define SLOT_DURATION_MAX 60      // Max buffer duration (60s for circular buffer)
#define SLOT_DURATION_NORMAL 15   // Normal mode slot duration (seconds)
#define SLOT_DURATION_NORMAL_WITH_SKEW 17.36  // 15s + 2.36s skew
#define NUM_TONES 8
#define TONE_SPACING 6.25f
#define BASE_FREQUENCY 1000.0f
#define SYMBOL_RATE_NORMAL 6.25f  // Normal mode baud rate (symbols per second)
#define FFT_SIZE 2048             // FFT size for 6.25 Hz/bin at 12.8 kHz
#define SYMBOL_SAMPLES_NORMAL (SAMPLE_RATE_RESAMPLED / SYMBOL_RATE_NORMAL) // 2048 samples at 12.8 kHz
#define SYMBOLS_PER_MESSAGE 79
#define SYMBOLS_PER_SLOT ((SLOT_DURATION_NORMAL_WITH_SKEW * SYMBOL_RATE_NORMAL)) // ~108.5, rounded down
#define AUDIO_MEMORY_BLOCKS 20
#define RESAMPLED_BUFFER_SIZE 222208 // ~222,208 samples

#endif