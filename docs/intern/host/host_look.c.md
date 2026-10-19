# src/host/host_look.c

This file does one look: it walks the content, meets the changed files, and loads them.

## The look

`look` clears the seen marks, walks the content with `see_file`, then drops the files that are gone.
`see_file` reads a file, finds its slot, and meets it when the hash mark changes.
`pass_file` reports `too many files: 32 are looked at` when no slot is free.
`drop_gone` unloads a module whose file is gone. `order_files` sorts the found files by identity, then by path.

## Functions

| Function | Result |
| --- | --- |
| `look` | Does one look and gives its work count, or -1 when a path fails. |
