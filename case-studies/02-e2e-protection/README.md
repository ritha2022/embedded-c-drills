# 02. End-to-end protection of a safety message

## The problem

A brake or steering ECU receives "vehicle speed" over CAN. CAN's own CRC catches most bit errors, but it can't detect:

- a **frozen sender** that keeps repeating an old value
- a **replayed** frame recorded earlier
- **lost** frames
- a frame from a **different message** that accidentally uses the same CAN ID
- corruption **inside an ECU or gateway** (memory, software), where the CAN CRC is recalculated anyway

Safety-related signals therefore carry **end-to-end (E2E) protection**, as in AUTOSAR's E2E library: the sender adds a CRC and an alive counter, and the receiver checks both.

## Frame layout

| Byte | Content |
|---|---|
| 0 | CRC-8 SAE J1850 |
| 1 (low 4 bits) | Alive counter 0–15, then wraps to 0 |
| 2–7 | Application data |

The CRC is calculated over a **16-bit Data ID plus bytes 1–7**. The Data ID is **never sent**: both ECUs know it. If a different message lands on this CAN ID, its CRC won't match, because it was made with a different Data ID.

## Receiver decisions

| Counter change | Result | Use the data? |
|---|---|---|
| CRC wrong | `E2E_ERROR` | No |
| First frame after start-up | `E2E_INITIAL` | Yes |
| +1 | `E2E_OK` | Yes |
| +2 up to `max_delta` | `E2E_OK_SOME_LOST` | Yes |
| 0 (same counter) | `E2E_REPEATED` | No: stale or replayed |
| more than `max_delta` | `E2E_WRONG_SEQUENCE` | No |

The counter difference is calculated as `(new − last) & 0x0F`, so 15 → 0 counts as +1, not as an error.

## Design decisions

- **Never learn from a bad frame:** a CRC error doesn't update the stored counter.
- **Repeated frames don't move the counter,** so a replay can't push the receiver out of sync.
- **The application decides the reaction** (substitute value, warning, safe state). E2E only reports what happened.

This is modelled on AUTOSAR E2E concepts for learning. Production systems use the exact AUTOSAR profile their project specifies.

## Tests prove

Normal frames, a replay, a flipped data bit, one lost frame, five lost frames, counter wraparound over 20 frames, and a message with a different Data ID.

## Interview questions this answers

- Why isn't the CAN CRC enough for safety-related data?
- What does the Data ID protect against, and why isn't it transmitted?
- How do you compare a 4-bit rolling counter correctly across the wrap?
