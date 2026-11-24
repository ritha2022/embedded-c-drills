# 01. Decoding J1939 messages

## The problem

Heavy vehicles (trucks, buses, tractors) talk J1939 over CAN. A telematics unit or dashboard receives frames like `ID 0x0CF00400, data F0 7D 7D 40 1F 00 F0 7D` and must answer: **who sent this, what is it, and what is the engine speed?** It must also never show a wrong number when the engine ECU reports "error" or "not available."

## How a J1939 identifier is built

```
 bits:  28-26     25   24   23-16   15-8    7-0
       priority   EDP  DP    PF      PS      SA
```

- **SA (source address):** who sent it. `0x00` is usually the engine.
- **PGN (parameter group number):** what the message means. Built from EDP, DP, PF and sometimes PS.
- **The trap:** if **PF < 240** (PDU1), PS is a **destination address** and is *not* part of the PGN. If **PF ≥ 240** (PDU2), the message is a broadcast and PS *is* part of the PGN. Getting this wrong gives a wrong PGN for every point-to-point message.

Example: `0x0CF00400` → priority 3, PF `0xF0` (≥ 240, broadcast), PS `0x04` → PGN `0xF004` = **61444 (EEC1)**, from SA `0x00`.

## Engine speed (SPN 190)

- In EEC1 **bytes 4–5** (J1939 counts from 1, so `data[3]` and `data[4]`), **little-endian**
- **0.125 rpm per bit**, so rpm = raw ÷ 8
- Special values: `0xFE00–0xFEFF` = **error**, `0xFF00–0xFFFF` = **not available**, above `0xFAFF` = outside the valid range

Example: bytes 4–5 = `40 1F` → raw `0x1F40` = 8000 → **1000 rpm**.

## Design decisions

- **Status before value:** the function returns a status and only writes the rpm when it's valid, so the caller can't accidentally display garbage.
- **Integer maths only:** no floats, which keeps it fast and predictable on small MCUs.

## Tests prove

- Broadcast and point-to-point IDs decode to the right PGN, source and destination
- 1000 rpm and 3000 rpm decode correctly
- Error, not-available and out-of-range values are reported, never shown as speeds

## Interview questions this answers

- How is a PGN derived from a 29-bit identifier, and what changes between PDU1 and PDU2?
- How does J1939 signal "sensor failed" versus "not supported"?
