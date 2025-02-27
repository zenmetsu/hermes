#include <Audio.h>
#include <Arduino.h>
#include "audio_io.h"
#include "signal_proc.h"
#include "utils.h"
#include "config.h"

AudioControlSGTL5000 audioShield;
IntervalTimer bufferTimer; // Timer to transfer queue data to buffer

static time_t start_time;
static bool first_demod_done = false;

void transfer_audio_buffer() {
    transfer_to_audio_buffer(); // Defined in signal_proc.cpp
}

void setup() {
    pinMode(PTT_PIN, OUTPUT);
    digitalWrite(PTT_PIN, LOW);

    AudioMemory(AUDIO_MEMORY_BLOCKS);
    audioShield.enable();
    audioShield.inputSelect(AUDIO_INPUT_LINEIN);
    audioShield.lineInLevel(5);
    audioShield.lineOutLevel(13);
    init_audio_io();

    init_signal_proc();
    init_utils();

    setSyncProvider(get_teensy_time);
    start_time = get_teensy_time();
    debug_print("JS8 on Teensy 4.1 initialized.");

    // Start timer to transfer audio every ~2.9 ms (344 Hz)
    bufferTimer.begin(transfer_audio_buffer, 2900); // µs

    pinMode(LED_STATUS_PIN, OUTPUT);
    digitalWrite(LED_STATUS_PIN, HIGH);
    delay(500);
    digitalWrite(LED_STATUS_PIN, LOW);
}

void loop() {
    time_t now = get_teensy_time();
    float seconds = second(now) + (millis() % 1000) / 1000.0;

    if ((seconds >= 2.36 && seconds < 2.37) || 
        (seconds >= 17.36 && seconds < 17.37) || 
        (seconds >= 32.36 && seconds < 32.37) || 
        (seconds >= 47.36 && seconds < 47.37)) {
        if (!first_demod_done && (now - start_time) >= 17.36) {
            demodulate_js8_normal();
            first_demod_done = true;
        } else if (first_demod_done) {
            demodulate_js8_normal();
        }
    }

    delay(1);
}