# Stage 12: one style, every platform, few dependencies

In progress. `PLAN.md`, stage 12, is what it is for and when it is done. This document is the
order it is done in, what is measured, and each decision as it is taken.

## What it replaced

Stage 12 was to be "self contained": the C library's routines and an allocator carried in the tree,
and no C runtime linked on Windows. It was planned, its sources were read and one experiment
was made, and then it was dropped for this. What the experiment found is in the record of
`build.md`, *The C library*, with the condition that would bring it back.

## The order

    1  the style         `STYLE.md`: the Epitech C Coding Style 3.1, made the project's own
    2  the stage         this document and `PLAN.md`
    3  the checker       `tools/style.py`, from scratch, run with the tests over the paths
                         that are clean, a list that only grows
    4  the decisions     what shapes files, before any is rewritten: one operating-system
                         layer; the module ABI at four arguments; the names; how storage is
                         cut with its helpers still inline; the test programs as tables of
                         cases; where a comment goes
    5  the refactor      a directory at a time, from what ships to what tests it, every
                         commit measured
    6  dependencies      the list, the case that holds it, and what can leave it
    7  platforms         a Windows workflow; Linux arm64, macOS x86-64, Windows arm64
    8  close             by the done-when

## What was decided before it began

    what the style covers     all C of ours, the tests and their modules included. Not Python
    its numbers               hard: a breach fails, and an exception is an entry of
                              `STYLE.md`, *Exceptions*, with its reason
    the seams                 not excepted: `_t` on the public types, four arguments an entry.
                              Excepting the module ABI's arguments was proposed and had no
                              reason: the entries over four run at load, and the ones a phase
                              calls take four or fewer already
    `f(void)`                 rule F5 wins over C23's `f()`, and `tools/c23.py` loses that rule
    the checker               ours, in Python. Epitech's own, banana, is a plug-in for its
                              compiler, built with CMake, and carries no licence: nothing of
                              it can be taken but the list of which rules a program can hold

## The tree against the style, before any of it

    F3  80 columns             written to about 100
    F4  20 lines a function    19 functions over 80 lines; the longest, a test's `main`, 742
    O3  5 functions a file     `storage.c` has 97; `abi.c` 35; `lifecycle.c` 29
    F5  4 arguments            `declare` 9, `system` 8, `observe` 7
    F6  no comment in a body   about 380 inside or beside code
    F2  a verb in the name     about 60 static functions named by a participle or a noun
    V1  `_t`                   no type has it
    L1  one statement a line   about 600 `if` with its statement on the same line
    L5  one declaration a      253 lines declare several
        line
    C3  no `goto`              20
    C1  3 branches             every test program is one chain on its argument, up to 28 arms
    G1  the header             no file has it

## The checker, as built

`tools/style.py` reads C as text, with no compiler: comments and literals are set aside, a
function is found by its braces, and its body is cut into statements. Before anything else it
checks itself, on C that infringes each rule and on C that infringes none.

    held          O1 to O4, G1 to G5, F2 to F6, L1 to L6, V1, V3, C1 to C3, H1 to H3
    not held      F1 and whether a file is coherent, the verb in a name, V2, A1, A2, and
                  whether a ternary is simple. A reader holds those
    the paths     the list `HELD` in the script, which is empty until the first directory is
                  made clean, and only grows. `--all` counts the whole tree instead
    as a case     `style`, beside `c23`

What the whole tree holds before any of it is rewritten, 95 files:

    F3  2656    L1   973    L3   861    F6   681    L5   647    L4   628
    L6   502    F5   242    C1   234    G3   159    G2   132    V1   117
    H1   102    G4    97    F4    95    G1    94    L2    80    O3    38
    C2    30    C3    20    O4     5    H3     5    G5     4    O2     1

