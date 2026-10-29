# .toolchain/tools/fail.py

`fail.py` makes a listed call of the code fail in a case: `docs/contributing/tests.md`, *Failed calls*.

## The calls

`listed` gives the functions that can fail, each with its failure value.
`malloc`, `calloc` and `realloc` are always in the list, with the value `nullptr`.
`[fail.calls]` adds more functions, and each file of `[fail] with` adds more: `.toolchain/docs/project.toml.md`, *[fail]*.
A failure value is a text of C, for example `nullptr`, `-1` or `false`. Each file that calls the function must compile it.

Each function is in the list one time. `listed` stops if a table names a function of the list again: `[fail.calls] lists <name> two times`.

## The layer

`layer` gives the layer of a file `.c` of `src`. The layer can make each listed function that the file uses fail.

- After the last `#include`, the layer gives each such function a new name: `.toolchain/docs/rewrite.py.md`, *The rename*.
- At the end of the file, a `static` function with the new name adds 1 to the count of the calls of its function.
- When the count is the number that the case states, that function gives the failure value, and does not call its function. Else it calls its function.
- The count is a `[[gnu::weak]]` variable with hidden visibility. Thus the files of one program or of one library share one count, on each system.
- A `[[gnu::constructor]]` reads the number of each such function from the variable `C23_FAIL` of the environment. A function with no number never fails.

For each call that the key `fail` of the case states, `C23_FAIL` holds a space, then `<function>=<number>`.
`test.py` gives the variable to the command of the case: `.toolchain/docs/test.py.md`, *A case*.

`layer` stops if a listed function that a file uses has no result: `<name> gives no result`.
It also stops where `signed` of `rewrite.py` stops: `.toolchain/docs/rewrite.py.md`, *The rename*.
`listed` stops if a file of `[fail] with` is not there: `[fail] with: <file> is not there`.

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
| `layer(text, used, declared, calls)` | Gives the layer of a file: the byte where its block starts, the block, and the end of the file. `used` names the functions of `calls` that the file uses, and `declared` gives their declarations. |
| `failed(config, values)` | Writes the copy of the run `fail`, as *The runs* tells. |
