#ifndef CONFIG_H
#define CONFIG_H

// Display pin definitions
#define TFT_DC   29
#define TFT_CS   28
#define TFT_RST  24
#define TFT_MOSI 11
#define TFT_SCK  13
#define TFT_MISO 12

// UI regions (top to bottom)
#define VISUALIZER_Y 0
#define VISUALIZER_HEIGHT 60
#define INDICATOR_Y (VISUALIZER_Y + VISUALIZER_HEIGHT)
#define INDICATOR_HEIGHT 20
#define STATUS_Y (INDICATOR_Y + INDICATOR_HEIGHT)
#define STATUS_HEIGHT 40
#define INPUT_Y (STATUS_Y + STATUS_HEIGHT)
#define INPUT_HEIGHT 20
#define OUTPUT_Y (INPUT_Y + INPUT_HEIGHT)
#define OUTPUT_HEIGHT 100
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// Audio sampling parameters
#define SAMPLE_RATE 44100U
#define BUFFER_SECONDS 60U
#define BUFFER_SIZE (SAMPLE_RATE * BUFFER_SECONDS)
#define HALF_BUFFER (SAMPLE_RATE * 30U)
#define SAMPLES_PER_ROW (HALF_BUFFER / VISUALIZER_HEIGHT)

// Color definitions (RGB565)
#define COLOR_BLACK     0x0000
#define COLOR_WHITE     0xFFFF
#define COLOR_DARKGREY  0x4208
#define COLOR_GREY      0x8410
#define COLOR_LIGHTGREY 0xC618
#define COLOR_SEAFOAM   0x4EF4
#define COLOR_GREEN   0x07E0
#define COLOR_ORANGE  0xFD20
#define COLOR_RED     0xF800 // Added (pure red in RGB565)

// Frequency range
#define MIN_FREQ 300    // Hz
#define MAX_FREQ 3600   // Hz

// Screen update interval
#define SCREEN_UPDATE_INTERVAL 33  // ~30 Hz (33.33 ms)

#endif