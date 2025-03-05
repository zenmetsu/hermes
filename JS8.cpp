#include "JS8.h"
#include <cstring> // For strncpy and memset

JS8::JS8() : pendingMessage(false) {
    memset(js8OutputBuffer, 0, sizeof(js8OutputBuffer));
}

void JS8::setOutputBuffer(const char* message) {
    strncpy(js8OutputBuffer, message, sizeof(js8OutputBuffer) - 1);
    js8OutputBuffer[sizeof(js8OutputBuffer) - 1] = '\0'; // Ensure null-terminated
    pendingMessage = true;
}

bool JS8::hasPendingMessage() {
    return pendingMessage;
}

void JS8::clearOutputBuffer() {
    memset(js8OutputBuffer, 0, sizeof(js8OutputBuffer));
    pendingMessage = false;
}