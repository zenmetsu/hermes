#ifndef UTILS_H
#define UTILS_H

#include <TimeLib.h>  // For time_t and time-related functions

// Function prototypes
void init_utils();              // Initialize utility functions
time_t get_teensy_time();       // Get the current time from the Teensy RTC
void set_teensy_time(time_t t); // Set the Teensy RTC time
void debug_print(const char* message); // Print a debug message via Serial

#endif  // UTILS_H