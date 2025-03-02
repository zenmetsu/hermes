#ifndef JS8_H
#define JS8_H

#include <vector>
#include "util.h"

typedef int (*cb_t)(int *a87, double hz0, double hz1, double off, const char *comment, double snr, int pass, int correct_bits);
void entry(double xsamples[], int nsamples, int start, int rate, double min_hz, double max_hz, int hints1[], int hints2[], cb_t cb);

#endif