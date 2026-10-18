# src/lib/module/module_identity.c

This file checks the identity of a module, before the rest of its config.

## The form

An identity has the form `namespace:name@version`: one `:`, then one `@`, and 1 byte or more in each of the three parts.
So the namespace ends at the `:`, and the stem, the identity without the version, ends at the `@`.
No part holds a `:` or an `@`, so an identity has one reading.
Bridge reads nothing in the version.

## Functions

| Function | Result |
| --- | --- |
| `has_form` | Gives whether an identity has the form that *The form* tells. |
| `module_check_identity` | Refuses a module with no identity, with an identity of more than 127 bytes, or with one of another form. Gives 1 with a record. |
