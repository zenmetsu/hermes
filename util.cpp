#include "util.h"

void ft8_crc(int msg[], int msglen, int crc[]) {
    // Placeholder: Fill crc with zeros
    for (int i = 0; i < 12; i++) crc[i] = 0;
}

void writewav(const std::vector<double>& samples, const char* filename, int rate) {
    // Placeholder: Do nothing (not used in Teensy)
}