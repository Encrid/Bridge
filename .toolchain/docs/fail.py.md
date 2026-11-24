# .toolchain/tools/fail.py

`fail.py` makes a listed call of the code fail in a case: `docs/contributing/tests.md`, *Failed calls*.

## The calls

`listed` gives the functions that can fail, each with its failure value.
`malloc`, `calloc`, `realloc`, `aligned_alloc`, `strdup` and `strndup` are always in the list, with the value `nullptr`.
The two loaders `GetProcAddress` and `dlsym` are always in the list too, with `nullptr`.
`[fail.calls]` adds more functions, and each file of `[fail] with` adds more: `.toolchain/docs/project.toml.md`, *[fail]*.
A failure value is a text of C, for example `nullptr`, `-1` or `false`. Each file that calls the function must compile it.

A function with no result has the empty text as its value. No case can make it fail, and a case can hold it or wait for it: *The hold*.

Each function is in the list one time. `listed` stops if a table names a function of the list again: `[fail.calls] lists <name> two times`.

`fail` of a case also fails a call through a pointer variable that `[fail.calls]` lists: `.toolchain/docs/rewrite.py.md`, *The pointer variables*.

## The layer

`layer` gives the layer of a file `.c` of `src`. The layer can make each listed function that the file uses fail.

- After the last `#include`, the layer gives each such function a new name: `.toolchain/docs/rewrite.py.md`, *The rename*.
- At the end of the file, a `static` function with the new name adds 1 to the count of the calls of its function.
- When the count is the number that the case states, that function gives the failure value, and does not call its function. Else it calls its function.
- The count is a `[[gnu::weak]]` variable with hidden visibility. Thus the files of one program or of one library share one count, on each system.
- A `[[gnu::constructor]]` reads the number of each such function from the variable `C23_FAIL` of the environment. A function with no number never fails.
- At each call, the layer reads the turn of `C23_FAIL`: *The turn*.
- The layer holds the parts of the artifacts that compile the file, as a text with a space before and after each name.
  The constructor reads `C23_FAIL` only if the variable `C23_FAIL_PART` is not set, or if the text holds its value. Else no call of the file fails.

For each call that the key `fail` of the case states, `C23_FAIL` holds a space, then `<function>=<number>`.
`C23_FAIL_PART` holds the part of the case, with a space before and after it. Thus a case refuses calls only in the programs of its own part, in each run.

A case that states `fail_parts` names the parts itself, and `C23_FAIL_PART` holds each of them, with a space before and after each name.
Thus a case of a program can refuse the calls of a library of another part that the program links. The files of 1 program share 1 count, so the number counts the calls of each named part.
The constructor reads `C23_FAIL` if the text of the file holds 1 of the names.
A run with `--part` also builds the layer for each part that a chosen case names in `fail_parts`.
`test.py` gives the variables to the command of the case: `.toolchain/docs/test.py.md`, *A case*.

`layer` stops if a listed function that a file uses has no result and a value: `<name> gives no result, so its value in [fail.calls] is the empty text`.
It stops if such a function has a result and the empty text: `<name> gives a result, so its value in [fail.calls] is not the empty text`.
It stops if the path of the file holds a space: `<path> holds a space, and the layer names each file in a key of C23_FAIL`.
It also stops where `signed` of `rewrite.py` stops: `.toolchain/docs/rewrite.py.md`, *The rename*.
`listed` stops if a file of `[fail] with` is not there: `[fail] with: <file> is not there`.

## The turn

A test program can change the calls that fail during its run: `.toolchain/docs/sweep.h.md`.
For that, `C23_FAIL` can hold a space, then `turn=<number>`. With no turn, the turn is 0.

At each call of a listed function, the layer reads the turn.

- If the turn is not the turn that the file read last, the file reads its numbers again, as its constructor does.
- If the turn is not the turn of the count of the function, the count starts again at 0. The turn of a count is a `[[gnu::weak]]` variable with hidden visibility, as the count is.
- When the layer makes a call fail, it looks for ` told=0` in `C23_FAIL`, and writes `1` in place of the `0`.
  It changes 1 byte of the text, and calls no function that changes the environment. Thus a call can fail on any thread.

Thus a case that states `fail` runs as before: its turn stays 0, and its `C23_FAIL` holds no ` told=0`.

## The hold

A case can make 1 call wait for calls on another thread: the key `hold` of a case, `.toolchain/docs/test.py.md`, *A case*.
Thus a case can give 2 threads the same order in each run, with no wait of a time.

`C23_FAIL` then holds ` hold@<file>@<function>=<at> until@<until>=1 @times=<times>`.

- Call `<at>` of `<function>` in `<file>` waits. The count of a hold is a `static` variable of the file, so the calls of other files do not count.
- The count of the releases is a `[[gnu::weak]]` variable with hidden visibility, so the files of 1 program share it.
- Each call of `<until>` in a file of the layer adds 1 to it, after the call returns. A call that the layer makes fail adds nothing, since it never runs. A call that runs adds 1, also when it gives an error.
- The held call goes on when the count of the releases is `<times>` or more. Thus a release that comes before the hold also counts.
- The wait gives the processor to another thread on each turn: `SwitchToThread` on Windows, and `sched_yield` on the other systems.
- Under `__wasm__` there is no hold, since a module has 1 thread.

If the releases never reach `<times>`, the case waits until its timeout, and fails.

## A wasm module

A wasm module has no environment. Thus under `__wasm__` the layer reads its plan from the area of the module: `.toolchain/docs/rewrite.py.md`, *The area*.
wasm-ld calls the constructors at the start of each export of a module with no entry. Thus under `__wasm__` the function that reads the numbers is not a constructor.

- The plan holds the value of `C23_FAIL`, a byte 0, the value of `C23_FAIL_PART`, and a byte 0: `.toolchain/docs/outcomes.h.md`, *The plan*.
- The host of the module writes the plan before `init`: `.toolchain/docs/outcomes.h.md`, *A host of wasm modules*.
- With no plan, the area holds only bytes 0. Thus no call fails. A wasm source can call a listed function, such as `malloc`, also with a host that writes no plan.
- The layer reads the plan at the first call of a listed function, and at each new turn.
- It reads the texts with loops of its own, and includes no header but `stddef.h`. Thus it needs no library of C.

A number of the plan is the digits after `<function>=`.

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
- A call of a wasm module fails only with a host that writes the plan into the area of the module.
- A wasm module cannot run a sweep. The layer writes ` told=1` only into the plan in the memory of the module.
  The steps of the host do not read the plan back: `.toolchain/docs/outcomes.h.md`, *The steps of the host*. Thus `.toolchain/docs/sweep.h.md` cannot see a failed call.

## Functions

| Function | Result |
| --- | --- |
| `listed(config, values)` | Gives each function that can fail, with its failure value. It reads the files of `[fail] with`. |
| `loaded_calls(calls, used, source, declared)` | Gives the loaded calls of a file: the listed functions that the text `source` names, and that the file does not call. |
| `wrapped(text, outer)` | Gives the bytes of a file with its layer: the block at its byte, and the end after the text. |
| `layer(text, used, declared, calls, parts)` | Gives the layer of a file: the byte where its block starts, the block, and the end of the file. `used` names the functions of `calls` that the file uses, and `declared` gives their declarations. `parts` names the parts of the file. |
| `failed(config, values)` | Writes the copy of the run `fail`, as *The runs* tells. It joins the trees of a file with `rewrite.united`. |
