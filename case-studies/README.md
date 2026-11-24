# Case studies: real automotive problems

Each case study is a problem that production ECUs solve every day, written as small, testable C modules: problem, design decisions, code and tests that prove it works.

| # | Problem | Where it's used | Key ideas |
|---|---|---|---|
| [01](01-j1939-decoder/) | Decode J1939 messages: who sent it, what it is, the engine speed | Trucks, buses, tractors, generators | 29-bit IDs, PDU1 vs PDU2, scaling, "not available" values |
| [02](02-e2e-protection/) | Protect a safety message against corruption, replay and loss | Brakes, steering, BMS, any safety signal on CAN | CRC-8 SAE J1850, alive counter, Data ID, wraparound |
| [03](03-bms-overvoltage/) | Stop charging when a battery cell is over-voltage, without false trips | Battery management systems in EVs | Debounce, hysteresis, plausibility, fail-safe default |

Run everything with `make test` from the repository root.
