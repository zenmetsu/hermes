#include "util.h"
#include <TimeLib.h>
#include <math.h>
#include <cassert> // Added for assert()

double hermes_now() { // Renamed from now()
    return (double)Teensy3Clock.get() + (micros() % 1000000) / 1000000.0;
}

// ... (rest unchanged until cycle_second)
double cycle_second() {
    double tt = hermes_now(); // Updated to hermes_now()
    double st = trunc(tt / 15.0) * 15;
    return tt - st;
}

void ft8_crc(int msg1[], int msglen, int out[12]) {
    int div[] = {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0};
    int* msg = (int*)malloc(sizeof(int) * (msglen + 12));
    for (int i = 0; i < msglen + 12; i++) {
        msg[i] = (i < msglen) ? msg1[i] : 0;
    }
    for (int i = 0; i < msglen; i++) {
        if (msg[i]) {
            for (int j = 0; j < 13; j++) {
                msg[i + j] = (msg[i + j] + div[j]) % 2;
            }
        }
    }
    for (int i = 0; i < 12; i++) {
        out[i] = msg[msglen + i];
    }
    out[10] ^= 1;
    out[8] ^= 1;
    out[6] ^= 1;
    free(msg);
}