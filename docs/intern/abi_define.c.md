# src/abi_define.c

This file defines the eight imports of the ABI for the instance of a module, each with the signature that it is declared with.

## The signatures

The two directions across the backend seam are not the same.
Bridge declares the signature of each import that it gives to a module, and the signatures are different.
A backend checks each export of a module that Bridge calls, and each export is `() -> i32`.

So `define_func` carries a `backend_sig_t` for each import: its parameters and its results.
A number of arguments is not enough: five entries give a value, and the arguments have different widths.
There are two types, `BACKEND_I32` and `BACKEND_I64`.
An offset, a length and a slot are 32 bits, and an entity id is 64 bits.
No entry takes or gives a float.

The signature goes to the backend as data, and not as a type of one engine.
So each backend makes of it what it needs, and a backend that is told the signature cannot drift from it.
A backend that checks the shape of an import refuses a module that declares an entry with another signature, when the module is instantiated.
Bridge defines all the imports right after it makes the instance, before it finds the first function of the module.

A function that Bridge gives to a module gives a status beside its results.
Non-zero says that the call that reached it must not go on.
A backend that can unwind a module does it there: the wasm backend raises a trap.
A backend that cannot lets the call return, and Bridge fails the invocation when it does.

## The entries

```text
declare       (I32)              -> I32
add_system    (I32)              -> I32
observe       (I32)              -> I32
spawn         ()                 -> I64
destroy       (I64)
attach        (I64, I32, I32)
detach        (I64, I32)
alloc         (I32)              -> I32
```

Each import is in the namespace `BRIDGE_NAMESPACE`, which is `bridge`, under the name of its entry.
Each is defined with the module as `ud`, so the entry is handed the module when it is called.
If the backend refuses an import, `abi_define` stops there and gives 1, with `err`.

## Constants

| Constant | Value |
| --- | --- |
| `ABI_ENTRY_COUNT` | 8, the number of entries. |
| `I`, `L` | `BACKEND_I32` and `BACKEND_I64`, short names for the table of entries. |
| `ABI_ENTRIES` | The eight entries, each an `abi_entry_t`: its name, its function and its signature. |

## Functions

| Function | Result |
| --- | --- |
| `abi_define` | Defines the eight imports for the instance of a module, each handed the module when it is called. 1, with `err`, if the backend refuses one. |
