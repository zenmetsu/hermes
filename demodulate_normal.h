#ifndef DEMODULATE_NORMAL_H
#define DEMODULATE_NORMAL_H

#include "config.h"
#include <vector>

struct Conversation {
    float center_freq;
    int symbols[SYMBOLS_PER_MESSAGE];
    size_t sym_count;
    bool active;
};

void demodulate_js8_normal();

#endif