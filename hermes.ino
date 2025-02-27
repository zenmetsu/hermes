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
#include "operator.h"

AudioControlSGTL5000 audioShield;
IntervalTimer bufferTimer;
IntervalTimer displayTimer;

static time_t start_time;
static bool first_demod_done = false;
static volatile bool update_display_flag = false; // Set by display ISR

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

void display_update_isr() {
    update_display_flag = true; // Flag for loop to handle
}

void show_welcome() {
    char msg[128];
    print_timestamp(msg, sizeof(msg));
    strcat(msg, "Drawing welcome message");
    Serial.println(msg);

    draw_welcome_message(&tft);
    tft.updateScreen();

    print_timestamp(msg, sizeof(msg));
    strcat(msg, "Welcome displayed, waiting 5s");
    Serial.println(msg);

    delay(5000);

    print_timestamp(msg, sizeof(msg));
    strcat(msg, "Clearing welcome message");
    Serial.println(msg);

    tft.fillScreen(COLOR_BLACK);
    tft.updateScreen();
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

    print_timestamp(msg, sizeof(msg));
    strcat(msg, "Before setSyncProvider");
    Serial.println(msg);
    setSyncProvider(get_teensy_time);

    print_timestamp(msg, sizeof(msg));
    strcat(msg, "After setSyncProvider");
    Serial.println(msg);
    start_time = get_teensy_time();

    print_timestamp(msg, sizeof(msg));
    strcat(msg, "Before bufferTimer");
    Serial.println(msg);
    bufferTimer.begin(transfer_audio_buffer, 2900);

    print_timestamp(msg, sizeof(msg));
    strcat(msg, "After bufferTimer");
    Serial.println(msg);

    print_timestamp(msg, sizeof(msg));
    strcat(msg, "Before displayTimer");
    Serial.println(msg);
    displayTimer.begin(display_update_isr, 33333); // 33,333 µs = ~30 FPS

    print_timestamp(msg, sizeof(msg));
    strcat(msg, "After displayTimer");
    Serial.println(msg);

    print_timestamp(msg, sizeof(msg));
    strcat(msg, "Setup completed");
    Serial.println(msg);

    show_welcome();
}

void loop() {
    if (update_display_flag) {
        update_display_flag = false;

        // Prepare operator info
        char date_str[11], time_str[9];
        time_t now = get_teensy_time();
        struct tm* timeinfo = localtime(&now);
        strftime(date_str, sizeof(date_str), "%Y-%m-%d", timeinfo);
        strftime(time_str, sizeof(time_str), "%H:%M:%S", timeinfo);

        // Update full display
        update_display_full(CALLSIGN, date_str, time_str, "Listening"); // State updated by ISR
    }

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