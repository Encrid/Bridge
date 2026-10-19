# src/host/config_file.c

This file loads one file from disk and adds it to the `config_t`.

`read_text` reads the whole file, and treats an empty file as empty text.
`config_add_file` stores the path, the folder and the whole path, then parses the text with `toml_read`.
It prints `cannot read '<path>'` when the load fails, and the reader's reason when the parse fails.
`config_find_whole` looks a file up by its whole path. `config_say_chain` prints the files of a naming loop.

## Functions

| Function | Result |
| --- | --- |
| `config_add_file` | Adds a parsed file, or gives -1 on a load or parse error. |
| `config_find_whole` | Gives the index of a file with that whole path, or -1. |
| `config_say_chain` | Prints the chain of files from one file back to another. |
