# Efficient on-device ML on a low-power microcontroller

Running a neural network efficiently on a Cortex-M4 (STM32G474, 128 KB RAM):
always-on keyword spotting from an I²S microphone, a two-stage wake-up so the
network runs only when there is sound, power management that picks the
lowest-energy clock speed, and thermal and power limits. Every claim is backed
by a measurement (energy per inference, latency, memory, accuracy).

Bare-metal C and restricted C++17 firmware; Python for training and test tooling.

**Status:** in progress (toolchain, build and C++ setup done).
Design decisions and measurements: [DECISIONS.md](DECISIONS.md).