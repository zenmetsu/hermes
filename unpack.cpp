#include <string>
#include <vector>
#include <Arduino.h>

struct htht {
    const char *a;
    const char *b;
};
static struct htht ht[] = {
    {" ", "01"}, {"E", "100"}, {"T", "1101"}, {"A", "0011"}, {"O", "11111"},
    {"I", "11100"}, {"N", "10111"}, {"S", "10100"}, {"H", "00011"}, {"R", "00000"},
    {"D", "111011"}, {"L", "110011"}, {"C", "110001"}, {"U", "101101"}, {"M", "101011"},
    {"W", "001011"}, {"F", "001001"}, {"G", "000101"}, {"Y", "000011"}, {"P", "1111011"},
    {"B", "1111001"}, {".", "1110100"}, {"V", "1100101"}, {"K", "1100100"}, {"-", "1100001"},
    {"+", "1100000"}, {"?", "1011001"}, {"!", "1011000"}, {"\"", "1010101"}, {"X", "1010100"},
    {"0", "0010101"}, {"J", "0010100"}, {"1", "0010001"}, {"Q", "0010000"}, {"2", "0001001"},
    {"Z", "0001000"}, {"3", "0000101"}, {"5", "0000100"}, {"4", "11110101"}, {"9", "11110100"},
    {"8", "11110001"}, {"6", "11110000"}, {"7", "11101011"}, {"/", "11101010"}, {0, 0}
};

static std::vector<std::string> words; // Empty for now; embed later if needed

void load_words() {
    // Stub: No file I/O on Teensy; words remain empty
}

unsigned long long un(const int a87[87], int start, int n) {
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