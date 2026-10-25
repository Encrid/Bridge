# .toolchain/tools/branches.py

`branches.py` measures the branches of the code that the cases run, for `test --branches`.
Each branch of the code must run in a case: `docs/contributing/tests.md`, *The branches*.

## What it measures

In each function of `src` and of `include`, in the files `.c` and `.h`, it finds:

- each condition that no case makes true, and each condition that no case makes false;
- each `case` and `default` of a `switch` that no case reaches;
- each function that no case calls.

A condition is the test of an `if`, a `while`, a `do`, a `for` or a `?:`, or an operand of `&&` or `||`.
A test that holds `&&` or `||` counts by its parts: `a && b` holds the two conditions `a` and `b`.
The parentheses, the casts and `!` do not change a condition.
The measure does not probe a constant part: `static_assert`, `sizeof`, `_Generic`, a `case` label, an `enum`, and the value of a `static` or `constexpr` variable.

## The steps

`probed` does these steps:

1. It finds each artifact that the build makes for this target.
2. It takes each file `.c` of `src` that such an artifact compiles, with the flags of the first such artifact.
3. It runs the command `[branches] dump` of `toolchain.toml` for each file, 8 files at the same time.
4. In each syntax tree, it finds the conditions, the labels and the functions of `src` and `include`.
5. It copies `src` and `include` into `<build>/branches/tree`, and writes the probes into the copies.

A header gets the probes of each tree that includes it.
`test.py` then builds the copies, runs the cases and calls `reported`: `.toolchain/docs/test.py.md`, *The branches*.

## The probes

- A condition `c` becomes `c23b_((c), <number>)`. `c23b_` records if `c` was true or false, and gives `c`.
- After the `:` of a label and after the `{` of a function, `c23b_hit_(<number>);` records that it ran.
- A file `.c` starts with an array of `_Atomic` bytes, one for each outcome, and these two functions.
- At its end, a file `.c` writes with the low-level file calls of the system, with no buffer of `stdio`.
- A header starts with one block, under a guard of its own, with the same parts under names of its own.

Each program writes one line `<path> <bits>` for each file, to the file that the variable `C23_BRANCHES` names.
A line has one character, `0` or `1`, for each outcome.
A `[[gnu::destructor]]` of each file writes its line, at the exit of the program and at the close of a library.
The writer builds each line in a buffer and writes it with one append, so parallel programs do not interleave.

On macOS, a `[[gnu::constructor]]` of each file opens its own library again, with `RTLD_NOLOAD` and `RTLD_NODELETE`.
Thus a closed library stays mapped, and it does not matter which code opens and closes it, for example a released library.
Then its destructor runs at the exit of the program, with the code of the library still in place.
The constructor finds its library with `dladdr`. It declares `dladdr` and `dlopen` with names of its own, so it includes no header.

The probes come before the first line of the file, and `#line 1` keeps the numbers of the lines.
A file `.c` includes no header before its own lines. Thus its own macros, such as `_POSIX_C_SOURCE`, come first.

## Macros

- An end of a condition in a macro with no arguments, such as `UINT32_MAX`, gets its probe after the name of the macro.
- A condition that starts with a call of a macro, such as `ckd_add(...)`, gets its probe around the full call.
- A condition in an argument or in the body of a macro cannot get a probe. The report writes it as not measured.

## The report

`reported` joins the lines of all the programs. An outcome is reached if one line or more has its bit at `1`.
Then it writes these lines, in the order of the files, the lines and the columns:

- `<path>:<line>:<column>: no case makes this condition true`
- `<path>:<line>:<column>: no case makes this condition false`
- `<path>:<line>:<column>: no case reaches this case`
- `<path>:<line>:<column>: no case calls this function`
- `<path>:<line>:<column>: this condition is in a macro, and is not measured`, or `this case`, or `this function`
- `<path>:<line>:<column>: this exemption states no reason`
- `<path>:<line>:<column>: this exemption excuses no unreached outcome`

Then it writes `branches: <outcomes> outcomes, <reached> reached`, and `, <exempt> exempt` when an exemption excuses an outcome.
If an outcome is not reached, or a branch is not measured, it stops: `outcomes not reached: <number>; branches not measured: <number>`.
If a line has a different number of bits than the probes of its file, it stops: `<hits>: a line of <path> does not match its probes`.

