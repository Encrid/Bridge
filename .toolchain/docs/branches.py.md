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

1. `rewrite.each` gives the syntax tree of each file `.c` of `src` that the build compiles: `.toolchain/docs/rewrite.py.md`, *The steps*.
2. In each syntax tree, `walked` finds the conditions, the labels and the functions of `src` and `include`.
3. `rewrite.found` finds the functions of `listed` of `fail.py` that the file uses, and `fail.layer` gives the layer.
4. `written` copies `src` and `include` into `<build>/branches/tree`, and writes the probes and the layers into the copies.

A header gets the probes of each tree that includes it.
`test.py` then builds the copies, runs the cases and calls `reported`: `.toolchain/docs/test.py.md`, *The branches*.

## The probes

- A condition `c` becomes `c23b_((c), <number>)`. `c23b_` records if `c` was true or false, and gives `c`.
- After the `:` of a label and after the `{` of a function, `c23b_hit_(<number>);` records that it ran.
- A file `.c` starts with an array of `_Atomic` bytes, one for each outcome, a pointer to the bytes, and these two functions.
- At its end, a file `.c` has a `[[gnu::constructor]]` that maps its part of the file of the hits: *The hits*.
- A file `.c` also gets its layer: `.toolchain/docs/fail.py.md`, *The layer*. Thus a case that states `fail` reaches the branch of the failed call.
- A header starts with one block, under a guard of its own, with the same parts under names of its own.

The probes come before the first line of the file, and `#line 1` keeps the numbers of the lines.
A file `.c` includes no header before its own lines. Thus its own macros, such as `_POSIX_C_SOURCE`, come first.

## The hits

The file of the hits holds one byte for each outcome of each file, in the order of the paths. `parts` gives the part of each file.
`test.py` writes the file with a byte 0 for each outcome, and the variable `C23_BRANCHES` names it.

The constructor of a file maps the part of the file into memory, with `mmap` on Linux and on macOS, and with `MapViewOfFile` on Windows.
It copies the bytes that the probes set before it ran, and then the pointer of the probes points to the mapped part.
A probe sets the byte of its outcome to 1 only the first time that the outcome is reached. After that, the probe only reads the byte.

The system keeps the bytes of a mapped file when the program ends. Thus an outcome stays, also when the program ends with `_Exit` or `abort`.
Each program, and each library that a program opens, maps the same file. Thus the file holds the outcomes of all the programs of the cases.
On Windows, the constructor declares `CreateFileMappingA` and `MapViewOfFile` itself, so `windows.h` does not come into the file.

## Macros

- An end of a condition in a macro with no arguments, such as `UINT32_MAX`, gets its probe after the name of the macro.
- A condition that starts with a call of a macro, such as `ckd_add(...)`, gets its probe around the full call.
- A condition in an argument or in the body of a macro cannot get a probe. The report writes it as not measured.

## The report

`reported` reads the file of the hits. An outcome is reached if its byte is 1.
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

## Exemptions

`[[branches.exception]]` of `project.toml` marks a defensive branch that stays though no test drives it.
Each exemption names a branch by `path`, `line`, `column` and `side`, and states a `why`.
The `side` is `true`, `false`, `function` or `case`.
An exemption can also name a `system`. It applies, and is checked, only on that system, so a platform-only branch gets a platform-only exemption.

`reported` excuses an unreached outcome that an exemption matches, and counts it as exempt, not as missed.
An exemption with no `why` excuses nothing and is a fault.
An exemption that matches no unreached outcome is a fault, so a branch that a case reaches again loses its exemption.
`.toolchain/docs/project.toml.md`, *[[branches.exception]]*.

## Limits

- A probe that runs before the constructor of its file keeps its outcome only if the constructor runs later.
- The measure uses only the target of this machine. The files of a different system, for example `src/posix` on Windows, are measured on a machine of that system.
- A header that two artifacts compile with different macros gets the probes of both.

## Functions

| Function | Result |
| --- | --- |
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
| `walked(root, path, tree)` | Gives the probes of the syntax tree of one file and of its headers. |
| `rewritten(text, path, edits, prefix, part, outer)` | Gives a file with its probes and its layer `outer`, and the line and column of each probe. `part` gives the first byte and the number of bytes of the file in the file of the hits. |
| `probed(config, values)` | Writes the copies with the probes and the layers, as *The steps* tells. It gives the probes and the branches that are not measured. |
| `written(root, where, edits, skips, outers)` | Copies `src` and `include` with `rewrite.copied`, and writes the probes and the layer of each file. `outers` gives the layer of each file. |
| `parts(rows)` | Gives the first byte and the number of bytes of each file in the file of the hits, and the size of that file. `rows` gives the probes of each file. |
| `joined(probes, hits)` | Gives the bits of each file, from its part of the file `hits`. |
| `excused(listed, system, name, line, col, said)` | Tells if an exemption of `listed` matches this unreached outcome on this system, and counts the match. |
| `reported(probes, gaps, hits, excepted, system)` | Writes the report, as *The report* tells. `excepted` holds the exemptions, and `system` is the system of the build. |
