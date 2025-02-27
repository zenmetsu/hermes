#include <Arduino.h>
#include "display.h"
#include "utils.h"

// Display buffer (double-buffered, 320x240 RGB565)
EXTMEM uint16_t display_buffer[2][320 * 240]; // ~300 KB total (153,600 bytes each)
uint8_t active_buffer = 0;    // 0 or 1, current drawing buffer

void init_display(ILI9341_t3n* tft) {
    char msg[128];
    print_timestamp(msg, sizeof(msg));
    strcat(msg, "Before tft begin");
    Serial.println(msg);

    tft->begin(60000000); // 60 MHz SPI clock speed

    print_timestamp(msg, sizeof(msg));
    strcat(msg, "After tft begin");
    Serial.println(msg);

    tft->setRotation(3);  // 90° counterclockwise (landscape, 240x320)

    print_timestamp(msg, sizeof(msg));
    strcat(msg, "Before frame buffer setup");
    Serial.println(msg);

    tft->useFrameBuffer(true);
    tft->setFrameBuffer(display_buffer[0]); // Start with buffer 0

    print_timestamp(msg, sizeof(msg));
    strcat(msg, "After frame buffer setup");
    Serial.println(msg);

    // Test PSRAM access
    print_timestamp(msg, sizeof(msg));
    strcat(msg, "Before PSRAM test");
    Serial.println(msg);
    display_buffer[0][0] = 0x1234;
    if (display_buffer[0][0] == 0x1234) {
        print_timestamp(msg, sizeof(msg));
        strcat(msg, "PSRAM test passed");
        Serial.println(msg);
    } else {
        print_timestamp(msg, sizeof(msg));
        strcat(msg, "PSRAM test failed");
        Serial.println(msg);
    }

    // Clear buffers to black
    memset(display_buffer[0], 0, sizeof(display_buffer[0]));
    memset(display_buffer[1], 0, sizeof(display_buffer[1]));
}

void draw_welcome_message(ILI9341_t3n* tft) {
    uint16_t* buffer = display_buffer[active_buffer];
    tft->setFrameBuffer(buffer);

    // Fill background
    tft->fillScreen(COLOR_BLACK);

    // Welcome message (centered for 240x320 landscape)
    tft->setTextColor(COLOR_SEAFOAM_GREEN);
    tft->setTextSize(3);
    tft->setCursor(240 / 2 - (10 * 18) / 2, 320 / 2 - 40); // "JS8 Teensy" ~10 chars, 18px/char
    tft->print("JS8 Teensy");

    tft->setTextColor(COLOR_WHITE);
    tft->setTextSize(2);
    tft->setCursor(240 / 2 - (7 * 12) / 2, 320 / 2); // "Welcome!" ~7 chars, 12px/char
    tft->print("Welcome!");

    tft->setTextColor(COLOR_GREY_LIGHT);
    tft->setTextSize(1);
    tft->setCursor(240 / 2 - (4 * 6) / 2, 320 / 2 + 20); // "v0.1" ~4 chars, 6px/char
    tft->print("v");
    tft->print(VERSION);
}

void update_display_state(const char* state, ILI9341_t3n* tft) {
    if (!tft) tft = &::tft; // Use global tft if nullptr

    uint16_t* buffer = display_buffer[active_buffer];
    tft->setFrameBuffer(buffer);

    // Clear previous state area (adjusted for 240x320)
    tft->fillRect(10, 200, 220, 40, COLOR_BLACK); // Bottom area for state

    // Draw new state
    tft->setTextColor(COLOR_GREY_DARK);
    tft->setTextSize(2);
    tft->setCursor(10, 200);
    tft->print(state);

    // Swap buffers (no update here, handled by update_display_full)
    active_buffer = (active_buffer + 1) % 2;
}

void update_display_full(const char* callsign, const char* date, const char* time, const char* state, ILI9341_t3n* tft) {
    if (!tft) tft = &::tft; // Use global tft if nullptr

    uint16_t* buffer = display_buffer[active_buffer];
    tft->setFrameBuffer(buffer);

    // Rebuild buffer from scratch
    tft->fillScreen(COLOR_BLACK);

    // Operator info
    tft->setTextColor(COLOR_WHITE);
    tft->setTextSize(2);
    int line_height = 20;
    int start_y = 10;
    tft->setCursor(240 / 2 - (strlen(callsign) * 12) / 2, start_y);
    tft->print(callsign);
    tft->setCursor(240 / 2 - (strlen(date) * 12) / 2, start_y + line_height);
    tft->print(date);
    tft->setCursor(240 / 2 - (strlen(time) * 12) / 2, start_y + 2 * line_height);
    tft->print(time);

    // State
    tft->fillRect(10, 200, 220, 40, COLOR_BLACK); // Clear state area
    tft->setTextColor(COLOR_GREY_DARK);
    tft->setTextSize(2);
    tft->setCursor(10, 200);
    tft->print(state);

    // Swap buffers and push
    active_buffer = (active_buffer + 1) % 2;
    tft->updateScreen();
}