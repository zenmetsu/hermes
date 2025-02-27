#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

void print_timestamp(char* buffer, size_t size);
time_t get_teensy_time();

#endif