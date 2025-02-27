#include <Arduino.h>
#include "audio_io.h"
#include "config.h"
#include "utils.h"

AudioInputI2S i2s_in;         // Standard I2S input
AudioRecordQueue queue_in;    // Queue to record left channel input
AudioPlayQueue queue_out;     // Queue to play to left channel output
AudioOutputI2S i2s_out;       // Stereo output to SGTL5000
AudioConnection patchCord1(queue_out, 0, i2s_out, 0); // Queue to left channel output
AudioConnection patchCord2(i2s_in, 0, queue_in, 0);   // Left channel input to queue

void init_audio_io() {
    queue_in.begin();  // Start recording from Line In (left channel)
}

bool audio_available() {
    return queue_in.available() > 0;
}

int16_t* get_audio_block() {
    if (audio_available()) {
        return queue_in.readBuffer();
    }
    return nullptr;
}

void play_audio_block(int16_t* block) {
    if (block != nullptr) {
        int16_t* play_buffer = queue_out.getBuffer();
        if (play_buffer != nullptr) {
            memcpy(play_buffer, block, AUDIO_BLOCK_SAMPLES * sizeof(int16_t));
            queue_out.playBuffer();
        }
    }
}