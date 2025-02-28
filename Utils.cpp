#include "Utils.h"
#include <Arduino.h>  // For Serial
#include <cstdio>     // For snprintf

Utils::Utils() : rtcActive(false) {
  // Constructor initializes RTC flag
}

void Utils::begin() {
  // Initialize Teensy 4.1 RTC
  setSyncProvider(getTeensy3Time);  // Use Teensy RTC as time source
  if (timeStatus() == timeSet) {
    rtcActive = true;
  } else {
    Serial.println("RTC not set");
  }
}

void Utils::debugPrint(const char* message) {
  // Print timestamped debug message to Serial
  if (rtcActive) {
    snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02d %02d:%02d:%02d",
             year(), month(), day(), hour(), minute(), second());
    Serial.print(timestamp);
    Serial.print(" - ");
  } else {
    Serial.print("No RTC - ");
  }
  Serial.println(message);
}

bool Utils::isRTCActive() {
  // Return RTC status
  return rtcActive;
}

time_t getTeensy3Time() {
  // Teensy 4.1 RTC time provider
  return Teensy3Clock.get();
}