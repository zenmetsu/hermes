#ifndef SIGNAL_PROC_H
#define SIGNAL_PROC_H

#include <Audio.h>
#include "config.h"
#include "audio_io.h"

extern float tone_frequencies[NUM_TONES];
extern int16_t audio_buffer[SLOT_DURATION_MAX * SAMPLE_RATE_NATIVE];
extern volatile size_t write_idx;

void init_signal_proc();
void report_buffer_write_speed();
void transfer_to_audio_buffer(); // New function to move queue data
void start_transmission(const char* message);
bool is_transmitting();
void generate_tx_audio(int16_t* block);
void demodulate_js8_normal();

#endif