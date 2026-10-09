# Project: efficient on-device ML on STM32G474 (Nucleo-G474RE)

## Layout
- firmware/src/      my C and C++ sources
- firmware/inc/      my headers
- firmware/vendor/   ST and Arm CMSIS files: never edit
- firmware/STM32G474RETX_FLASH.ld, startup and system files: ST's
- DECISIONS.md       architecture decision records (mine)
- AI_LOG.md          log of AI-assisted changes (mine)

## Commands (run from firmware/)
- Build:   make
- Clean:   make clean
- Check:   arm-none-eabi-nm -C build/main.elf | grep -E "__cxa_throw|__cxa_demangle|malloc"   (must print nothing)
- Size:    arm-none-eabi-size build/main.elf
- Flash:   openocd -f board/st_nucleo_g4.cfg -c "program build/main.elf verify reset exit"

## Language rules
- C for startup code, register-level code and interrupt handlers.
- C++17 only in the restricted subset: no exceptions, no RTTI, no thread-safe
  statics, no new/malloc/heap after init, no std::vector/std::string.
- Anything C calls from C++ must be extern "C".

## Hardware rules
- No ST HAL or LL calls. Write registers directly through the CMSIS headers.
- Every register write gets a comment citing its RM0440 section,
  e.g. // RM0440 §7.4.17 RCC_AHB2ENR: enable GPIOA clock
- If you are not sure a register or bit exists on the STM32G474, say so; do not guess.

## Working rules
- Never edit or delete a test to make it pass. Explain the failure instead.
- Ask before: flashing the board, running hardware tests, editing the Makefile,
  the linker script, the startup file, or anything in vendor/.
- DECISIONS.md and AI_LOG.md are written by me. You may review them, not write them.
- Explain what you changed and why, so I can verify it.