#ifndef UNPACK_H
#define UNPACK_H

#include <string>

struct htht; // Forward declaration only
extern struct htht ht[]; // Declaration, defined in huffman_table.h

std::string unpack(const int a87[87], std::string& other_call);

#endif