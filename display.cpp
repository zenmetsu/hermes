#include <Arduino.h>
#include "display.h"
#include "utils.h"

// Display buffer (double-buffered, 320x240 RGB565)
EXTMEM uint16_t display_buffer[2][320 * 240]; // ~300 KB total (153,600 bytes each)
static volatile uint8_t active_buffer = 0;    // 0 or 1, current drawing buffer

void init_display(ILI9341_t3n* tft) {
    char msg[128];
    print_timestamp(msg, sizeof(msg));
    strcat(msg, "Before tft begin");
    Serial.println(msg);

    tft->begin(60000000); // 60 MHz SPI clock speed
    
    print_timestamp(msg, sizeof(msg));
    strcat(msg, "After tft begin");
    Serial.println(msg);

    tft->setRotation(0);  // Adjust as needed (0-3)
    
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
    print_timestamp(msg, sizeof(msg));
    strcat(msg, "Before memset buffer 0");
    Serial.println(msg);
    memset(display_buffer[0], 0, sizeof(display_buffer[0]));
    
    print_timestamp(msg, sizeof(msg));
    strcat(msg, "After memset buffer 0");
    Serial.println(msg);
    memset(display_buffer[1], 0, sizeof(display_buffer[1]));
    
    print_timestamp(msg, sizeof(msg));
    strcat(msg, "After memset buffer 1");
    Serial.println(msg);

    // Draw welcome message on buffer 0
    print_timestamp(msg, sizeof(msg));
    strcat(msg, "Before draw welcome");
    Serial.println(msg);
    draw_welcome_message(tft);
    
    print_timestamp(msg, sizeof(msg));
    strcat(msg, "After draw welcome");
    Serial.println(msg);

    // Initial DMA update
    print_timestamp(msg, sizeof(msg));
    strcat(msg, "Before DMA update");
    Serial.println(msg);
    update_display_dma(tft);
    
    print_timestamp(msg, sizeof(msg));
    strcat(msg, "After DMA update");
    Serial.println(msg);
}

void draw_welcome_message(ILI9341_t3n* tft) {
    uint16_t* buffer = display_buffer[active_buffer];
    tft->setFrameBuffer(buffer);
    
    // Fill background (black already from memset)
    tft->fillScreen(COLOR_BLACK);
    
    // Welcome message (centered-ish, 320x240)
    tft->setTextColor(COLOR_SEAFOAM_GREEN);
    tft->setTextSize(3);
    tft->setCursor(40, 80); // Approx center
    tft->print("JS8 Teensy");
    
    tft->setTextColor(COLOR_WHITE);
    tft->setTextSize(2);
    tft->setCursor(70, 120);
    tft->print("Welcome!");
    
    tft->setTextColor(COLOR_GREY_LIGHT);
    tft->setTextSize(1);
    tft->setCursor(140, 150);
    tft->print("v1.0");
}

void update_display_state(const char* state, ILI9341_t3n* tft) {
    if (!tft) tft = &::tft; // Use global tft if nullptr
    
    uint16_t* buffer = display_buffer[active_buffer];
    tft->setFrameBuffer(buffer);
    
    // Clear previous state area
    tft->fillRect(20, 180, 280, 40, COLOR_BLACK); // Bottom area for state
    
    // Draw new state
    tft->setTextColor(COLOR_GREY_DARK);
    tft->setTextSize(2);
    tft->setCursor(20, 180);
    tft->print(state);

    // Swap buffers and trigger DMA
    active_buffer = (active_buffer + 1) % 2;
    update_display_dma(tft);
}

void update_display_dma(ILI9341_t3n* tft) {
    char msg[128];
    print_timestamp(msg, sizeof(msg));
    strcat(msg, "DMA transfer started");
    Serial.println(msg);

    // Use previous buffer for DMA transfer
    tft->setFrameBuffer(display_buffer[(active_buffer + 1) % 2]);
    tft->updateScreenAsync(); // Non-blocking DMA transfer
    
    print_timestamp(msg, sizeof(msg));
    strcat(msg, "Waiting for DMA completion");
    Serial.println(msg);
    
    while (tft->asyncUpdateActive()) {} // Wait for DMA completion (temporary)
    
    print_timestamp(msg, sizeof(msg));
    strcat(msg, "DMA transfer completed");
    Serial.println(msg);
}