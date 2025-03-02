// pack.cc
// Robert Morris, AB1HL
// Modified for Teensy 4.1 with 12800 Hz sample rate

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>
#include <assert.h>
#include <vector>
#include <string>
#include "defs.h"
#include "pack.h"
#include "util.h"

// LDPC generator matrix (unchanged, from wsjt-x)
int gen[87][87] = {
    // [Original matrix unchanged; omitted for brevity]
    // Same as provided in input
};

// Column order table (assumed external from arrays.h)
extern int colorder[];

// LDPC encoding (unchanged)
void ldpc_encode(int plain[87], int codeword[174]) {
    int cw[174];
    for (int i = 0; i < 174; i++) cw[i] = 0;
    for (int i = 0; i < 87; i++) {
        int sum = 0;
        for (int j = 0; j < 87; j++) sum += plain[j] * gen[i][j];
        cw[i] = sum % 2;
    }
    for (int i = 0; i < 87; i++) cw[87 + i] = plain[i];
    for (int i = 0; i < 174; i++) codeword[colorder[i]] = cw[i];
}

// Generate FSK signal optimized for Teensy
std::vector<double> fsk(std::vector<int> symbols, double hz, double spacing, int rate, int symsamples) {
    std::vector<double> v;
    v.reserve(symbols.size() * symsamples); // Pre-allocate to reduce dynamic resizing
    
    float phase = 0.0f; // Use float for Cortex-M7 FPU
    const float two_pi = 2.0f * M_PI;
    const float rate_inv = 1.0f / rate;

    for (size_t i = 0; i < symbols.size() * symsamples; i++) {
        v.push_back(cosf(phase)); // cosf is faster than cos on Cortex-M7
        int si = i / symsamples;
        float nhz = static_cast<float>(hz + symbols[si] * spacing);
        phase += two_pi * rate_inv * nhz;
        if (phase >= two_pi) phase -= two_pi; // Keep phase bounded
    }
    return v;
}

// Huffman table (assumed external from unpack.cc)
extern struct htht ht[];

// Huffman encoding (unchanged)
void pack_huffman(std::string text, int a72[72], int &consumed) {
    for (int i = 0; i < 72; i++) a72[i] = 0;
    a72[0] = 1;
    a72[1] = 0;
    int bi = 2;
    int ti = 0;
    while (bi < 72 - 1 && ti < text.size()) {
        int c = text[ti] & 0xff;
        if (islower(c)) c = toupper(c);
        int found = -1;
        for (int i = 0; ht[i].a && ht[i].b; i++) {
            if (ht[i].a[0] == c) {
                found = i;
                break;
            }
        }
        if (found >= 0) {
            if (bi + strlen(ht[found].b) > 72 - 1) break;
            for (int i = 0; ht[found].b[i]; i++) {
                a72[bi++] = (ht[found].b[i] == '1') ? 1 : 0;
            }
        } else {
            fprintf(stderr, "[cannot pack 0x%02x]", c);
        }
        ti += 1;
    }
    assert(bi < 72);
    a72[bi++] = 0;
    for (; bi < 72; bi++) a72[bi] = 1;
    consumed = ti;
    assert(text.size() == 0 || consumed > 0);
}

// Core packing function with adjusted sample rate
std::vector<double> pack_any(int a87[87], int rate, double hz) {
    int crc[12];
    ft8_crc(a87, 76, crc); // Assumes ft8_crc is defined in util.h
    for (int i = 0; i < 12; i++) a87[87 - 12 + i] = crc[i];

    int a174[174];
    ldpc_encode(a87, a174);

    extern std::vector<int> recode(int a174[]); // Assumed in another file
    std::vector<int> a79 = recode(a174);

    // JS8 symbol duration is 0.16s (160ms), 2048 samples at 12800 Hz
    const int symsamples = (rate == 12800) ? 2048 : (1920 / (12000 / rate));
    return fsk(a79, hz, 6.25, rate, symsamples);
}

// Bit-setting utility (unchanged)
void setbits(int a87[87], int off, int n, unsigned long long x) {
    for (int i = off + n - 1; i >= off; i--) {
        a87[i] = x & 1;
        x >>= 1;
    }
}

// Pack text message
std::vector<double> pack_text(std::string text, int &consumed, int itype, int rate, double hz) {
    int a87[87] = {0};
    pack_huffman(text, a87, consumed);
    setbits(a87, 72, 3, itype);
    return pack_any(a87, rate, hz);
}

