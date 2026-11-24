# ADR: architecture decision record

#ADR-001: Which languages, where

**Context:** Small chip (512 KB flash, 128 KB RAM); interrupts must respond on time.

**Options:** C only or full C++ or C for startup and registers + restricted C++ elsewhere.

**Decision:** Restricted C++17: no exceptions, no RTTI, no thread-safe statics, no heap after startup.

**Measurement:** nm found no exception code, no error-message printer and no heap.
Clean build: 1104 bytes. With one new: 1800 bytes. 

**Consequences:** Functions return error codes instead of throwing; no std::vector or std::string.

**Revisit when:** Moving to a much bigger chip.


## ADR-002 — PWM frequency and brightness resolution (Step 3.2)

### The problem
A pin can only be fully on (3.3 V) or fully off (0 V). To get an LED to look
25 % bright, we switch it on and off very fast and control what fraction of
the time it is on. Two numbers describe that switching:

- **Frequency** — how many on/off cycles per second. Set by the timer tick
  rate and ARR: f = 1 MHz / (ARR + 1).     (TIM2 ticks per s / ARR + 1) 
- **Resolution** — how many different brightness levels we can choose. Set
  by ARR: levels = ARR + 1.

They pull against each other. The timer ticks 1,000,000 times a second
(16 MHz prescaled by 16). Those ticks get divided between "how many cycles
per second" and "how many ticks inside each cycle". More cycles per second
means fewer ticks per cycle, so fewer brightness levels.

### What the frequency has to satisfy
- **Low enough to leave room for resolution.** At 100 kHz there would be
  only 10 ticks per cycle: 10 brightness levels, visibly coarse.
- **High enough that the eye cannot see the switching.** The eye blends
  anything above roughly 100 Hz into a steady light. A phone camera, which
  exposes each frame for about a millisecond, can still see banding at a
  few hundred Hz.
- **Not higher than needed.** On a real power transistor every on/off edge
  wastes a little energy, so switching faster than necessary costs power
  for nothing. Irrelevant for an LED through a resistor, relevant later.

### Options
| Frequency | Levels at 1 MHz ticks | Verdict |
|---|---|---|
| 100 Hz | 10,000 | at the edge of visible flicker; too many levels to use |
| 1 kHz | 1,000 | 10× above flicker; 1,000 levels is finer than the eye can tell |
| 100 kHz | 10 | no flicker risk, but only 10 brightness levels |

### Decision
**1 kHz, 1,000 levels** (PSC = 15, ARR = 999). The switching is invisible
with margin, each level is one tick = 1 µs, and CCR1 reads directly as
microseconds, which makes debugging simple.

### Measurement that confirmed it
Logic analyzer on PA5: 1.002 kHz (+0.2 %, the HSI oscillator's tolerance)
and 25.05 % duty at CCR1 = 250 (+0.05 %, the analyzer's 1 µs sampling).
Changing CCR1 one count at a time moved the pulse by 1 µs.
Captures: `docs/measurements/03-pwm-fixed.png`, `03-pwm-sweep.png`.

### Consequences and when to revisit
Fine for LEDs. A motor or switching converter needs 20 kHz or more to stay
above hearing range, which at 1 MHz ticks leaves only 50 levels; Step 14's
170 MHz clock would recover that. Revisit if the load changes or the timer
clock changes.

---

## Note — Breathing rate of the demo loop (Step 3.4)

This is a separate choice from the PWM frequency and is not an ADR; it is
how fast the *demo* changes brightness, recorded here so the two are not
confused.

- The PWM frequency (1 kHz) is how fast the pin switches. It is meant to be
  invisible.
- The breathing rate is how fast the brightness setting changes. It is
  meant to be visible.

The loop raises the duty by 1 % (10 counts of CCR1) per iteration and then
waits. For the eye to see a smooth fade rather than flicker, one full fade
needs to take a few hundred milliseconds at least. With 100 steps per fade:

- 1 ms per step → 100 ms per fade → 5 breaths/s → reads as flicker.
- 5 ms per step → 500 ms per fade → 1 breath/s → reads as a smooth fade.

Chosen: ~5 ms per step (`delay_rough(8000)`, a busy-wait of about 80,000
CPU cycles at 16 MHz). On the analyzer each duty value is held for about
5 consecutive 1 ms periods, which confirms the delay. The busy-wait is a
placeholder; Step 4 replaces it with a timer interrupt.

The PWM frequency could be 400 Hz or 4 kHz and the breath would still be
one per second, because the breath is set by the loop, not the timer.
