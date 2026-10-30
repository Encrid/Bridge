# .toolchain/tools/style.py

`style.py` checks the style of the C code of the project.
`docs/contributing/coding_style.md` gives the rules. `.toolchain/tools/c23.py` checks the spellings of C23.

```sh
python .toolchain/tools/style.py [--root <folder>]
```

## The files

`style.py` checks each file `.c` and `.h` that `ours` gives.
`[style]` of `.toolchain/toolchain.toml` gives the limits of the rules F3, F4, F5, O3 and C1.

Before the checks, `style.py` hides the text of each comment, each string and each character constant, so no check reads it.
A character constant can have the prefix `L`, `u`, `U` or `u8`, for example `L'?'`.
A `'` after a digit or a letter is a digit separator of C23, as in `1'000` and `0xFF'FF`. Only a prefix that stands alone starts a character constant.

## The checks

| Rule | The check |
| --- | --- |
| O1, O4 | `.toolchain/tools/layout.py` checks them, because they apply to each file of the project. |
| O2 | `style.py` reads only the files `.c` and `.h`. A person checks the other files of `src`. |
| O3 | The number of the functions of a file. |
| G1 | The legal notice of `.toolchain/header.txt`, between `/*` and `*/`, with each line after `** `. |
| G2 | One empty line after each function that is not at the end of the file. |
| G3 | The level of each directive, and no space after `#`. |
| G4 | Each global variable that `const` or `constexpr` does not mark. |
| G5 | Each function of a file `.c` that is not `static`, if no header of the project declares it. The section *G5* gives the exceptions. |
| F1 | No program checks it. |
| F2 | The snake case. A person checks the verb. |
| F3 | The columns of each line. A tab moves the next character to the next multiple of 8 columns. |
| F4 | The lines between the line of the opening bracket and the line of the closing bracket. |
| F5 | `void` in a declaration with no argument, and the number of the arguments. A `...` is one argument. |
| F6 | Each comment that is not the legal notice. |
| L1 | Each line with more than one statement or assignment, an assignment in a condition, and a condition with its statement. |
| L2, L3, L4, L5, L6 | Each rule. |
| V1 | The case of each identifier, and `_t`. A person checks the English. |
| V2, A1, A2 | No program checks them. |
| V3 | The `*` of each pointer. |
| C1 | The branches of each chain of `if` and `else`, and the depth of the levels. |
| C2, C3 | Each rule. |
| H1 | The items of a header. A person checks the includes. |
| H2, H3 | Each rule. |

For C1, `if`, `while`, `for`, `switch` and `do` are levels, and an `else if` is not a new level.
For L1, a comma that joins two statements makes a line with more than one statement.

## G5

`style.py` does not report these functions:

- `main`,
- a function that a macro defines,
- a function whose declaration holds `dllexport` or `visibility`,
- a function whose declaration holds a word that ends with `EXPORT` or `_API`.

## The self-test

Before it reads the files, `style.py` checks itself:

1. For each rule that it checks, it reads a short text that breaks the rule. The rule must find a fault in the text.
2. It reads a short clean text. No rule but G1, H1 and H2 can find a fault in it.

If the self-test fails, `style.py` stops: `the checker itself: <faults>`.

## Exceptions

`[[style.exception]]` of `.toolchain/project.toml` gives the exceptions. `.toolchain/docs/project.toml.md`, *[[style.exception]]* gives its keys.
`style.py` reports each fault that no exception excuses, and each exception that excuses no fault.
An exception whose path matches no file excuses no fault too. Thus no exception stays after its reason or its file goes.
The line of an exception that excuses nothing is `.toolchain/project.toml:1: <rule> the exception for '<path>' excuses nothing`.

## Output

A fault is `<path>:<line>: <rule> <what>`.
The output follows `.toolchain/docs/shared.py.md`, *The report of a check*.

## Functions

| Function | Result |
| --- | --- |
| `Source(path, text)` | A file of C: its lines, its comments, its directives, and its code without comments, strings and character constants. |
| `tokens_of(code, start, end)` | Gives the tokens of a part of the code, with their positions. |
| `closing(code, at)` | Gives the position of the bracket that closes the bracket at `at`. |
| `opening(code, at)` | Gives the position of the bracket that opens the bracket at `at`. |
| `top_level(text, what)` | Tells if the text holds the character `what` out of all brackets. |
| `constant_in(text)` | Tells if a declaration declares a constant. |
| `parameters(text)` | Gives the number of the parameters of a list: -1 for an empty list, and 0 for `void`. |
| `Function(...)` | A function of a file: its name, its parameters, its brackets and the text before its name. |
| `file_scope(src)` | Gives the functions and the statements of the file scope. |
| `blank_lines(src, first, last)` | Gives the empty lines from `first` to `last`. |
| `check_lines(src)` | Checks each line: F3, L2, L3 and V3. |
| `check_file(src, functions)` | Checks the file: G1, F6, O3, G2, G3, V1, H1, H2 and H3. |
| `check_outside(src, statements, functions, declared)` | Checks the file scope: V1, H1, G4, F5 and G5. |
| `check_parameters(src, line, params, name)` | Checks the parameters of a function: F5. |
| `check_function(src, f)` | Checks the body of a function: F2, F4, F5, L1, L2, L4, L5, L6, V1, C1, C2 and C3. |
| `check(path, text, declared)` | Gives the faults of one file. `declared` holds the names that the headers of the project declare. |
| `exceptions()` | Gives the exceptions of `[[style.exception]]`. |
| `ours()` | Gives the files that `style.py` checks. |
| `prototypes()` | Gives the names that the headers of the project declare. |
| `self_test()` | Gives the faults of the self-test. |
| `main()` | Runs the self-test, checks the files, and writes the faults. |
