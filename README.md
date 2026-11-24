**# Embedded C drills and automotive case studies

![tests](https://github.com/ritha2022/embedded-c-drills/actions/workflows/ci.yml/badge.svg)

Ten small embedded C problems that come up in real automotive firmware and in interviews, each with automatic tests. I solve one or two a week and explain the reasoning below.

| # | Drill | Where it's used | Status |
|---|---|---|---|
| 01 | Set, clear, toggle and check a bit | Register access | ✅ |
| 02 | Power of two without loops | Buffer sizes, alignment | ⬜ |
| 03 | Count set bits | Error masks, flags | ⬜ |
| 04 | Big-endian and little-endian reads | Motorola and Intel CAN signals | ⬜ |
| 05 | Decode a CAN signal with scale and offset | DBC signal decoding | ⬜ |
| 06 | Alive (rolling) counter check | End-to-end protection | ⬜ |
| 07 | Timeout that survives timer wraparound | Message timeout monitoring | ⬜ |
| 08 | Switch debounce | Digital inputs | ⬜ |
| 09 | Ring buffer (FIFO) | ISR-to-task data passing | ⬜ |
| 10 | CRC-8 SAE J1850 | AUTOSAR E2E, CAN message checks | ⬜ |

Change ⬜ to ✅ as you finish each one.

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

Only drills marked `1` in `include/drills.h` are tested, so the repository stays green while work is in progress.

## Rules I follow

- Fixed-width types (`uint8_t`, `uint32_t`) and no undefined behaviour
- Builds with `-Wall -Wextra -Wconversion -Werror`
- Each solution has a short note below on why it works

## Notes

### 01. Bits

Each function builds a mask with only bit *n* set, `1u << n`, then combines it with the register: OR to set, AND with the inverted mask to clear, XOR to toggle. To check a bit, shift it down to position 0 and keep only that bit with `& 1u`. I use `1u` (unsigned) because shifting a signed `1` into bit 31 is undefined behaviour in C.**
