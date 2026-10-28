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