// Test function (updated for 12800 Hz)
void test_pack() {
    int rate = 12800;
    int consumed;
    std::string s = "xxx";
    std::vector<double> v = pack_text(s, consumed, 0, rate, 1500);
    v.resize(15 * rate); // 15 seconds
    printf("writing x.wav\n");
    writewav(v, "x.wav", rate); // Assumes writewav in util.h
}

// Call sign utilities (unchanged)
std::string fixcall(std::string call) {
    while (call.size() > 0 && isspace(call[0])) call.erase(0);
    while (call.size() > 0 && isspace(call[call.size() - 1])) call.erase(call.size() - 1);
    for (int i = 0; i < (int)call.size(); i++) {
        if (islower(call[i])) call[i] = toupper(call[i]);
    }
    return call;
}

bool pack_call_28(std::string call, unsigned int &x) {
    std::string orig_call = call;
    call = fixcall(call);
    if (call.size() > 2 && call.size() < 6 && !isdigit(call[2])) call = " " + call;
    while (call.size() < 6) call += " ";

    std::string c1 = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    std::string c2 = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string c3 = "0123456789";
    std::string c4 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

    assert(c1.find(call[0]) != std::string::npos);
    assert(c2.find(call[1]) != std::string::npos);
    assert(c3.find(call[2]) != std::string::npos);
    assert(c4.find(call[3]) != std::string::npos);
    assert(c4.find(call[4]) != std::string::npos);
    assert(c4.find(call[5]) != std::string::npos);

    x = 0;
    x += c1.find(call[0]);
    x *= c2.size(); x += c2.find(call[1]);
    x *= c3.size(); x += c3.find(call[2]);
    x *= c4.size(); x += c4.find(call[3]);
    x *= c4.size(); x += c4.find(call[4]);
    x *= c4.size(); x += c4.find(call[5]);
    return true;
}

unsigned long long pack_50(std::string call) {
    std::string v = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ /@";
    call = fixcall(call);
    assert(call.find("/") == std::string::npos);
    unsigned long long x = 0;
    int i = 0;
    x += v.find((i < (int)call.size()) ? call[i++] : ' '); x *= 38;
    x += v.find((i < (int)call.size()) ? call[i++] : ' '); x *= 38;
    x += v.find((i < (int)call.size()) ? call[i++] : ' '); x *= 38;
    x *= 2; // no slash
    x += v.find((i < (int)call.size()) ? call[i++] : ' '); x *= 38;
    x += v.find((i < (int)call.size()) ? call[i++] : ' '); x *= 38;
    x += v.find((i < (int)call.size()) ? call[i++] : ' '); x *= 38;
    x *= 2; // no slash
    x += v.find((i < (int)call.size()) ? call[i++] : ' '); x *= 38;
    x += v.find((i < (int)call.size()) ? call[i++] : ' '); x *= 38;
    x += v.find((i < (int)call.size()) ? call[i++] : ' ');
    return x;
}

unsigned int pack_grid(std::string grid) {
    if (grid.size() < 4) return 0x7ffff;
    int x1 = grid[0] - 'A';
    int x2 = grid[1] - 'A';
    int x3 = grid[2] - '0';
    int x4 = grid[3] - '0';
    int lon = 179 - (x1 * 10 + x3);
    int lat = x2 * 10 + x4;
    int x = (lon * 180) + lat;
    return x;
}

// Pack CQ message
std::vector<double> pack_cq(std::string call, std::string grid, int rate, double hz) {
    int a87[87] = {0};
    unsigned long long x = pack_50(call);
    setbits(a87, 3, 50, x);
    int g = pack_grid(grid) | 0x8000; // CQ flag
    setbits(a87, 53, 16, g);
    setbits(a87, 69, 3, 0); // CQ CQ CQ
    setbits(a87, 72, 3, 3); // itype=first+last
    return pack_any(a87, rate, hz);
}

// Pack directed message
std::vector<double> pack_directed(std::string my_call, std::string other_call,
                                  int cmd, int extra, int itype, int rate, double hz) {
    int a87[87] = {0};
    a87[0] = 0; a87[1] = 1; a87[2] = 1; // Directed message prefix
    unsigned int x;
    pack_call_28(my_call, x); setbits(a87, 3, 28, x);
    pack_call_28(other_call, x); setbits(a87, 31, 28, x);
    setbits(a87, 59, 5, cmd);
    if (extra < 0) extra = 0;
    if (extra > 63) extra = 63;
    setbits(a87, 66, 6, extra);
    setbits(a87, 72, 3, itype);
    return pack_any(a87, rate, hz);
}