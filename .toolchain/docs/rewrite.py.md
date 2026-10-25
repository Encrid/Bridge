# .toolchain/tools/rewrite.py

`rewrite.py` makes a changed copy of the code, for `test --branches` and for `test --leaks`.
It holds the parts that the two runs share.
`.toolchain/docs/branches.py.md` and `.toolchain/docs/leaks.py.md` tell what each run writes into the copy.

## The steps

1. `sources` finds each artifact that the build makes for this target.
2. It takes each file `.c` of `src` that such an artifact compiles, with the flags of the first such artifact.
3. `each` runs the command `[rewrite] dump` of `toolchain.toml` for each file, 8 files at the same time. It gives each syntax tree to the run.
4. `copied` copies `src` and `include` into `<build>/<run>/tree`. The run then writes its changes into the copies.

`test.py` then builds the copies, and runs the cases: `.toolchain/docs/test.py.md`.

## The writer

`writer` gives the text that ends each changed file `.c`.
A `[[gnu::destructor]]` writes one line for the file, at the exit of the program and at the close of a library.
It writes to the file that a variable of the environment names. If the variable is not set, it writes nothing.
The line starts with the path of the file. Each run gives the rest of the line.

The writer builds each line in a buffer and writes it with one append, so parallel programs do not interleave.
It uses the low-level file calls of the system, with no buffer of `stdio`.

On macOS, a `[[gnu::constructor]]` of each file opens its own library again, with `RTLD_NOLOAD` and `RTLD_NODELETE`.
Thus a closed library stays mapped, and it does not matter which code opens and closes it, for example a released library.
Then its destructor runs at the exit of the program, with the code of the library still in place.
The constructor finds its library with `dladdr`. It declares `dladdr` and `dlopen` with names of its own, so it includes no header.

## The syntax tree

`dump` runs `zig cc` with `-fsyntax-only`, and with the other flags of the artifact.
zig then writes an error ending with `FileNotFound`, because no object is made. `dumped` ignores only such an error.
If clang writes a different error, `dumped` stops: `the tree of <path> cannot be read:`, and the errors.
If the output is not JSON, it stops: `the tree of <path> is not JSON`.

The dump sets `_FORTIFY_SOURCE` to 0. Then a call that the system would fortify, such as `snprintf`, does not read as a macro.

## Limits

- A case that crashes, or that stops before its end, writes no line.
- Every file writes with a `[[gnu::destructor]]`. A destructor of a library that `dlclose` unmaps on macOS would run at the exit of the program with its code gone. So on macOS each file keeps its own library mapped, and the destructor runs in place.

## Functions

| Function | Result |
| --- | --- |
| `tree(values, run)` | Gives the folder of the copies of a run, `<build>/<run>/tree`. |
| `marked(node, state)` | Gives each location of a syntax tree the file that it is in. The dump writes a file only when it changes. |
| `is_macro(loc)` | Tells if a location comes from a macro. |
| `owned(loc, root)` | Gives the path of a location in `src` or in `include`, or `None`. |
| `dumped(command, root)` | Runs the command `dump`, and gives the syntax tree, with the file of each location. |
| `sources(config, values)` | Gives each file `.c` of `src` that an artifact of this target compiles, with its command `dump`. |
| `each(config, values, use)` | Gives the result of `use(root, path, tree)` for each file of `sources`, 8 files at the same time. |
| `copied(root, where)` | Empties `where`, and copies `src` and `include` into it. |
| `placing(text, at)` | Gives the line and the column of a byte. |
| `writer(values)` | Gives the text of the writer of a file, as *The writer* tells. `values` gives the variable, the rest of the line and the array that the macOS constructor finds. |
