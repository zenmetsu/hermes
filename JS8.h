#ifndef JS8_H
#define JS8_H

class JS8 {
public:
    JS8();
    void setOutputBuffer(const char* message);
    bool hasPendingMessage();
    void clearOutputBuffer();
    const char* getOutputBuffer() const; // Added getter
private:
    char js8OutputBuffer[53]; // Text buffer, matches inputBuffer size
    bool pendingMessage;      // Tracks if a message is queued
};

#endif