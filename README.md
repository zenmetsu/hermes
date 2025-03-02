# JS8 Teensy Transceiver

## IMPORTANT NOTE
This repository is under heavy development.  Please don't buy hardware with the expectation that you will be able to cobble together a functional device as it may be some time before the code within this project matures to that point.

## Overview
The JS8 Teensy Transceiver is an experimental software-defined radio (SDR) application designed to transmit and receive JS8 digital mode signals using a Teensy 4.1 microcontroller and a Revision D Teensy Audio Adaptor. JS8 is a weak-signal digital mode derived from FT8, offering various speed modes (Turbo: 6s, Fast: 10s, Normal: 15s, Slow: 30s) for amateur radio communication. This project currently focuses on the Normal mode (15s slot, 6.25 baud), with plans to expand to other modes.

The program leverages the Teensy 4.1’s processing power and PSRAM to handle real-time audio processing, resampling, and signal demodulation, making it a compact, standalone solution for JS8 communication when paired with a compatible radio transceiver.

This project is a learning experiment in C++ and digital signal processing (DSP), iteratively developed to improve code organization and functionality.

## Features
- **Receive**: Demodulates JS8 Normal mode signals from a 44.1 kHz audio input, resampling to 12.8 kHz for precise 6.25 Hz/bin FFT analysis.
- **Transmit**: Generates JS8 Normal mode signals for transmission (currently unimplemented in hardware).
- **Real-Time**: Uses interrupt-driven audio buffering and timer-based demodulation.
- **Diagnostics**: Includes profiling (e.g., initialization, resampling times) and timestamped debug messages.
- **Modular Design**: Structured into separate files (`signal_proc`, `transmit`, `demodulate_normal`, `utils`) for maintainability.

## Hardware Prerequisites
- **Teensy 4.1**: 
  - Microcontroller with 600 MHz Cortex-M7, 1 MB RAM (512 KB DTCM, 512 KB OCRAM), and optional 8 MB PSRAM (minimum required).
  - PSRAM is essential for the 5.17 MB `audio_buffer` (60s at 44.1 kHz) and ~444 KB `resampled_buffer` (17.36s at 12.8 kHz).
- **Revision D Teensy Audio Adaptor**:
  - Provides stereo audio I/O via I2S, connected to the Teensy 4.1.
  - Input: Line In (left channel) from a radio’s audio output (e.g., 3.5mm jack).
  - Output: Line Out (left channel) for transmission (not yet implemented).
- **Radio Transceiver**:
  - Compatible with JS8 frequencies (e.g., HF bands like 40m, 20m).
  - Audio output connected to Teensy Audio Adaptor’s Line In.
  - PTT (Push-to-Talk) control via Teensy pin 2 (not yet implemented).
- **USB Connection**:
  - For power, programming, and serial debugging via a host computer.

## Software Requirements
- **Arduino IDE**: With Teensyduino addon for compiling and uploading to Teensy 4.1.
- **Libraries**:
  - `Audio.h` (Teensy Audio Library): For I2S audio processing.
  - `TimeLib.h`: For RTC-based timestamps.
  - `arm_math.h` (CMSIS-DSP): For FFT and signal processing.
- **Dependencies**: Standard C++ libraries (`vector`, `algorithm`).

## Installation
1. **Hardware Setup**:
   - Solder the Teensy Audio Adaptor to the Teensy 4.1 (pins aligned for I2S).
   - Connect Line In from the radio to the Audio Adaptor’s left channel input.
   - Connect Teensy pin 2 to the radio’s PTT (optional, not yet functional).
   - Add 8 MB PSRAM chip to Teensy 4.1 if not pre-installed.
2. **Software Setup**:
   - Install Arduino IDE and Teensyduino.
   - Clone this repository: `git clone [repo-url]`.
   - Open `hermes.ino` in Arduino IDE.
   - Select Teensy 4.1 board and USB Serial port.
   - Compile and upload.

## Usage
- **Running**: Upload the sketch to the Teensy 4.1. Open the Serial Monitor (115200 baud) to view debug output.
- **Debug Output**: Timestamped messages show initialization, FFT status, resampling timing, and demodulation progress.
- **Current State**: Receives JS8 Normal mode signals every 15s (±2.36s skew), but tone detection needs debugging (no signals detected yet).

## Project Structure
- `hermes.ino`: Main sketch, setup, and loop.
- `signal_proc.h/cpp`: Core initialization and buffer management.
- `transmit.h/cpp`: Transmission logic (not yet hardware-integrated).
- `demodulate_normal.h/cpp`: JS8 Normal mode demodulation.
- `utils.h/cpp`: Utility functions (e.g., timestamps).
- `config.h`: Constants (e.g., sample rates, FFT size).
- `audio_io.h/cpp`: Audio I/O handling via Teensy Audio Library.

## Status
- **Stable**: No crashes with FFT_SIZE = 2048.
- **To-Do**:
  - Debug demodulation (no tone sets detected—threshold or bin issue).
  - Implement TX hardware integration.
  - Add Turbo (6s) and Fast (10s) modes.
  - Refactor to class-based design.

## Contributing
This is a learning project—contributions welcome! Fork, modify, and submit pull requests via GitHub.

## License
MIT License—see `LICENSE` file (to be added).
