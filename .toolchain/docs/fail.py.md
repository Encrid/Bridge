# .toolchain/tools/fail.py

`fail.py` makes a listed call of the code fail in a case: `docs/contributing/tests.md`, *Failed calls*.

## The calls

`listed` gives the functions that can fail, each with its failure value.
`malloc`, `calloc` and `realloc` are always in the list, with the value `nullptr`, and so are the two loaders,
`GetProcAddress` and `dlsym`, with `nullptr`.
`[fail.calls]` adds more functions, and each file of `[fail] with` adds more: `.toolchain/docs/project.toml.md`, *[fail]*.
A failure value is a text of C, for example `nullptr`, `-1` or `false`. Each file that calls the function must compile it.

Each function is in the list one time. `listed` stops if a table names a function of the list again: `[fail.calls] lists <name> two times`.

`fail` of a case also fails a call through a pointer variable that `[fail.calls]` lists: `.toolchain/docs/rewrite.py.md`, *The pointer variables*.

## The layer

`layer` gives the layer of a file `.c` of `src`. The layer can make each listed function that the file uses fail.

- After the last `#include`, the layer gives each such function a new name: `.toolchain/docs/rewrite.py.md`, *The rename*.
- At the end of the file, a `static` function with the new name adds 1 to the count of the calls of its function.
- When the count is the number that the case states, that function gives the failure value, and does not call its function. Else it calls its function.
- The count is a `[[gnu::weak]]` variable with hidden visibility. Thus the files of one program or of one library share one count, on each system.
- A `[[gnu::constructor]]` reads the number of each such function from the variable `C23_FAIL` of the environment. A function with no number never fails.
- The layer holds the parts of the artifacts that compile the file, as a text with a space before and after each name.
  The constructor reads `C23_FAIL` only if the variable `C23_FAIL_PART` is not set, or if the text holds its value. Else no call of the file fails.

For each call that the key `fail` of the case states, `C23_FAIL` holds a space, then `<function>=<number>`.
`C23_FAIL_PART` holds the part of the case, with a space before and after it. Thus a case refuses calls only in the programs of its own part, in each run.
`test.py` gives the variables to the command of the case: `.toolchain/docs/test.py.md`, *A case*.

`layer` stops if a listed function that a file uses has no result: `<name> gives no result`.
It also stops where `signed` of `rewrite.py` stops: `.toolchain/docs/rewrite.py.md`, *The rename*.
`listed` stops if a file of `[fail] with` is not there: `[fail] with: <file> is not there`.

## The loaded calls

A loader gives a pointer for the name of its second argument: `GetProcAddress` on Windows, and `dlsym` on Linux and macOS.
A **loaded call** of a file is a listed function that the file names in a text literal, and does not call by its name.
A file with no call of a loader has no loaded call.
For each loaded call, the layer adds to the end of the file:

- the count and the number of `C23_FAIL`, as for a listed call;
- a pointer `typeof(*<function>) *c23b_fp_<function>`. That type is a pointer to the function, also when the name is a pointer variable: `.toolchain/docs/rewrite.py.md`, *The pointer variables*;
- a stand-in `c23b_ft_<function>` with the parameters of its declaration. It counts the call,
  gives the failure value at its number, and else calls the pointer.

The function of the new name of the loader counts as always, then calls the loader.
If the name is a loaded call, it keeps the pointer, and gives the stand-in in its place.
Thus `fail = { socket = 1 }` fails the first call of `socket` through its pointer, with the value of `[fail.calls]`.
A loaded call that no case states calls its function as always.

## The runs

| Run | The copy of the code |
| --- | --- |
| `test` | `failed` writes the layer into a copy in `<build>/fail/tree`. `test.py` builds the copy, and runs on it each case that states `fail`. |
| `test --branches` | `probed` of `branches.py` writes the layer and the probes into one copy. |
| `test --leaks` | `counted` of `leaks.py` writes the layer and the count into one copy. The count calls the layer, so a failed call takes nothing. |

`.toolchain/docs/test.py.md`, *main*, tells when `test` makes the run `fail`.

## Limits

- A call before the last `#include` of its file cannot fail, and a call in a function of a header cannot fail.
- Only the calls of the files `.c` of `src` count. A call in `tests` or in a different project does not count.
- A listed function that no file of `src` calls never fails. Thus a case can state the functions of each system, for example `CreateThread` and `pthread_create`.

## Functions

| Function | Result |
| --- | --- |
| `listed(config, values)` | Gives each function that can fail, with its failure value. It reads the files of `[fail] with`. |
| `loaded_calls(calls, used, source, declared)` | Gives the loaded calls of a file: the listed functions that the text `source` names, and that the file does not call. |
| `layer(text, used, declared, calls, parts)` | Gives the layer of a file: the byte where its block starts, the block, and the end of the file. `used` names the functions of `calls` that the file uses, and `declared` gives their declarations. `parts` names the parts of the file. |
| `failed(config, values)` | Writes the copy of the run `fail`, as *The runs* tells. |
