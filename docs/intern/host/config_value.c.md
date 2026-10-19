# src/host/config_value.c

This file checks a value against the rule of its key.

## The rules

`RULES` holds one `value_rule_t` per `enum check`: the allowed kinds, the bounds, the words, and the reason.
`is_allowed` branches on the kind of the value and weighs it against the rule.
`is_word` accepts a text that matches one of a space-parted word list, such as `none` or `ask`.
`is_names` accepts a list of texts that are not empty. `is_pairs` accepts a list of two-name pairs.
`config_check_value` returns the reason of the rule when the value is not allowed.

## Functions

| Function | Result |
| --- | --- |
| `config_check_value` | Gives null when a value fits its check, else the reason. |
