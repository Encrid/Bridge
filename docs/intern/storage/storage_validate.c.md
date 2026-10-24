# src/storage/storage_validate.c

This file hashes the column of each live table, and tells which column changed since it last looked.
The checksum pass uses it to find a write that no running system declared:
`docs/intern/schedule/schedule_validate.c.md`.

## What it covers

It hashes a column of each type that a live table keeps, from the first table.
A type with no bytes hashes to a constant, so it never looks changed: a tag, or a toggle.
It does not hash the entity index, the string arena, a singleton or a direct-indexed cell.
The index has its own cheap check, and a wrong string shows itself to whoever reads it.

## The baseline

`col_hash` holds the last hash of each column, beside `columns` and with the same shape.
`storage_validate_scan` walks the live tables and scans each one.
A scan writes a hash that changed, and tells the caller the type of that column.
`storage_validate_baseline` scans with no caller, so it only writes the new hashes.
The barrier calls the baseline after it applies the queues, so the applied writes become the baseline, named to nobody.

## Functions

| Function | Result |
| --- | --- |
| `get_col_hash` | The row of last hashes for a table, beside its columns. |
| `hash_column` | The hash of the bytes of one column: its type's size for each row. |
| `scan_table` | Hashes each column of a table, stores a hash that changed, and tells the caller its type. |
| `storage_validate_scan` | Scans each live table, and tells the caller each type whose column changed. |
| `storage_validate_baseline` | Stores the hash of each column and tells nobody, so the columns become the baseline. |
