# Bring-up log

Running notes on measurements, problems and causes. Newest at the bottom.

## 2026-10-28 — Step 2: register-level blink, flash, GDB

**Setup:** Nucleo-G474RE, OpenOCD 0.12.0 via on-board ST-LINK V3, arm-none-eabi-gdb.
System clock at reset = HSI16, 16 MHz.

**Flash:** `openocd -f board/st_nucleo_g4.cfg -c "program build/main.elf verify reset exit"`
→ Verified OK. OpenOCD erased the whole first 2 KB page before writing (flash erases by page).

**Reset state (`monitor reset halt`):** PC = 0x0800030C (Reset_Handler), MSP = 0x20020000
(top of RAM, loaded from word 0 of the vector table).

**Watching the toggle (breakpoint at main.c:27, `GPIOA->ODR ^= GPIO_ODR_OD5`):**

| Step | Command | GPIOA_ODR (0x48000014) | Meaning |
|---|---|---|---|
| at breakpoint | `x/1xw 0x48000014` | 0x00000000 | bit 5 = 0, PA5 low, LED off |
| after `next` | `x/1xw 0x48000014` | 0x00000020 | bit 5 = 1, PA5 high, LED on |
| `continue`, `next` | `x/1xw 0x48000014` | 0x00000000 | bit 5 back to 0, LED off |

0x20 = binary 0010 0000 = bit 5 = PA5. The XOR flips only that bit; the LED followed
each step.

**Problems and causes:**
q. `target extended-remote` timed out. Cause: the earlier OpenOCD had exited.
   Fix: run a plain `openocd -f board/st_nucleo_g4.cfg` and leave it running.
2. `break main` → "No symbol table is loaded". Cause: GDB started from the wrong folder.
   Fix: `file build/main.elf`.
3. A very short delay looked like "LED stuck on". Cause: toggling at kHz, which the eye
   averages. Lesson: check with the analyzer before assuming hardware is dead.

**To do:** measure the real blink period on the logic analyzer; read MODER and
RCC_AHB2ENR by address and check them against RM0440.


Step 3 entry

Configuration (the inputs):

Timer clock source and frequency: HSI, 16 MHz (reset default, SystemInit unchanged)
PSC = 15, ARR = 999, CCR1 = 250
Predicted: f = 16 MHz / 16 / 1000 = 1.000 kHz; duty = 250/1000 = 25.00 %; tick = 1 µs
Pin: PA5 = LD2, AF1 = TIM2_CH1 (DS12288 §4.11)

Measurement (the outputs):

Instrument: Lonely Binary 8-ch FX2 analyzer, Logic 2 / PulseView, sample rate used
Measured: 1.002 kHz, 25.05 % duty
Deviation and cause: +0.2 % → HSI ±1 % tolerance; +0.05 % → 1 µs sample quantization




