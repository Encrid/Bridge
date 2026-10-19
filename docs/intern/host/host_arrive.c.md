# src/host/host_arrive.c

This file asks whether an arriving module may load.

`may_arrive` loads at once when the arrive rule is `load`, and asks when the rule is `ask`.
`is_trusted` prints `? trust <identity>, <backend>, <file>` and reads a line, accepting only `yes`.
A refusal prints `not loaded: <file>: <identity>, <backend>, not trusted`.

## Functions

| Function | Result |
| --- | --- |
| `may_arrive` | Gives true when the module may load under its arrive rule. |
