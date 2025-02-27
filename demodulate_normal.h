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

static const int COSTAS_PATTERN[7] = {4, 2, 5, 6, 1, 3, 0};

void demodulate_js8_normal();

#endif