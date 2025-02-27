#include <Arduino.h>  // Added to define Serial and other Arduino-specific functions
#include "utils.h"

// Initialize utility functions
void init_utils() {
    // Ensure Serial is initialized for debugging
    if (!Serial) {
        Serial.begin(9600);
        while (!Serial) {
            ; // Wait for Serial to connect
        }
    }
}

// Get the current time from the Teensy RTC
time_t get_teensy_time() {
    return Teensy3Clock.get();
}

// Set the Teensy RTC to a specific time
void set_teensy_time(time_t t) {
    Teensy3Clock.set(t);
    setTime(t);  // Sync the TimeLib internal clock with the RTC
}

// Print a debug message with a timestamp
void debug_print(const char* message) {
    if (Serial) {
        char time_str[20];
        time_t now = get_teensy_time();
        sprintf(time_str, "%04d-%02d-%02d %02d:%02d:%02d",
                year(now), month(now), day(now),
                hour(now), minute(now), second(now));
        Serial.print("[");
        Serial.print(time_str);
        Serial.print("] ");
        Serial.println(message);
    }
}