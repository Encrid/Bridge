# src/host/manifest/manifest_take.c

This file takes one author key of a manifest into `manifest_t`.

`take_number` reads `abi` as an integer that is not negative.
`take_word` copies `config` or `settings` when the value fits the field.
`take_flag` reads `pinned` or `reloadable` as `true` or `false`, and marks that the key was present.
`manifest_take` tries each taker in order.

## Functions

| Function | Result |
| --- | --- |
| `manifest_take` | Gives 1 when a key is taken, 0 when it is unknown, and -1 when the value is wrong. |
