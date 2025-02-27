#ifndef TRANSMIT_H
#define TRANSMIT_H

#include "config.h"

void start_transmission(const char* message);
bool is_transmitting();
void generate_tx_audio(int16_t* block);

#endif