# .toolchain/runtime/counts.h

`counts.h` writes the line of the counts of 1 file into the file of the leaks of a case, for `test --leaks`.
The writer of `rewrite.py` includes it at the end of each file of the count: `.toolchain/docs/rewrite.py.md`, *The writer*.
A host of wasm modules calls it for each chunk of counts: `.toolchain/docs/outcomes.h.md`, *A host of wasm modules*.
Thus the file of the leaks has 1 writer.

## The room of the line

The macro `C23_COUNTS_ROOM(length, slots)` gives the number of bytes that `line` needs, for a path of `length` bytes and `slots` counts: `length + 2 + 22 * slots`.
It is a constant expression. Thus the writer of `rewrite.py` gives it the size of its array, and a host computes it at the run.

## Functions

| Function | Result |
| --- | --- |
| `c23_counts_put_number(line, at, n)` | Writes the number `n` in decimal into `line` at `at`, with a `-` before a number below 0. It gives the byte after the number. |
| `c23_counts_append(line, length)` | Appends `length` bytes of `line` to the file that `C23_LEAKS` names, with 1 write. It tells if it wrote each byte. |
| `c23_counts_keep(line, path, counts, slots)` | Writes the line of the file `path` into `line`, and appends it with `c23_counts_append`. It tells if it wrote the line. |

## The line

The line is the path of the file, then each count after a space, then a line end. For example, `src/parse.c 0 0` holds 2 counts.
`line` must hold `C23_COUNTS_ROOM(length, slots)` bytes, with `length` the number of bytes of `path`.
`.toolchain/docs/leaks.py.md`, *The report*, tells how the run adds the lines.

## The append

`c23_counts_append` writes nothing, and tells it, if `C23_LEAKS` is not set or the file does not open.
It writes the line with 1 append, so programs that end at the same time do not mix their lines.
On Windows, an append is a seek and a write, so 2 programs that end at the same time can write over each other.
Thus on Windows it opens the file with `_SH_DENYRW`, and opens it again while another program holds it.
It uses the low-level calls of the system, with no buffer of `stdio`, and it takes no memory.
