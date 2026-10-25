# .toolchain/tools/leaks.py

`leaks.py` counts the resources that the code takes and gives back, for `test --leaks`.
Each case must give back each resource that the code of `src` takes: `docs/contributing/tests.md`, *The leaks*.

## What it counts

Each resource has a kind. Functions of the code take a resource of a kind, and functions give it back.

- The kind `memory`: `malloc` and `calloc` take it, `free` gives it back, and `realloc` changes a block of it.
- Each function of `[leaks] memory` also takes memory, for example a function of a different project whose result the code frees.
- Each pair of `[leaks.pairs]` is a kind, with the name of the function that takes it.
- `[leaks] with` names files that give more `memory` and `pairs`, for example a file of a package.

`.toolchain/docs/project.toml.md`, *[leaks]*, gives the keys.
A function that takes a resource of a pair gives a pointer. The function that gives it back gets that pointer as its first argument.

A take adds 1 when its result is not null. A give subtracts 1 when its first argument is not null.
`realloc` adds 1 when its pointer is null and its result is not null.
`realloc` with 0 bytes is a fault, because C23 does not define it.

## The steps

`counted` does these steps:

1. `listed` reads the counted functions and their kinds.
2. `rewrite.each` gives a syntax tree for each compiled file `.c` of `src`: `.toolchain/docs/rewrite.py.md`.
3. In each tree, `found` finds the counted functions that the file uses and does not define, and their declarations.
4. `rewrite.copied` copies `src` and `include` into `<build>/leaks/tree`.
5. `rewritten` writes the count into each copy of a file that uses a counted function.

Then `test.py` builds the copies and runs the cases, and `reported` writes the report: `.toolchain/docs/test.py.md`, *The leaks*.

## The count in a file

After the last `#include` of a file, a block renames each counted function that the file uses. For `free`, the block is:

```c
[[maybe_unused]] static typeof(free) c23b_l_free;
#pragma push_macro("free")
#undef free
#define free c23b_l_free
```

Each later use of the name calls the function that counts, also a use of the name as a pointer.
No header sees the new name, because the headers come before the block.
If the last `#include` is in an `#if` that holds only lines of the preprocessor, the block comes after the `#endif` of that `#if`.
A `#line` after the block keeps the numbers of the lines.

At the end of the file, `#pragma pop_macro` gives each name its first meaning again. Then the file has these parts:

- an array of `_Atomic long long`, with a count for each kind and a count of the calls of `realloc` with 0 bytes;
- for each counted function, a `static` function that counts and calls it, with the parameters of its declaration and with each `typedef` replaced;
- the writer of `rewrite.py`: `.toolchain/docs/rewrite.py.md`, *The writer*.

Each program writes one line `<path> <counts>` for each file into the file of the variable `C23_LEAKS`.
The line has a count for each kind, in the order of `listed`, and then the count of the calls of `realloc` with 0 bytes.

`counted` stops if a counted function does not fit its kind:

- `<name> takes a resource, and does not give a pointer`
- `<name> gives back a resource, and does not get a pointer first`
- `<name> takes a list of arguments of no fixed length`
- `[leaks] with: <file> is not there`

## The report

`reported` adds the lines of each case, kind by kind. It writes one line for each fault, case after case:

- `leaks: <case> keeps <number> <kind>`
- `leaks: <case> gives back <number> <kind> that it did not take`
- `leaks: <case> calls realloc with 0 bytes`
- `leaks: the exemption of <case> for <kind> states no reason`
- `leaks: the exemption of <case> for <kind> excuses no kept resource`

Then it writes `leaks: <cases> cases, <balanced> give back what they take`, and `, <exempt> exempt` when an exemption excuses a case.
If a case does not give back what it takes, or an exemption is a fault, it stops.
The message is `cases that do not give back what they take: <number>; exemptions that are faults: <number>`.
A line whose counts do not fit the kinds stops the report: `<file>: a line of <path> does not match its counts`.

## Exemptions

`[[leaks.exception]]` of `project.toml` marks a resource that a case keeps by design.
Each exemption names a `case` and a `kind`, and states a `why`. The `case` can contain `*`.
An exemption can also name a `system`. Then the exemption is used and checked on that system alone.

`reported` excuses a kept resource that an exemption matches. A case whose kept resources are all excused counts as exempt.
An exemption without a `why` is a fault, and excuses no resource.
An exemption that excuses no kept resource is a fault, so a case that gives the resource back again loses its exemption.
`.toolchain/docs/project.toml.md`, *[[leaks.exception]]*.

## Limits

- A program that does not end normally writes no line, so its counts are lost: `.toolchain/docs/rewrite.py.md`, *Limits*.
- Code before the last `#include` of a file is not counted.
- A function of a header that calls a counted function is not counted.
- The code of `tests`, of a different project and of a third party is not counted. If it frees memory that counted code took, the case keeps it.
- If code that is not counted takes memory that counted code frees, the case gives back memory that it did not take. A name in `[leaks] memory` counts that take.
- A thread that runs after the exit of the program can change a count after its file wrote its line.

## Functions

| Function | Result |
| --- | --- |
| `listed(config, values)` | Gives the role and the kind of each counted function, and the list of the kinds. It reads the files of `[leaks] with`. |
| `plain(kind)` | Gives a type of the syntax tree with each `typedef` replaced, if the dump gives it. A header can use a name that the end of the file does not know. |
| `declaration(node)` | Gives the type of the result and the types of the parameters of the declaration of a function. |
| `found(root, path, tree, names)` | Gives the counted functions that a file uses and does not define, and the declaration of each of them. |
| `directive(line)` | Gives the name of the directive of a line, an empty text for a blank line, or `None` for a line of code. |
| `included(text)` | Gives the byte after the last `#include` of a file, or after the `#endif` of the `#if` that holds it. |
| `wrapped(name, role, kinds, declaration)` | Gives the `static` function that counts the calls of a function. |
| `rewritten(text, path, used, declared, names, kinds)` | Gives a file with its count. |
| `counted(config, values)` | Writes the copies with the counts, as *The steps* tells. It gives the files that count, and the kinds. |
| `added(path, files, slots)` | Gives the counts of one case, added kind by kind from the lines of its file. |
| `excused(listed, system, case, kind)` | Tells if `listed` holds an exemption for this kept resource on this system, and counts the match. |
| `reported(files, kinds, cases, excepted, system)` | Writes the report, as *The report* tells. `cases` gives the file of the lines of each case. |
