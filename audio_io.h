#ifndef AUDIO_IO_H
#define AUDIO_IO_H

#include <Audio.h>

extern AudioInputI2S i2s_in;    // Standard I2S input
extern AudioRecordQueue queue_in; // Queue for input audio blocks
extern AudioPlayQueue queue_out;  // Queue for output audio blocks
extern AudioOutputI2S i2s_out;    // I2S output to SGTL5000
extern AudioConnection patchCord1; // queue_out to i2s_out (left)
extern AudioConnection patchCord2; // i2s_in to queue_in (left)

void init_audio_io();
bool audio_available();
int16_t* get_audio_block();
void play_audio_block(int16_t* block);

#endif