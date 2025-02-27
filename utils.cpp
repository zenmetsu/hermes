#include <Arduino.h>
#include "utils.h"
#include <TimeLib.h>

time_t get_teensy_time() {
    return Teensy3Clock.get();
}

void print_timestamp(char* buffer, size_t size) {
    time_t now = get_teensy_time();
    struct tm* timeinfo = localtime(&now);
    strftime(buffer, size, "[%Y-%m-%d %H:%M:%S] ", timeinfo);
}