# src/abi_relay_fields.c

This file matches the fields of a struct as it was, by name, to the fields of what it becomes.
`docs/intern/abi_relay.c.md`, *What each change does*, gives the rules.

## A field matched

For each field of the struct as it was, in order:

1. Name the field by its path: the type, then each field, joined by dots, for example `game:Body.at.z`.
2. Find the field of that name in what the struct becomes. If there is none, record a loss.
3. Refuse a field of another type: bytes of one type are not a value of another.
4. If the array is now shorter, record its end as a loss.
5. If the field is a struct that this reload changed, match its fields, element by element.
6. Else record a run: the elements that both have, from the old offset to the new one.

What is new in the struct is in no run, so it stays zero.

A field is of the type that it was if all of these are true:

- It is of the same kind.
- Where it names a declared type, it names that type, or the type that this reload made of it.
- It has the same size, unless it is a struct.

## Functions

| Function | Result |
| --- | --- |
| `find_field` | The field of a name in a struct, or nullptr. |
| `is_same_type` | Whether a field is of the type that it was. |
| `relay_structs` | Matches a field that is a struct this reload changed, element by element: the same rules, one level down. 1 with a refusal. |
| `relay_field` | Matches one field of a struct as it was. 1 with a refusal. |
| `abi_relay_fields` | Matches the fields of a struct as it was to those of what it becomes. 1 with a refusal. |
