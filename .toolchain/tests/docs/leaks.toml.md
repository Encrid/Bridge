# .toolchain/tests/cases/leaks.toml

The group `leaks` checks `.toolchain/tools/leaks.py`, through `test --leaks` of `.toolchain/tools/test.py`.
Each case runs `test.py` on a project of `.toolchain/tests/data/leaks`, with its output in `{case}/out`.

| Project | Content |
| --- | --- |
| `held` | The program `held`, from six files of `src` and `tests/grab.c`. Its first argument picks what it does, and each case of the project gives one argument. The case `silent` runs Python, which writes no line. |
| `opened` | The library `chunk`, which takes memory in `chunk_make` and frees it in `chunk_drop`, and the test program `loader`. `loader` opens the library, calls it and closes it. |
| `excused` | The program `keep`, which keeps one block of memory, and an exemption of `project.toml` that excuses it. |
| `badexempt` | The same program, with an exemption that states no reason and an exemption for a kind that no case keeps. |
| `systemed` | The same program, with an exemption for the system `elsewhere`. |

The case `whole` of `held` frees each block that it takes: with `free`, and with `free` given as a pointer.
It takes a block in `src/pool.c` and frees it in `src/main.c`, and it opens and closes a box of the pair `box_open`.
The last `#include` of `src/main.c` is in an `#if` that holds only lines of the preprocessor.
The includes and the code of `src/wrapped.c` are in one `#if`. `src/plain.c` uses no counted function.
`box_close` gives back a box and returns a number, and `free` returns nothing.
`[leaks] with` of `held` names the file of the variable `counted`, which is `tests/leaks.toml` by default.
That file names `grab` as a function that takes memory, and the pair `box_open` and `box_close`.
`grab` and `seize` of `tests/grab.c` take memory with no count. The case `named` frees the result of `grab`, and `foreign` frees the result of `seize`.

| Case | Claim | False variant |
| --- | --- | --- |
| a case that gives back what it takes | `test` on the case `whole` writes `leaks: 1 cases, 1 give back what they take`, and ends with 0. | The case `kept`. |
| a case that writes no line | `test` on `silent` writes `leaks: 1 cases, 1 give back what they take`, and ends with 0. | The case `kept`. |
| a case that keeps memory | `test` on `kept` writes `leaks: kept keeps 1 memory`, and stops. | The case `whole`. |
| a call of realloc with 0 bytes | `test` on `zero` writes `leaks: zero calls realloc with 0 bytes`, and stops. | The case `whole`. |
| memory that no count took | `test` on `foreign` writes `leaks: foreign gives back 1 memory that it did not take`, and stops. | The case `named`. |
| a function of [leaks] memory takes memory | `test` on `named` writes `leaks: 1 cases, 1 give back what they take`, and ends with 0. | The case `foreign`. |
| a pair keeps a count of its own | `test` on `boxed`, which does not close its box, writes `leaks: boxed keeps 1 box_open`, and stops. | The case `whole`. |
| a helper is counted | `test` on `helped`, whose helper keeps a block, writes `leaks: helped keeps 1 memory`, and stops. | The case `whole`. |
| a line that does not fit the kinds | `test` on `spoiled`, which adds a line for a file that is not counted and a line with one count, stops: `a line of src/main.c does not match its counts`. | The case `whole`. |
| a closed library gives its counts | `test` on the case `dropped` of `opened` writes `leaks: 1 cases, 1 give back what they take`, and ends with 0. | The case `kept`, which closes the library with its memory. |
| an exemption excuses a kept resource | `test` on `excused` writes `leaks: 1 cases, 0 give back what they take, 1 exempt`, and ends with 0. | The project `badexempt`. |
| an exemption for a different system does not apply | `test` on `systemed` writes `leaks: kept keeps 1 memory`, and does not write that the exemption excuses nothing. | The project `excused`. |
| an exemption with no reason or no target is a fault | `test` on `badexempt` writes the kept memory, `states no reason` and `excuses no kept resource`, and stops. | The project `excused`. |
| leaks with a false variant | `test --leaks --break` stops: `--leaks runs each case one time`. | No `--break`. |
| a file of [leaks] with that is not there | With `--set counted=tests/none.toml`, `test` stops: `[leaks] with: <file> is not there`. | No `--set`. |
| a take that gives no pointer | With `tests/misfit_take.toml`, the pair `box_count` gives an `int`, and `test` stops: `box_count takes a resource, and does not give a pointer`. | No `--set`. |
| a give that gets no pointer first | With `tests/misfit_give.toml`, the give `box_number` gets an `int` first, and `test` stops: `box_number gives back a resource, and does not get a pointer first`. | No `--set`. |
| a counted function with no fixed list of arguments | With `tests/misfit_list.toml`, `box_format` takes memory and has `...`, and `test` stops: `box_format takes a list of arguments of no fixed length`. | No `--set`. |
