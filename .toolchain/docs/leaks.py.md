# .toolchain/tools/leaks.py

`leaks.py` counts the resources that the code takes and gives back, for `test --leaks`.
Each case must give back each resource that the code of `src` takes: `docs/contributing/tests.md`, *The leaks*.

## What it counts

Each resource has a kind. Functions of the code take a resource of a kind, and functions give it back.

- The kind `memory`: `malloc` and `calloc` take it, `free` gives it back, and `realloc` changes a block of it.
- Each function of `[leaks] memory` also takes memory, for example a function of a different project whose result the code frees.
- Each pair of `[leaks.pairs]` is a kind, with the name of the function that takes it.
- Each table `[leaks.kinds.<kind>]` is a kind. Its `takes` and `gives` name the functions that take and give back a resource of the kind, each at a place: *The places*.
- `[leaks] with` names more files, which add `memory`, `pairs` and `kinds`, for example a file of a package.

`.toolchain/docs/project.toml.md`, *[leaks]*, gives the keys.
A function that takes a resource of a pair gives a pointer. The function that gives it back gets that pointer as its first argument.

A take adds 1 when its value is not zero, and a give subtracts 1 when its value is not zero.
A value is not zero when one of its bytes is not zero. Examples are a pointer that is not null, a number that is not 0, and a handle with a byte that is not 0.
The value of a take of `memory` or of a pair is its result, and the value of a give is its first argument.
`realloc` adds 1 when its pointer is null and its result is not null.
`realloc` with 0 bytes is a fault, because C23 does not define it.

## The places

A function of a kind takes or gives back a resource at a place:

| Place | Value |
| --- | --- |
| `result` | The result of the function. |
| `<n>` | The argument `n`, counted from 1. |
| `*<n>` | The value that the argument `n` points to. The count reads it only when the argument is not null. |

- A take is at `result` or at `*<n>`. It counts after the call.
- A give at `<n>` or at `*<n>` reads its value before the call.
- A give at `result` is a function of `src` that hands its result to a library, for example a callback that a library calls. It counts after the call.
- A function can have several places in a kind, in a list, and it can be in several kinds.

When a call can leave the value at `*<n>` as it was, the code must clear that value before the call.
The count reads the value only after the call, because before the call the value can have no defined content.

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
A give at `result` that the file defines gets its block after the end of its definition.
Thus each later use of its name, also a use as a pointer, counts the results that it hands to the library.

At the end of the file, `#pragma pop_macro` gives each name its first meaning again. Then the file has these parts:

- an array of `_Atomic long long`, with a count for each kind and a count of the calls of `realloc` with 0 bytes;
- for each counted function, a `static` function that counts and calls it, with the parameters of its declaration and with each `typedef` replaced. It reads the values of the gives before the call, and the values of the takes and of the results after the call;
- the writer of `rewrite.py`: `.toolchain/docs/rewrite.py.md`, *The writer*.

Each program writes one line `<path> <counts>` for each file into the file of the variable `C23_LEAKS`.
The line has a count for each kind, in the order of `listed`, and then the count of the calls of `realloc` with 0 bytes.

`counted` stops if a counted function does not fit its kind:

- `<name> takes a resource, and does not give a pointer`
- `<name> gives back a resource, and does not get a pointer first`
- `<name> takes a list of arguments of no fixed length`
- `<name> gives no result`
- `<name> has no argument <n>`
- `<name> does not get a pointer as argument <n>`
- `[leaks] with: <file> is not there`
- `[leaks.kinds.<kind>]: <name> cannot take at <place>`, or `cannot give back at <place>`
- `[leaks.kinds.<kind>]: a kind has no key <key>`

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
- A resource whose value of failure is not 0, for example -1, does not fit a kind.
- A resource that moves by an assignment, for example into an element of an array, is not counted.
- A function that gives back many resources at once is not counted. Give each resource back, or write an exemption.
- A give at `result` counts only the uses of its name after its definition in its file. A call of it by the code also counts as a give.
- A struct whose fields are 0 but whose padding is not 0 counts as a resource. The case then keeps a resource that it does not hold.

## Functions

| Function | Result |
| --- | --- |
| `listed(config, values)` | Gives the effects of each counted function, and the list of the kinds. An effect is a role, a kind, a place, and whether the function must fit a pointer. It reads the files of `[leaks] with`. |
| `kinded(names, kind, acts)` | Adds the effects of one table `[leaks.kinds.<kind>]` to `names`. It stops on a key or a place that does not fit. |
| `plain(kind)` | Gives a type of the syntax tree with each `typedef` replaced, if the dump gives it. A header can use a name that the end of the file does not know. |
| `declaration(node)` | Gives the type of the result and the types of the parameters of the declaration of a function. |
| `found(root, path, tree, names)` | Gives the counted functions that a file uses and does not define, and the declaration of each of them. It also gives the end of the definition of each give at `result` that the file uses. |
| `directive(line)` | Gives the name of the directive of a line, an empty text for a blank line, or `None` for a line of code. |
| `included(text)` | Gives the byte after the last `#include` of a file, or after the `#endif` of the `#if` that holds it. |
| `fitted(name, effects, returned, parameters)` | Stops if a place of a function does not fit its declaration. |
| `tested(effect, kinds)` | Gives the line that counts one effect, when its value is not zero. |
| `wrapped(name, effects, kinds, declaration)` | Gives the `static` function that counts the calls of a function. |
| `rewritten(text, path, used, ends, declared, names, kinds)` | Gives a file with its count. |
| `counted(config, values)` | Writes the copies with the counts, as *The steps* tells. It gives the files that count, and the kinds. |
| `added(path, files, slots)` | Gives the counts of one case, added kind by kind from the lines of its file. |
| `excused(listed, system, case, kind)` | Tells if `listed` holds an exemption for this kept resource on this system, and counts the match. |
| `reported(files, kinds, cases, excepted, system)` | Writes the report, as *The report* tells. `cases` gives the file of the lines of each case. |
