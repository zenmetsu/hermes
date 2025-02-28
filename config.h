#ifndef CONFIG_H
#define CONFIG_H

// Configuration constants for the Hermes project (JS8 Teensy Transceiver)
// Purpose: Centralize all constants for easy modification and reference

// Display pin definitions
#define TFT_DC   29
#define TFT_CS   28
#define TFT_RST  24
#define TFT_MOSI 11
#define TFT_SCK  13
#define TFT_MISO 12

// Display dimensions
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240
#define WATERFALL_HEIGHT (SCREEN_HEIGHT / 4)  // 60 pixels

// Audio parameters
#define SAMPLE_RATE 44100
#define BUFFER_SECONDS 60
#define BUFFER_SIZE (SAMPLE_RATE * BUFFER_SECONDS)  // 2,646,000 samples

// Frequency range for waterfall
#define MIN_FREQ 300    // Hz
#define MAX_FREQ 3600   // Hz

// Color definitions (RGB565 format)
#define COLOR_BLACK     0x0000
#define COLOR_WHITE     0xFFFF
#define COLOR_DARKGREY  0x4208  // R:2 G:2 B:2
#define COLOR_GREY      0x8410  // R:4 G:4 B:4
#define COLOR_LIGHTGREY 0xC618  // R:6 G:6 B:6
#define COLOR_SEAFOAM   0x4EF4  // R:2 G:7 B:5

// JS8 Normal mode parameters
#define JS8_NORMAL_BAUD 6.25f  // Baud rate (symbols/sec)
#define JS8_NORMAL_TONES 79    // Number of 8FSK tones
#define JS8_NORMAL_DURATION (JS8_NORMAL_TONES / JS8_NORMAL_BAUD)  // 12.64s
#define JS8_SEARCH_DURATION 17.36f  // 12.64s + 2.36s TX delay + 2.36s RX delay
#define JS8_RESAMPLE_RATE 12800  // Hz for 6.25 Hz/bin with FFT 2048
#define JS8_FFT_SIZE 2048
#define JS8_RESAMPLE_SIZE ((uint32_t)(JS8_RESAMPLE_RATE * JS8_SEARCH_DURATION))  // 222,208 samples
#define JS8_COSTAS_ARRAY {4, 2, 5, 6, 1, 3, 0}  // JS8 Normal mode Costas array

#endif