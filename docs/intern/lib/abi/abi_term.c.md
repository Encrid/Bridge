# src/lib/abi/abi_term.c

This file checks a term of a system or an observer.

## A term

A term is a `bridge_term_t` of three things: the slot that `declare` gave, the access, and the operator.
The operator says whether the term must match, `BRIDGE_AND`, must not, `BRIDGE_NOT`, or may, `BRIDGE_OPTIONAL`.
The terms are an array of structs and not a string, so Bridge has no parser for a query.
The access is read or write, and the scheduler reads it to know what may run at once.

## What a query may not ask for

A term is one declared type.
There is no term for "this type or that one", and none for "any type".
The component that such a term matched would change from table to table.
But a module reads a field through an accessor that has its type at the build.

These terms are refused, each naming the term:

- A term of a slot that the module never declared.
- A term whose access or operator is unset, or is not one there is.
- A term that writes a component that the module may not write.
- A term of a system that selects by what a system does not walk.
- A term of a sparse set that is negated.

A system walks tables, or one sparse set alone.
The rows of a table and the entries of a set are in different orders, so one loop cannot step through both.
So the terms that select, `BRIDGE_AND` and `BRIDGE_NOT`, are all of components kept in tables.
Or there is one term that selects, a `BRIDGE_AND` of a component kept in a sparse set, and every other term is optional.

A term of a component kept elsewhere is stated optional, and is reached from the row.
A singleton is on no entity, and is read where it is.
A direct-indexed component is reached by the id of the entity.
A component kept in a sparse set is looked for at each row, or its term is the one that selects.
`docs/extern/bridge_abi.h.md`, *A term*, quotes why Bridge refuses a negated term of a sparse set.

An observer walks nothing, so the rules of what a system walks do not apply to it.
If the call in progress has a refusal already, `abi_check_term` refuses the term.

## Functions

| Function | Result |
| --- | --- |
| `check_values` | Refuses a term whose access or operator is unset, or is not one there is. |
| `selects_alone` | Whether term `i` is the one term that selects: every other term is optional. |
| `check_sparse` | Refuses a term of a sparse set that selects beside another term, or that is negated. |
| `check_walked` | Refuses a term of a system that selects by what a system does not walk. |
| `abi_check_term` | Refuses term `i` if it asks for what a query may not. 1 with a refusal. |
