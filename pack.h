// pack.h
#ifndef PACK_H
#define PACK_H

#include <vector>
#include <string>

// Note: Functions generate FSK signals at a target sample rate (e.g., 12800 Hz for Teensy Audio Library).
void ldpc_encode(int plain[87], int codeword[174]);

std::vector<double> pack_text(std::string text, int &consumed, int itype, int rate, double hz);

std::vector<double> pack_cq(std::string call, std::string grid, int rate, double hz);

std::vector<double> pack_directed(std::string my_call, std::string other_call,
                                  int cmd, int extra, int itype, int rate, double hz);

bool pack_call_28(std::string call, unsigned int &x);

unsigned int pack_grid(std::string grid);

#endif