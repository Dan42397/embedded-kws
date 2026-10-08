# ADR: architecture decision record

#ADR-001: Which languages, where

**Context:** Small chip (512 KB flash, 128 KB RAM); interrupts must respond on time.

**Options:** C only or full C++ or C for startup and registers + restricted C++ elsewhere.

**Decision:** Restricted C++17: no exceptions, no RTTI, no thread-safe statics, no heap after startup.

**Measurement:** nm found no exception code, no error-message printer and no heap.
Clean build: 1104 bytes. With one new: 1800 bytes. 

**Consequences:** Functions return error codes instead of throwing; no std::vector or std::string.

**Revisit when:** Moving to a much bigger chip.

