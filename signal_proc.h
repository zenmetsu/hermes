#ifndef SIGNAL_PROC_H
#define SIGNAL_PROC_H

#include <Audio.h>
#include "config.h"
#include "audio_io.h"

extern int16_t audio_buffer[SLOT_DURATION_MAX * SAMPLE_RATE_NATIVE];
extern int16_t resampled_buffer[RESAMPLED_BUFFER_SIZE];
extern volatile size_t write_idx;
extern float tone_frequencies[NUM_TONES];

void init_signal_proc();
void report_buffer_write_speed();
void transfer_to_audio_buffer();
void init_fft(); // Added for FFT initialization

#endif