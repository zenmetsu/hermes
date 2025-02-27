#include <Audio.h>
#include <Arduino.h>
#include <TimeLib.h>
#include <ILI9341_t3n.h>
#include "audio_io.h"
#include "signal_proc.h"
#include "transmit.h"
#include "demodulate_normal.h"
#include "display.h"
#include "utils.h"
#include "config.h"

AudioControlSGTL5000 audioShield;
IntervalTimer bufferTimer;

static time_t start_time;
static bool first_demod_done = false;

// ILI9340C SPI0 Pins (your tested config)
#define TFT_DC   29
#define TFT_CS   28
#define TFT_RST  24
#define TFT_MOSI 11
#define TFT_SCK  13
#define TFT_MISO 12

ILI9341_t3n tft = ILI9341_t3n(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCK, TFT_MISO);

void transfer_audio_buffer() {
    transfer_to_audio_buffer();
    update_display_state("Listening");
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
    
    char msg[128];
    print_timestamp(msg, sizeof(msg));
    strcat(msg, "Before init_display");
    Serial.println(msg);
    
    init_display(&tft);
    
    print_timestamp(msg, sizeof(msg));
    strcat(msg, "After init_display");
    Serial.println(msg);

    setSyncProvider(get_teensy_time);
    start_time = get_teensy_time();

    bufferTimer.begin(transfer_audio_buffer, 2900);

    pinMode(LED_STATUS_PIN, OUTPUT);
    digitalWrite(LED_STATUS_PIN, HIGH);
    delay(500);
    digitalWrite(LED_STATUS_PIN, LOW);
    
    print_timestamp(msg, sizeof(msg));
    strcat(msg, "Setup completed");
    Serial.println(msg);
}

void loop() {
    time_t now = get_teensy_time();
    float seconds = second(now) + (millis() % 1000) / 1000.0;

    if ((seconds >= 2.36 && seconds < 2.37) || 
        (seconds >= 17.36 && seconds < 17.37) || 
        (seconds >= 32.36 && seconds < 32.37) || 
        (seconds >= 47.36 && seconds < 47.37)) {
        if (!first_demod_done && (now - start_time) >= 17.36) {
            update_display_state("Demodulating");
            demodulate_js8_normal();
            first_demod_done = true;
            update_display_state("Listening");
        } else if (first_demod_done) {
            update_display_state("Demodulating");
            demodulate_js8_normal();
            update_display_state("Listening");
        }
    }

    delay(1);
}