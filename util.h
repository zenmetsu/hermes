#ifndef UTIL_H
#define UTIL_H

#include <Arduino.h>
#include <TimeLib.h>

// Stub for ft8_crc (to be replaced with actual implementation)
void ft8_crc(int msg[], int msglen, int crc[]);

// Stub for writewav (unused in Teensy, but needed for pack.cc)
void writewav(const std::vector<double>& samples, const char* filename, int rate);

// Replacement for now() using Teensy RTC
inline double now() {
    return (double)Teensy3Clock.get() + (micros() % 1000000) / 1000000.0;
}

#endif