# 03. Cell over-voltage protection in a BMS

## The problem

Charging a Li-ion cell above its maximum voltage can damage it and, in the worst case, cause a thermal event. The battery management system (BMS) must **stop charging quickly when a cell is really over-voltage**, but it must **not trip on measurement noise**, and it must **never treat a broken measurement as a healthy cell**.

## Three ideas solve it

**1. Debounce (confirm before acting)**
Measurements have noise spikes. The fault is set only after **3 samples in a row** above 4200 mV (300 ms at a 100 ms cycle). A single spike resets nothing and trips nothing.

**2. Hysteresis (different set and clear limits)**
Set above **4200 mV**, clear only below **4100 mV**, after **5 samples in a row**. Without the gap, a cell sitting near 4200 mV would switch charging on and off repeatedly (chattering), stressing contactors and the cell.

**3. Plausibility (don't trust impossible readings)**
A broken sense wire may read **0 mV**. Without a check, that looks like a perfectly safe cell, and charging would continue while the real voltage is unknown. Any reading below 1000 mV or above 5000 mV is a **sensor fault**, and charging stops.

## State machine

```
            3 samples > 4200 mV
  CELL_OK ───────────────────────▶ CELL_OV_FAULT
     ▲    ◀─────────────────────────     │
     │     5 samples < 4100 mV           │
     │                                    │
     └──── plausible reading ◀── CELL_SENSOR_FAULT ◀── reading < 1000 or > 5000 mV (from any state)
```

## Design decisions

- **Fail safe:** `ov_charging_allowed()` returns true **only** in `CELL_OK`. Any doubt means no charging.
- **Plausibility is checked first,** before any comparison with the limits.
- **Counters must be consecutive:** one good sample resets the count.

The limits here are example values for an NMC cell. Real limits come from the cell datasheet and the project's functional safety concept (ISO 26262).

## Tests prove

A noise spike doesn't trip; three samples do; the fault holds between 4100 and 4200 mV; it clears after five samples below 4100 mV; 0 mV and 6000 mV are sensor faults that stop charging.

## Interview questions this answers

- Why use hysteresis, and what goes wrong without it?
- How do you choose a debounce time for a safety function?
- Why must a 0 mV reading block charging rather than allow it?
