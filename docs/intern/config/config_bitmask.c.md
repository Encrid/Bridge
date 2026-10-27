# src/config/config_bitmask.c

This file writes a setting into a bitmask field.

## A bitmask

A bitmask takes an integer whose bits are all bits of its constants.
A bitmask is the unsigned integer that its author stated. A bitmask of n bytes holds from 0 to 2^(8n) - 1.

`config_seed_bitmask` does these steps in order. Each refusal ends them:

1. Refuse a setting that is not an integer as a misfit.
2. Refuse an integer below 0, or above the most that the bitmask holds.
3. Refuse an integer with a bit that no constant has.
4. Write the value into the field: `docs/intern/config/config_scalar.c.md`, *An integer*.

`docs/extern/bridge.h.md`, *The settings*, gives the reason of each refusal.
`get_bitmask_bits` gives the bits that the constants have, for step 3: the bitwise OR of their values.
The value 0 sets no bit. Thus each bitmask takes it.
No setting sets bit 63 of a bitmask of 64 bits: a setting is an `int64_t`, and step 2 refuses a value below 0.

## Functions

| Function | Result |
| --- | --- |
| `get_bitmask_bits` | Gives each bit that a constant of a bitmask has: the bitwise OR of the values of its constants. |
| `config_seed_bitmask` | Writes an integer into a bitmask, if the bitmask holds it and each of its bits is a bit of a constant. Gives 1 if it refuses. |
