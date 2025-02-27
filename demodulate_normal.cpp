#include <Arduino.h>
#include "demodulate_normal.h"
#include "signal_proc.h"
#include "utils.h"
#include <arm_math.h>

static float fft_input[FFT_SIZE * 2];
static arm_cfft_radix2_instance_f32 fft_instance;

static std::vector<Conversation> conversations;
static uint32_t demod_start_time, demod_end_time;
static uint32_t resample_start_time, resample_end_time;

void init_fft() {
    arm_status status = arm_cfft_radix2_init_f32(&fft_instance, FFT_SIZE, 0, 1);
    char msg[128];
    print_timestamp(msg, sizeof(msg));
    if (status != ARM_MATH_SUCCESS) {
        sprintf(msg + strlen(msg), "FFT init failed - Status: %d", status);
        Serial.println(msg);
    } else {
        sprintf(msg + strlen(msg), "FFT init succeeded");
        Serial.println(msg);
    }
}

void demodulate_js8_normal() {
    demod_start_time = micros();
    char msg[128];
    print_timestamp(msg, sizeof(msg));
    sprintf(msg + strlen(msg), "Starting demodulation process for Normal mode");
    Serial.println(msg);

    // Resample 44.1 kHz to 12.8 kHz
    size_t input_size = (SLOT_DURATION_NORMAL + 2.36) * SAMPLE_RATE_NATIVE; // 17.36s = 765,126 samples
    size_t output_size = RESAMPLED_BUFFER_SIZE; // ~222,208 samples
    size_t current_write_idx = write_idx;
    size_t read_start_idx = (current_write_idx - input_size + SLOT_DURATION_MAX * SAMPLE_RATE_NATIVE) % (SLOT_DURATION_MAX * SAMPLE_RATE_NATIVE);

    resample_start_time = micros();
    float resample_ratio = static_cast<float>(SAMPLE_RATE_NATIVE) / SAMPLE_RATE_RESAMPLED; // ~3.4453
    float resample_acc = 0.0f;

    for (size_t i = 0, j = 0; i < input_size && j < output_size; i++) {
        resample_acc += 1.0f / resample_ratio;
        if (resample_acc >= 1.0f) {
            size_t buffer_idx = (read_start_idx + i) % (SLOT_DURATION_MAX * SAMPLE_RATE_NATIVE);
            resampled_buffer[j++] = audio_buffer[buffer_idx];
            resample_acc -= 1.0f;
        }
    }
    resample_end_time = micros();
    print_timestamp(msg, sizeof(msg));
    sprintf(msg + strlen(msg), "Resampling to 12.8 kHz completed in %lu us", resample_end_time - resample_start_time);
    Serial.println(msg);

    // Power integration over 17.36s at 12.8 kHz
    std::vector<float> power_spectrum(FFT_SIZE / 2, 0.0f);
    std::vector<float> process_buffer(FFT_SIZE);
    uint32_t window_count = 0;

    for (size_t offset = 0; offset + FFT_SIZE <= output_size; offset += FFT_SIZE) {
        for (int i = 0; i < FFT_SIZE; i++) {
            process_buffer[i] = resampled_buffer[offset + i] / 32768.0f * (0.54f - 0.46f * cosf(2.0f * PI * i / (FFT_SIZE - 1)));
            fft_input[2 * i] = process_buffer[i];
            fft_input[2 * i + 1] = 0.0f;
        }
        arm_cfft_radix2_f32(&fft_instance, fft_input);
        arm_cmplx_mag_f32(fft_input, std::vector<float>(FFT_SIZE / 2).data(), FFT_SIZE / 2);

        // Accumulate power
        for (size_t bin = 0; bin < FFT_SIZE / 2; bin++) {
            power_spectrum[bin] += fft_input[bin];
        }
        window_count++;
    }

    // Average power per bin and debug
    float max_power = 0.0f;
    int max_bin = 0;
    for (size_t bin = 0; bin < FFT_SIZE / 2; bin++) {
        power_spectrum[bin] /= window_count;
        if (power_spectrum[bin] > max_power) {
            max_power = power_spectrum[bin];
            max_bin = bin;
        }
    }
    print_timestamp(msg, sizeof(msg));
    sprintf(msg + strlen(msg), "Max FFT power after averaging: %.4f at bin %d (%.2f Hz)", 
            max_power, max_bin, max_bin * (SAMPLE_RATE_RESAMPLED / (float)FFT_SIZE));
    Serial.println(msg);

    // Find top 10 tone sets by power
    struct ToneSet {
        float center_freq; // Frequency of tone 4
        float total_power;
    };
    std::vector<ToneSet> tone_sets;
    for (int bin = 48; bin <= 568 - NUM_TONES; bin++) {
        float total_power = 0.0f;
        bool is_tone_set = true;
        for (int k = 0; k < NUM_TONES; k++) {
            int tone_bin = bin + k;
            if (tone_bin >= FFT_SIZE / 2 || power_spectrum[tone_bin] < 0.5f) {
                is_tone_set = false;
                break;
            }
            total_power += power_spectrum[tone_bin];
        }
        if (is_tone_set) {
            float freq = (bin + 4) * (static_cast<float>(SAMPLE_RATE_RESAMPLED) / FFT_SIZE); // Tone 4’s frequency
            tone_sets.push_back({freq, total_power});
        }
    }

    // Debug tone set count
    print_timestamp(msg, sizeof(msg));
    sprintf(msg + strlen(msg), "Number of tone sets detected: %u", (unsigned)tone_sets.size());
    Serial.println(msg);

    // Sort and limit to top 10
    std::sort(tone_sets.begin(), tone_sets.end(), 
              [](const ToneSet& a, const ToneSet& b) { return a.total_power > b.total_power; });
    if (tone_sets.size() > 10) tone_sets.resize(10);

    // Log top 10 tone sets
    for (size_t i = 0; i < tone_sets.size(); i++) {
        print_timestamp(msg, sizeof(msg));
        sprintf(msg + strlen(msg), "Top tone set %u at %.2f Hz, power: %.4f", (unsigned)i + 1, tone_sets[i].center_freq, tone_sets[i].total_power);
        Serial.println(msg);
    }

    std::vector<std::vector<int>> symbol_streams(tone_sets.size(), std::vector<int>(SYMBOLS_PER_SLOT, -1));
    std::vector<float> power_spectrum_temp(FFT_SIZE / 2);

    if (!tone_sets.empty()) {
        conversations.resize(tone_sets.size());
        for (size_t c = 0; c < tone_sets.size(); c++) {
            conversations[c].center_freq = tone_sets[c].center_freq;
            conversations[c].sym_count = 0;
            conversations[c].active = true;

            // Extract 108 symbols per tone set
            for (size_t sym_idx = 0; sym_idx < SYMBOLS_PER_SLOT; sym_idx++) {
                size_t offset = sym_idx * SYMBOL_SAMPLES_NORMAL; // 2048 samples (~0.16s at 12.8 kHz)
                if (offset + FFT_SIZE > output_size) break;

                for (int i = 0; i < FFT_SIZE; i++) {
                    process_buffer[i] = resampled_buffer[offset + i] / 32768.0f * (0.54f - 0.46f * cosf(2.0f * PI * i / (FFT_SIZE - 1)));
                    fft_input[2 * i] = process_buffer[i];
                    fft_input[2 * i + 1] = 0.0f;
                }
                arm_cfft_radix2_f32(&fft_instance, fft_input);
                arm_cmplx_mag_f32(fft_input, power_spectrum_temp.data(), FFT_SIZE / 2);

                int base_bin = static_cast<int>(tone_sets[c].center_freq / TONE_SPACING) - 4; // Tone 0’s bin
                int max_tone = 0;
                float max_power = 0.0f;
                for (int k = 0; k < NUM_TONES; k++) {
                    int tone_bin = base_bin + k;
                    if (tone_bin >= 0 && tone_bin < FFT_SIZE / 2) {
                        if (power_spectrum_temp[tone_bin] > max_power) {
                            max_power = power_spectrum_temp[tone_bin];
                            max_tone = k;
                        }
                    }
                }
                symbol_streams[c][sym_idx] = max_tone;
            }
        }

        // Costas synchronization
        print_timestamp(msg, sizeof(msg));
        sprintf(msg + strlen(msg), "Starting Costas synchronization");
        Serial.println(msg);
        for (size_t c = 0; c < tone_sets.size(); c++) {
            float max_corr = 0.0f;
            size_t best_offset = 0;
            for (size_t offset = 0; offset + SYMBOLS_PER_MESSAGE <= SYMBOLS_PER_SLOT; offset++) {
                float corr_start = 0.0f, corr_mid = 0.0f, corr_end = 0.0f;
                for (int k = 0; k < 7; k++) {
                    if (symbol_streams[c][offset + k] == COSTAS_PATTERN[k]) corr_start += 1.0f;
                    if (offset + 36 + k < SYMBOLS_PER_SLOT && symbol_streams[c][offset + 36 + k] == COSTAS_PATTERN[k]) corr_mid += 1.0f;
                    if (offset + 72 + k < SYMBOLS_PER_SLOT && symbol_streams[c][offset + 72 + k] == COSTAS_PATTERN[k]) corr_end += 1.0f;
                }
                float total_corr = (corr_start + corr_mid + corr_end) / 3.0f;
                if (total_corr > max_corr) {
                    max_corr = total_corr;
                    best_offset = offset;
                }
            }

            if (max_corr >= 4.0f) {
                print_timestamp(msg, sizeof(msg));
                sprintf(msg + strlen(msg), "Tone set at %.2f Hz, Costas corr: %.1f", tone_sets[c].center_freq, max_corr);
                Serial.println(msg);

                if (max_corr >= 5.0f) {
                    for (int i = 0; i < SYMBOLS_PER_MESSAGE; i++) {
                        conversations[c].symbols[i] = symbol_streams[c][best_offset + i];
                    }
                    conversations[c].sym_count = SYMBOLS_PER_MESSAGE;

                    print_timestamp(msg, sizeof(msg));
                    sprintf(msg + strlen(msg), "Message at %.2f Hz: ", tone_sets[c].center_freq);
                    Serial.print(msg);
                    for (int i = 0; i < SYMBOLS_PER_MESSAGE; i++) {
                        Serial.print(conversations[c].symbols[i]);
                        Serial.print(" ");
                    }
                    Serial.println();
                    Serial.flush();
                    conversations[c].active = false;
                }
            }
        }
    }

    demod_end_time = micros();
    print_timestamp(msg, sizeof(msg));
    sprintf(msg + strlen(msg), "Demodulation completed in %lu us", demod_end_time - demod_start_time);
    Serial.println(msg);
    print_timestamp(msg, sizeof(msg));
    sprintf(msg + strlen(msg), "End of demodulation process");
    Serial.println(msg);
}