## Exemptions

`[[branches.exception]]` of `project.toml` marks a defensive branch that stays though no test drives it.
Each exemption names a branch by `path`, `line`, `column` and `side`, and states a `why`.
The `side` is `true`, `false`, `function` or `case`.
An exemption can also name a `system`. It applies, and is checked, only on that system, so a platform-only branch gets a platform-only exemption.

`reported` excuses an unreached outcome that an exemption matches, and counts it as exempt, not as missed.
An exemption with no `why` excuses nothing and is a fault.
An exemption that matches no unreached outcome is a fault, so a branch that a case reaches again loses its exemption.
`.toolchain/docs/project.toml.md`, *[[branches.exception]]*.

## The syntax tree

`dump` runs `zig cc` with `-fsyntax-only`, and with the other flags of the artifact.
zig then writes an error ending with `FileNotFound`, because no object is made. `dumped` ignores only such an error.
If clang writes a different error, `dumped` stops: `the tree of <path> cannot be read:`, and the errors.
If the output is not JSON, it stops: `the tree of <path> is not JSON`.

The dump sets `_FORTIFY_SOURCE` to 0. Then a condition in a call that the system would fortify, such as `snprintf`, stays measurable and does not read as a macro.

## Limits

- A case that crashes, or that stops before its end, writes no line. Its outcomes count as not reached.
- The measure uses only the target of this machine. The files of a different system, for example `src/posix` on Windows, are measured on a machine of that system.
- Every file writes with a `[[gnu::destructor]]`. A destructor of a library that `dlclose` unmaps on macOS would run at the exit of the program with its code gone. So on macOS each file keeps its own library mapped, and the destructor runs in place.
- A header that two artifacts compile with different macros gets the probes of both.

## Functions

| Function | Result |
| --- | --- |
| `tree(values)` | Gives the folder of the copies with the probes, `<build>/branches/tree`. |
| `marked(node, state)` | Gives each location of a syntax tree the file that it is in. The dump writes a file only when it changes. |
| `is_macro(loc)` | Tells if a location comes from a macro. |
| `owned(loc, root)` | Gives the path of a location in `src` or in `include`, or `None`. |
| `placed(loc, end, root)` | Gives the byte of a location in its file, and the path. With `end`, the byte after the last character. It gives `macro` for an argument of a macro. |
| `stripped(node)` | Gives the node in parentheses, a cast and `!`. |
| `is_logical(node)` | Tells if a node is `&&` or `||`. |
| `child(raw, index)` | Gives a child of a node, or `None`. |
| `Probing` | Finds the probes of one syntax tree. |
| `Probing.walk(node, inside, depth)` | Walks the tree. `inside` tells that the node is in the body of a function. |
| `Probing.function(node, raw, depth)` | Adds the probe of a function, and walks its body. |
| `Probing.label(node, raw)` | Adds the probe of a `case` or a `default`. |
| `Probing.condition(node, depth)` | Adds the probe of a condition, or of each part of a test with `&&` or `||`. |
| `Probing.called(path, at)` | Gives the end of the call of a macro whose name ends at `at`, or `at` if no parenthesis follows. |
| `Probing.add(path, key, edit)` | Keeps one probe of a file. |
| `Probing.skip(loc, kind)` | Keeps a branch that cannot get a probe. |
| `dumped(command, root)` | Runs the command `dump`, and gives the syntax tree. |
| `walked(root, command)` | Gives the probes of one file and of its headers. |
| `placing(text, at)` | Gives the line and the column of a byte. |
| `rewritten(text, path, edits, prefix)` | Gives a file with its probes, and the line and column of each probe. |
| `probed(config, values)` | Writes the copies with the probes, as *The steps* tells. It gives the probes and the branches that are not measured. |
| `written(root, where, edits, skips)` | Copies `src` and `include`, and writes the probes of each file. |
| `joined(probes, hits)` | Gives the bits of each file, joined from all the lines of `hits`. |
| `excused(listed, system, name, line, col, said)` | Tells if an exemption of `listed` matches this unreached outcome on this system, and counts the match. |
| `reported(probes, gaps, hits, excepted, system)` | Writes the report, as *The report* tells. `excepted` holds the exemptions, and `system` is the system of the build. |
