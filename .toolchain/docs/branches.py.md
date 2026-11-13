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
4. With `--part`, `mine` keeps only the probes and the gaps of the files of the named parts.
5. `written` copies `src` and `include` into `<build>/branches/tree`, and writes the probes and the layers into the copies.

A header gets the probes of each tree that includes it.
`test.py` then builds the copies, runs the cases and calls `reported`: `.toolchain/docs/test.py.md`, *The branches*.

## The files of the named parts

With `--part`, a file gets probes only if it belongs to a named part. `part_of` of `shared.py` gives the part of the file, from its path after `src/` or `include/`.
A file of a different part, or of no part, compiles with no probes. Its own part measures it, and so does a run with no `--part`.
Thus a part that compiles a source or includes a header of a different part does not get the outcomes of that code.

A source with no probes still gets its layer of fail, so a case with `fail` acts as in a run with no `--part`.
The report then starts with `branches: measured parts <part>, <part>`.

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
- If a condition starts or ends in a call of a macro, its probe starts or ends at the edge of the full call. This is also true for an argument, such as `y` in `x == SAME(y)`.
- If a condition holds only a part of a call, it cannot get a probe. An example is `x == y` in `x == PICK(y) + 1`, with `#define PICK(c) c ? 0 : 1`. The report writes it as not measured, at the name of the call.

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
- A probe holds the full text of each call that it touches. Thus an operator at an edge of a body, such as `&&` in `#define AND(a) a &&`, breaks the build of the copy. A parenthesis in a string or in a character constant of an argument does the same.

## Functions

| Function | Result |
| --- | --- |
| `placed(loc, end, root)` | Gives the byte of a location in its file, and the path. With `end`, the byte after the last character. It gives `macro` for an argument of a macro. |
| `stripped(node)` | Gives the node in parentheses, a cast and `!`. |
| `is_logical(node)` | Tells if a node is `&&` or `||`. |
| `child(raw, index)` | Gives a child of a node, or `None`. |
| `traced(top)` | Gives the parent of each node under `top`. For each call of a macro, it gives the nodes with an end in the call, and the tokens of those ends. |
| `Probing` | Finds the probes of one syntax tree. |
| `Probing.walk(node, inside, depth)` | Walks the tree. `inside` tells that the node is in the body of a function. |
| `Probing.function(node, raw, depth)` | Adds the probe of a function, traces its body with `traced`, and walks it. |
| `Probing.label(node, raw)` | Adds the probe of a `case` or a `default`. |
| `Probing.condition(node, depth)` | Adds the probe of a condition, or of each part of a test with `&&` or `||`. A condition that holds only a part of a call is not measured. |
| `Probing.edge(loc, end)` | Gives the byte and the path of an end of a condition, as `placed`. An end in an argument of a macro goes to the name of the call. |
| `Probing.partial(node)` | Tells if a condition leaves out a node of a call that it touches. A node that holds the condition does not count if its tokens in the call are all in the condition. |
| `Probing.called(path, at)` | Gives the end of the call of a macro whose name ends at `at`, or `at` if no parenthesis follows. |
| `Probing.add(path, key, edit)` | Keeps one probe of a file. |
| `Probing.skip(loc, kind)` | Keeps a branch that cannot get a probe. |
| `walked(root, path, tree)` | Gives the probes of the syntax tree of one file and of its headers. |
| `rewritten(text, path, edits, prefix, part, outer)` | Gives a file with its probes and its layer `outer`, and the line and column of each probe. `part` gives the first byte and the number of bytes of the file in the file of the hits. |
| `mine(config, path, named)` | Tells if a file gets probes: each file with no `named`, else only a file of a part of `named`. |
| `probed(config, values, named)` | Writes the copies with the probes and the layers, as *The steps* tells. It gives the probes and the branches that are not measured. `named` holds the parts of `--part`. |
| `written(root, where, edits, skips, outers)` | Copies `src` and `include` with `rewrite.copied`, and writes the probes and the layer of each file. `outers` gives the layer of each file. A source with a layer and no probes gets its layer with `wrapped` of `fail.py`. |
| `parts(rows)` | Gives the first byte and the number of bytes of each file in the file of the hits, and the size of that file. `rows` gives the probes of each file. |
| `joined(probes, hits)` | Gives the bits of each file, from its part of the file `hits`. |
| `excused(listed, system, name, line, col, said)` | Tells if an exemption of `listed` matches this unreached outcome on this system, and counts the match. |
| `reported(probes, gaps, hits, excepted, system, named)` | Writes the report, as *The report* tells. `excepted` holds the exemptions, and `system` is the system of the build. With `named`, it first names the measured parts. |
