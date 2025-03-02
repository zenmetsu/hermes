#include <Audio.h>
#include <TimeLib.h>
#include "js8.h" // Simplified, 12800 Hz
#include "pack.h" // From 1896012795776549375
#include "resample.h" // From 1895998174680256538

AudioInputI2S audioInput;
AudioRecordQueue queue;
AudioOutputI2S audioOutput;
AudioPlayQueue playQueue;
AudioConnection patchCord1(audioInput, 0, queue, 0);
AudioConnection patchCord2(playQueue, 0, audioOutput, 0);

EXTMEM int16_t inputBuffer[44100 * 15];
EXTMEM int16_t resampledBuffer[12800 * 15];
volatile uint32_t writeIdx = 0, resampleIdx = 0;
const uint32_t bufferSize = 44100 * 15;
const uint32_t resampledSize = 12800 * 15;
std::vector<double> rxSamples(resampledSize);
PolyphaseResampler resampler;

std::string mycall = "YOURCALL";
std::string mygrid = "YOURGRID";
std::string rx_call, rx_buf, tx_buf;
double rx_hz = 0, rx_snr = 0, tx_hz = -1;
bool transmitting = false;

void setup() {
    Serial.begin(115200);
    AudioMemory(10);
    queue.begin();
    setSyncProvider(getTeensyRTCTime);
    memset(inputBuffer, 0, sizeof(inputBuffer));
    memset(resampledBuffer, 0, sizeof(resampledBuffer));
}

void loop() {
    if (queue.available() >= 1) {
        int16_t *block = queue.readBuffer();
        for (int i = 0; i < 128; i++) {
            inputBuffer[writeIdx] = block[i];
            writeIdx = (writeIdx + 1) % bufferSize;
        }
        uint32_t out_samples;
        int16_t temp_output[INPUT_BLOCK_SIZE];
        resampler.resample(block, temp_output, &out_samples);
        for (uint32_t i = 0; i < out_samples; i++) {
            resampledBuffer[resampleIdx] = temp_output[i];
            resampleIdx = (resampleIdx + 1) % resampledSize;
        }
        queue.freeBuffer();
    }

    if (second(now()) % 15 == 0 && !transmitting) {
        for (int i = 0; i < resampledSize; i++) {
            rxSamples[i] = resampledBuffer[i] / 32768.0;
        }
        entry(rxSamples.data(), rxSamples.size(), 12800 * 0.5, 12800, 100, 2900, nullptr, nullptr, hermes_cb);
    }

    if (Serial.available()) {
        char c = Serial.read();
        if (c == '\n') tx_buf += " ";
        else if (c == 129) tx_buf += (char)129;
        else if (c == 130) tx_buf += (char)130;
        else if (c == 131) tx_buf += (char)131;
        else tx_buf += c;
    }

    if (second(now()) % 15 == 0 && tx_buf.length() > 0 && !transmitting) {
        if (tx_hz < 0) tx_hz = 1500;
        std::vector<double> samples;
        if (tx_buf[0] == (char)129) {
            samples = pack_cq(mycall, mygrid, 12800, tx_hz);
            tx_buf.erase(0, 1);
        } else if (tx_buf[0] == (char)130 && rx_call.length() > 0) {
            samples = pack_directed(mycall, rx_call, 19, 0, 3, 12800, tx_hz);
            tx_buf.erase(0, 1);
        } else if (tx_buf[0] == (char)131 && rx_call.length() > 0) {
            samples = pack_directed(mycall, rx_call, 25, rx_snr + 31, 3, 12800, tx_hz);
            tx_buf.erase(0, 1);
        } else {
            int consumed;
            samples = pack_text(tx_buf, consumed, 2, 12800, tx_hz);
            tx_buf.erase(0, consumed);
        }
        transmit(samples);
    }
}

int hermes_cb(int *a87, double hz0, double hz1, double off, const char *comment, double snr, int pass, int correct_bits) {
    std::string other_call, txt = unpack(a87, other_call);
    Serial.printf("Decode: %.0f Hz, SNR=%.1f, %s\n", hz0, snr, txt.c_str());
    if (other_call == rx_call) {
        rx_buf += txt;
        rx_snr = snr;
    }
    return 2;
}

void transmit(std::vector<double> samples) {
    transmitting = true;
    for (size_t i = 0; i < samples.size(); i += 128) {
        int16_t *buffer = playQueue.getBuffer();
        for (int j = 0; j < 128 && (i + j) < samples.size(); j++) {
            buffer[j] = static_cast<int16_t>(samples[i + j] * 32767);
        }
        playQueue.playBuffer();
        while (playQueue.available() > 0) delay(1);
    }
    transmitting = false;
}

time_t getTeensyRTCTime() {
    return Teensy3Clock.get();
}