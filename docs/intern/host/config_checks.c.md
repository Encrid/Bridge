# src/host/config_checks.c

This file reads and checks the `[checks]` table, which holds the diagnostic switches of a world.

`take_checks` copies the `checks.profile` flag into `profile` of the host, and leaves it off when no file states it.
`config_check_checks` allows `profile`, a flag at depth two, and refuses a key or a depth that is not that.
The local file may not carry a `checks` key: a world fixes its checks when it is made.

## Functions

| Function | Result |
| --- | --- |
| `take_checks` | Reads the profile switch into the host. |
| `config_check_checks` | Checks a key of the checks table, or says why not. |