Four readings the rules left open, each put to Enkrid and decided:

    a `case`            one level inside its `switch`, and what it runs one more. The tree
                        wrote it at the level of the `switch`
    `i ++`              infringes L3, as a space after any unary operator does. About 620
    a macro for text    infringes H3 as a macro for a number does. The five there are, such
                        as `#define BRIDGE_NAMESPACE "bridge"`, are removed with the
                        directory each is in, and what joined one to another literal is
                        rewritten
    wasmtime's library  no infringement of O1, which is reworded: nothing a build makes is
                        tracked, and a library somebody else published is tracked as
                        published, in a folder that holds its licence. The exception first
                        entered for it is withdrawn

## The decisions

What shapes files, each decided before any file is rewritten. What each refused is in this
stage's record of refusals, under the same heading.

### One operating-system layer

The same code was in three places: `src/os.c`, `backends/wasm/os.c` and
`backends/native/os.c`. A backend is a shared library of its own, so the linker finds
`os_mutex_lock` in each binary from that binary's own objects, and each had been given its own
source for it since stage 10, where a backend stopped compiling anything of `src/`. None of the
three reads a type of the library: they are calls to the operating system and nothing else.

**One source, in `utils/os/`, compiled into the library and into each backend.** Each binary
still holds its own machine code, and a call to it is the direct call it was. The rule of
stage 10 stands as it is written in `build.py`: a backend compiles nothing of `src/`. `utils/`
depends on nothing of ours, which is what lets both sides take it.

The folder is `utils/` and not `shared/`: `shared` already names the memory every module reads,
in `BRIDGE_SHARED` and throughout storage, and a word has one meaning here.

    fast          the same direct calls as before
    scalable      a new platform is written in one place, where it was three
    modular       the layer is a unit with no dependency, taken whole by whoever needs it
    extensible    a backend somebody else writes may take the folder, or write its own
    minimal       about 250 lines once, where it was about 490
    explicit      one place, named in `build.md`, *The C library*

### The three entries a module calls at load

`declare` took nine arguments, `system` eight and `observe` seven, and rule F5 allows four. All
three run when a module loads and never in a phase.

What a module passes today is already mostly not values. A name, a descriptor and the terms of
a query are offsets into the module's memory, which the library checks and reads; and the terms
are structs, `bridge_term`, whose layout `bridge-abi.h` asserts.

**Each of the three takes one struct, by its offset.** The module writes it in its memory and
passes where it is, as it passes its terms; the library checks the range and reads the fields.
The three layouts are stated in `bridge-abi.h` with their sizes and offsets asserted, as every
ABI layout is. A field a C initializer leaves out is zero, and zero is already the unset value
that `declare` refuses for a kind, for what happens at unload and for a storage layout.

    fast          at load only: one check and one read more
    scalable      a new field is a field, and no entry changes its arguments
    modular       the layouts are in `bridge-abi.h` with the others
    extensible    any language writes bytes at stated offsets
    minimal       three structs and three readers
    explicit      every field is named where it is written, and its place is asserted

### The three entries of the backend seam over four

`module_compile` took six arguments, `instance_create` five and `define_func` seven. All three
run at load.

**What states one thing is one struct, as for a module's entries.** What is compiled is a
`backend_source_t`: the bytes, their length, the path and what is demanded of the backend. What
is defined is a `backend_entry_t`: the namespace, the field, the function, what it is handed
and its signature. Each is passed by pointer, with where a failure is written.

**`instance_create` loses its backend.** It is handed the module, and a module is of one
backend: the wasm backend's module held it already, and the native backend never read the
argument. A backend that needs it keeps it in its module.

    fast          at load only, and the same
    scalable      a new field is a field
    modular       a module holds what it is of, as the wasm backend's did
    extensible    a third backend stores its own in its module, if it needs it
    minimal       two structs, and one argument fewer
    explicit      one way to the backend from an instance, where an unused second stood

### A line of the log

`bridge_log` took five arguments, and six in the native SDK: the level, the text, its length
and two numbers. It is a function of the SDK and no entry of the ABI: it writes a record in the
ring.

