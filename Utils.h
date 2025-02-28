#ifndef UTILS_H
#define UTILS_H

#include <TimeLib.h>
#include "Config.h"

// Class for utility functions
// Purpose: Provides timestamped debug messages and other helper functions
class Utils {
public:
  Utils();          // Constructor
  void begin();     // Initialize RTC
  void debugPrint(const char* message);  // Print timestamped debug message
  bool isRTCActive();  // Check if RTC is active (public accessor)

private:
  bool rtcActive;   // Flag for RTC status
  char timestamp[20];  // Buffer for timestamp string
};

// Teensy 4.1 RTC time provider (defined outside class to avoid scope issues)
time_t getTeensy3Time();

#endif