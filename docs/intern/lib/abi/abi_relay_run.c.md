# src/lib/abi/abi_relay_run.c

This file records what a type laid out again keeps and what it loses.
It records each run of bytes carried over, and each loss: one that the reload accepts, or one that the refusal says.

## A run

A run says that `len` bytes at `from` of the old value are the bytes at `to` of the new value.
The runs of a type are a list of `storage_run_t`, which storage reads when it writes the rows again.
The size of one element of a field is what its kind says, or the size of the declared type that it names.
If there is no memory for a run, the relay is marked `short_of`, and refused when it is done.

## A loss stated by name

A reload that would drop values is refused until it states the loss.
The first try is refused, and the refusal names every field that would go, and how many rows hold the type.
A reload that states the loss carries the rows over, and `src/lib/reload/reload_rows.c` says each loss in a record, with its rows.
A change that drops nothing needs no statement: a field added, fields in another order, an array made longer.

The configuration of the module states the losses: `losses`, with `loss_count` of them, or `BRIDGE_NONE` for none.
A loss is named by the component and the field, for example `game:Position.z`, and one level down `game:Body.at.z`.
The end of an array made shorter is named as its field is.
For a toggle that the component leaves, the loss of which rows were off is the name of the component alone.

A loss is a list of names, and not a yes or a no, because the rule is for a rename.
A field renamed is a field removed and a field added, and a yes would let a rename drop values with no word.
Bridge does not drop data on its own account in a reload.
A field that the new struct does not have was removed by whoever edited the struct, and the record tells what went.

A loss counts only if a row holds the type.
`abi_accepts` records each accepted loss that the reload makes, in `loss_rows`: the rows plus one.
`src/lib/module/module_start.c` refuses a loss that is accepted and not made, so that a list does not stay from one reload to the next.
`abi_say_lost` writes each loss that is not accepted into `lost`, until `lost` is full.

## Constants

| Constant | Value |
| --- | --- |
| `ELEMENT_SIZES` | The bytes of one element by kind: 1 for a bool, a char and an 8-bit integer, 2 and 4 for 16 and 32 bits. |

A float is 4 bytes in `ELEMENT_SIZES`, and a 64-bit integer, a double and a `bridge_text_t` are 8.
A struct has no size there: it is the size of the declared type that the field names.

## Functions

| Function | Result |
| --- | --- |
| `abi_accepts` | Whether the reload accepts the loss named `name`. If it does, records how many rows lose something under that name. |
| `abi_get_element_size` | How many bytes one element of a field is: what its kind says, or the size of the declared type that it names. The kind of a field is at most `SCHEMA_STRUCT`, since the parser sets no other. |
| `abi_add_run` | Records a run of bytes carried over, or marks the relay short of memory. |
| `abi_say_lost` | Records by name what has no place in what the type becomes, unless no row holds the type or the reload accepts the loss. |