**What is logged is one struct, `bridge_line_t`**: the text, its length and the two numbers.
`bridge_log(level, line)`, and `(api, level, line)` in the native SDK. `BRIDGE_LOG(level, text,
a0, a1)` builds the line, so an author writes what was written before. The function is inline,
and the compiler sees the struct made and read in one place.

    fast          the same record written; the struct does not outlive the inlining
    scalable      a third number is a field
    modular       the same form in both SDKs
    extensible    not an entry: another language writes the record itself, as before
    minimal       one struct
    explicit      what a line holds is named, where it was the third to fifth arguments

### attach, which runs in a phase

The wasm entry took four arguments, `attach(id, slot, ptr, len)`, and the native one five, since
a native entry is handed its table first. `len` did two things: zero asked for a zeroed value,
and anything else had to be the declared size or the attach was refused. The library knows the
size, and `declare` already refuses a declarer whose size differs.

**`attach` loses `len`.** `attach(id, slot, ptr)`: the library reads the declared size at `ptr`,
checked against the scratch arena as before, and a zeroed value is asked for by name, with
`BRIDGE_ZEROED` as `ptr`. The SDK copies the value above what the scratch arena has in use and
does not count it as used, since the library takes its copy during the call: what it saved
before the call and put back after is no longer written.

Measured before it was decided, in a worktree, on the native path, three runs of each tree in
turn, in ns:

                                            before          without len
    attach, first component: the call       9.5 to 10.2     9.6 to 10.3
    attach, first component: the barrier    26.6 to 27.6    26.3 to 26.5
    attach, a second: the barrier           29.6 to 30.6    29.1 to 29.2
    attach of 4 bytes, the whole            30.3 to 30.4    29.3 to 29.9
    a sparse set given: the call            10.4 to 10.7    10.3 to 10.5
    direct-indexed given: the call          10.4 to 10.7    10.0 to 10.6
    detach: the call                        7.7 to 8.1      8.4 to 8.8

It buys the rule and one refusal fewer, and no speed: the two stores saved do not show. The
call of `detach` was slower in all six runs though nothing of it was changed, which points at
where its code was placed and was not confirmed. The commit that builds this explains that line
or undoes it.

    fast          the same on attach; detach to be held at the commit
    scalable      the same
    modular       a size has one owner, the declaration
    extensible    another language passes three values
    minimal       code is removed on both sides
    explicit      a zeroed value is asked for by a name, where it was a length of zero

### Names

Three rules ask for renames, and each got one scheme.

**The verb, rule F2.** Of 323 functions in the library, the backends and the SDKs, about sixty
were a participle (`moved`, `killed`, `carried`), an adjective (`alive`, `empty`) or a noun
(`set_of`, `value_of`, `fingerprint`). **A fixed set, everywhere**: `is_` or `has_` for what
answers yes or no, `get_` for what answers something held and changes nothing, and the verb of
what it does for everything else. The seams follow, keeping what they belong to first:
`bridge_get_rows`, `os_get_now`, `storage_is_alive`. It is written in `STYLE.md`, rule F2, since
a reader holds it and no program can.

    fast          nothing
    scalable      a new function has its form before it has its name
    modular       one form in the library, the backends and the SDKs
    extensible    a host's author and a module's learn one form
    minimal       about sixty renames inside and about twenty-five at the seams
    explicit      the first word says whether the call reads or changes

**`_t`, rule V1.** None of the 65 typedefs had it. **The tag keeps the bare name and the typedef
adds `_t`**: `typedef struct module module_t;`. One stem and one suffix.

**Files, rule O4.** The five headers with a hyphen take an underscore: `bridge_abi.h`,
`bridge_backend.h`, `bridge_schema.h`, `bridge_native.h`, `bridge_wasm.h`. **What is built
follows**, `bridge_wasm.dll` and `bridge_native.dll`, so that a backend is spelled one way in
its header, its library and a host's configuration.
