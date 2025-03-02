// unpack.cc
// Robert Morris, AB1HL

#include "unpack.h"
#include <string>
#include <vector>
#include <Arduino.h>
#include <cassert> // Added for assert()
#include "huffman_table.h"

static std::vector<std::string> words; // Empty for now; embed later if needed

void load_words() {
    // Stub: No file I/O on Teensy; words remain empty
}

unsigned long long un(const int a87[87], int start, int n) {
    assert(n > 0 && n <= 64 && start + n <= 72);
    unsigned long long x = 0;
    for (int i = 0; i < n; i++) {
        x <<= 1;
        x += a87[start + i];
    }
    return x;
}

std::string unpack_huffman(const int a87[87]) {
    int end = 71;
    while (end > 0 && a87[end] != 0) end -= 1;

    std::string ret;
    int i = 2;
    while (i < end) {
        int j;
        for (j = 0; ht[j].a; j++) {
            int k;
            for (k = 0; ht[j].b[k] && i + k < 72; k++) {
                if (ht[j].b[k] == '1') {
                    if (a87[i + k] != 1) break;
                } else {
                    if (a87[i + k] != 0) break;
                }
            }
            if (ht[j].b[k] == '\0') break;
        }
        if (ht[j].a) {
            ret += ht[j].a;
            i += strlen(ht[j].b);
        } else {
            ret += "?";
            i++;
        }
    }
    if (a87[73]) ret += "<>";
    return ret;
}

std::string unpack(const int a87[87], std::string& other_call) {
    other_call.erase();
    load_words(); // Stubbed

    if (a87[0] == 1 && a87[1] == 0) {
        return unpack_huffman(a87); // Minimal support for now
    } else {
        return "Unsupported message type"; // Stub others until needed
    }
}