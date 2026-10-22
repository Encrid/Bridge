# src/host/config_checks.c

This file reads and checks the `[checks]` table, which holds the diagnostic switches of a world.

`take_checks` copies the `checks.profile` and `checks.validate` flags into the host, and leaves each off when no file states it.
`config_check_checks` allows `profile` and `validate`, each a flag at depth two, and refuses a key or a depth that is not that.
The local file may not carry a `checks` key: a world fixes its checks when it is made.

## Functions

| Function | Result |
| --- | --- |
| `take_checks` | Reads the profile and validate switches into the host. |
| `config_check_checks` | Checks a key of the checks table, or says why not. |
