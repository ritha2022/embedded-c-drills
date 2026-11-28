# Embedded C drills and automotive case studies

![tests](https://github.com/ritha2022/embedded-c-drills/actions/workflows/ci.yml/badge.svg)

Ten small embedded C problems that come up in real automotive firmware and in interviews, each with automatic tests, plus three case studies of real automotive problems. Every solution is explained below.

| # | Drill | Where it's used | Status |
|---|---|---|---|
| 01 | Set, clear, toggle and check a bit | Register access | ✅ |
| 02 | Power of two without loops | Buffer sizes, alignment | ✅ |
| 03 | Count set bits | Error masks, flags | ✅ |
| 04 | Big-endian and little-endian reads | Motorola and Intel CAN signals | ✅ |
| 05 | Decode a CAN signal with scale and offset | DBC signal decoding | ✅ |
| 06 | Alive (rolling) counter check | End-to-end protection | ✅ |
| 07 | Timeout that survives timer wraparound | Message timeout monitoring | ✅ |
| 08 | Switch debounce | Digital inputs | ✅ |
| 09 | Ring buffer (FIFO) | ISR-to-task data passing | ✅ |
| 10 | CRC-8 SAE J1850 | AUTOSAR E2E, CAN message checks | ✅ |

## Case studies: real automotive problems

| # | Problem | Key ideas |
|---|---|---|
| [01](case-studies/01-j1939-decoder/) | Decode J1939 IDs and engine speed | PDU1 vs PDU2, scaling, error and not-available values |
| [02](case-studies/02-e2e-protection/) | Protect a safety CAN message end to end | CRC-8 J1850, alive counter, Data ID, replay detection |
| [03](case-studies/03-bms-overvoltage/) | BMS cell over-voltage protection | Debounce, hysteresis, plausibility, fail-safe |

## How to run

```bash
make test
```

On Windows, use WSL or MSYS2 (both provide `gcc` and `make`). Every push also runs the tests automatically on GitHub.

## Rules I follow

- Fixed-width types (`uint8_t`, `uint32_t`) and no undefined behaviour
- Builds with `-Wall -Wextra -Wconversion -Werror`
- Each solution has a short note below on why it works

## Notes

### 01. Bits

Each function builds a mask with only bit *n* set, `1u << n`, then combines it with the register: OR to set, AND with the inverted mask to clear, XOR to toggle. To check a bit, shift it down to position 0 and keep only that bit with `& 1u`. I use `1u` (unsigned) because shifting a signed `1` into bit 31 is undefined behaviour in C.

### 02. Power of two

A power of two has exactly one bit set. Subtracting 1 turns that bit off and turns every lower bit on, so `v & (v - 1)` is 0 only when `v` had a single bit set. Zero needs its own check, because `0 & (0 - 1)` is also 0, yet 0 is not a power of two. Firmware uses this to check that a ring buffer's size allows `index & (size - 1)` instead of a slower `%`.

### 03. Count set bits

`v &= (v - 1)` clears the lowest 1 bit each time (Brian Kernighan's trick), so the loop runs once per set bit instead of 32 times. Used to count active fault flags or error bits.

### 04. Endianness

CAN signals come in two byte orders. **Big-endian (Motorola):** the first byte is the most significant, so `(p[0] << 8) | p[1]`. **Little-endian (Intel):** the first byte is the least significant, so `(p[1] << 8) | p[0]`. Building the value with shifts gives the same result on any CPU, whatever its own byte order. Mixing these up is the most common signal-decoding bug.

### 05. CAN signal decoding

Battery temperature sits in bytes 2–3, big-endian, at 0.1 °C per bit with a −40 °C offset. Returning tenths of a degree avoids floating point: raw × 0.1 °C is simply *raw* tenths, and −40 °C is −400 tenths, so the result is `raw − 400`. The subtraction is done in `int32_t` so it can't overflow before the cast.

### 06. Alive counter

A 4-bit counter goes 0–15 and wraps to 0. `(current − previous) & 0x0F` gives the step even across the wrap: 15 → 0 is +1. A step of 0 means the sender is frozen or the frame was repeated; more than 1 means frames were lost.

### 07. Timeout with wraparound

A 32-bit millisecond timer wraps to 0 after about 49.7 days. Unsigned subtraction wraps the same way, so `now − last` still gives the true elapsed time across the wrap. Comparing `now > last + limit` instead would fail right at the wrap.

### 08. Debounce

A mechanical switch bounces for a few milliseconds when pressed. The output changes only after 4 equal readings in a row that differ from the current stable level; any bounce restarts the count. Calling it from a 10 ms periodic task gives a 40 ms debounce without blocking the CPU.

### 09. Ring buffer

A fixed array used as a queue between an interrupt (writer) and a task (reader): `head` is where the next byte goes, `tail` where the next byte comes from, and both wrap around the end. A separate `count` tells full from empty, so all slots are usable, and a full buffer refuses new data rather than overwriting unread bytes.

### 10. CRC-8 SAE J1850

Start at 0xFF. For each byte, XOR it into the CRC; then for each of its 8 bits, shift left, and if a 1 fell off the top, XOR in the polynomial 0x1D. Finish with XOR 0xFF. The standard check value for the text "123456789" is 0x4B, which the tests confirm. This CRC protects safety-related CAN messages in AUTOSAR E2E.
