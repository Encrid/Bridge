# Stage 12: one style, every platform, few dependencies

In progress. `plan.md`, stage 12, is what it is for and when it is done. This document is the
order it is done in, what is measured, and each decision as it is taken.

## What it replaced

Stage 12 was to be "self contained": the C library's routines and an allocator carried in the tree,
and no C runtime linked on Windows. It was planned, its sources were read and one experiment
was made, and then it was dropped for this. What the experiment found is in the record of
`build.md`, *The C library*, with the condition that would bring it back.

## The order

    1  the style         `style.md`: the Epitech C Coding Style 3.1, made the project's own
    2  the stage         this document and `plan.md`
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
                              `style.md`, *Exceptions*, with its reason
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
are structs, `bridge_term`, whose layout `bridge_abi.h` asserts.

**Each of the three takes one struct, by its offset.** The module writes it in its memory and
passes where it is, as it passes its terms; the library checks the range and reads the fields.
The three layouts are stated in `bridge_abi.h` with their sizes and offsets asserted, as every
ABI layout is. A field a C initializer leaves out is zero, and zero is already the unset value
that `declare` refuses for a kind, for what happens at unload and for a storage layout.

    fast          at load only: one check and one read more
    scalable      a new field is a field, and no entry changes its arguments
    modular       the layouts are in `bridge_abi.h` with the others
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
`bridge_get_rows`, `os_get_now`, `storage_is_alive`. It is written in `style.md`, rule F2, since
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

### How a file of many functions is cut

`storage.c` had 97 functions, `abi.c` 35 and `lifecycle.c` 29, and rule O3 allows five a file.

The compiler puts a `static` function inside its caller only when both are in one translation
unit. `storage.c` was one: nine helpers are forced inline by an attribute, and the rest are
inlined as the compiler judges. Cut into `.c` files, a call from one to another is a real call.

Measured, with `storage.c`, `abi.c` and `block.c` compiled so that only the nine forced helpers
are inline, which is the worst a cut can do. Three runs of each, in ns:

                                        before          only the nine inline
    attach, the first component         26.6 to 27.7    37.3 to 37.8
    attach, a component it has          12.1 to 12.3    18.6 to 18.9
    attach, a second component          29.6 to 30.4    48.0 to 48.7
    detach                              27.5 to 28.2    42.3 to 44.2
    destroy                             14.9 to 15.7    20.5 to 21.3
    a sparse set given                  27.4 to 29.3    44.6 to 46.6
    direct-indexed given                17.5 to 18.0    32.7 to 33.4
    the stress run, 4 threads, ms       1457 to 1546    1767 to 1896

The nine are not enough: most of what is inlined, the compiler chose.

**What runs once for a change or for a row is `static inline` in a header, five a header, and
what runs once for a barrier or more rarely is in a `.c`.** Rule H1 allows a header exactly
that. The `.c` that holds a barrier's loop includes those headers, so the compiler has the same
code in one unit as before. Every commit of the cut is measured beside the tree before the
stage, and a slower line stops it.

    fast          the unit the compiler sees is the one it saw; held at every commit
    scalable      a new function has its place by how often it runs
    modular       every file compiles alone
    extensible    a test or a backend can include one header of it
    minimal       the build does not change
    explicit      where a function is says how often it runs

### The test programs

Six programs each had one `main` that was a chain of `if` and `else if` on its argument, of 11
to 73 arms and 91 to 742 lines. The arms shared what `main` had built, `sink`, `load`, `frames`
and the two that describe an artifact were written again in four to six programs, and what a
sink counted was in variables of the file, which rule G4 does not allow.

**A case is a function, and a program is a table of them.** `main` builds a fixture, finds the
name in a constant table and calls. The fixture holds what `main` built and what the sink
counts, and it is what the sink is handed. **A program is a directory**, `tests/stage5/`, with
its cases five a file, and what the programs share is `tests/support/`, compiled once. The
cases `build.py` lists and every transcript stay as they are.

    fast          the same
    scalable      a new case is a function and a row
    modular       a stage's cases stay together, and what is shared is in one place
    extensible    a new program is a directory
    minimal       nothing of `build.py`'s cases changes
    explicit      the table says every case a program has

### Comments

468 comments were inside a function or beside a line of it, which rule F6 does not allow, and
215 above a function were not in the form the rule gives. 136 cited a document, and nothing
checked them: `tools/xref.py` read the documents only.

What becomes of a comment inside a function is not a choice. One that names a step: rule F4
makes the step a function, and the comment is above it. One that says why: it joins the comment
above the function it was in. One that repeats the code is deleted.

**Code cites a document in the documents' own form, on the last lines of the function's
comment, and `tools/xref.py` reads `.c` and `.h` too.** A dead citation in code fails the scan
as one in a document does.

    fast          nothing
    scalable      holds as the documents move
    modular       one form of citation, in code and in documents
    extensible    nothing
    minimal       a few lines of `tools/xref.py`
    explicit      the reason for a function can be reached, and that is held

**A function that is not `static` always has its comment, and `tools/style.py` holds that. A
`static` one has it when its name and arguments do not say it.**

## The refactor, as it goes

Every commit is measured beside a worktree of the tree before the stage, `dev` at `40c921b`,
built by the same script.

    every type ends in _t         72 typedefs of the library, the backends and the SDKs, and
                                  every use. The descriptor's word for text follows the type:
                                  `bridge_text_t`. The types a test module declares wait for
                                  their directory
    files in snake_case           the five headers, the SDK's `bridge_wasm.wat`, and what is
                                  built: `bridge_wasm`, `bridge_native`
    every tracked name            rule O4 was read as reaching every tracked file and folder
                                  of ours, at Enkrid's word: 63 documents and transcripts lose
                                  their hyphen, `host_api.md`, `build_refused.md`,
                                  `storage_sparse.txt`. `tools/xref.py` had matched only names
                                  in lower case, so a citation of `plan.md` or of a stage's
                                  document was never checked: it reads every name now
    and in lower case             19 documents whose names had capitals: `readme.md`, `plan.md`,
                                  `style.md`, `design.md`, and every stage's. `tools/style.py`
                                  holds rule O4 for every tracked file and folder of ours
    the verb, at the seams        43 names: `bridge_run_phase`, `bridge_list_modules`,
                                  `bridge_get_span`, `bridge_count_rows`, `bridge_is_alive`,
                                  `backend_get_abi`, `os_get_now`, and the rest of the table
                                  Enkrid was shown. The sparse set is spelled out where it meets
                                  a verb: `bridge_get_sparse_index`. The entry `system` is
                                  `add_system`, in the ABI, both SDKs and the two messages
                                  that name it
    the entries at load           `declare`, `add_system` and `observe` take one struct by its
                                  offset: `bridge_declaration_t`, `bridge_system_statement_t`,
                                  `bridge_observer_statement_t`, asserted in `bridge_abi.h`
    attach without its length     `attach(id, slot, ptr)`, `BRIDGE_ZEROED`, and the SDKs lend
                                  the value: `bridge_lend`, `bridge_native_lend`. The case
                                  `storage-tagbad` went with the refusal it was for: a tag
                                  cannot be handed a length any more. Every line within what
                                  two builds differ by, `detach` included
    the backend seam              `module_compile(b, source, err)`, `instance_create(m,
                                  memories, count, err)`, `define_func(i, entry, err)`, with
                                  `backend_source_t` and `backend_entry_t`
    a line of the log             `bridge_log(level, line)` and `bridge_native_log(api, level,
                                  line)`, with `bridge_line_t` in each SDK

With these, no entry of a seam and no function of an SDK takes more than four arguments.

**A schema macro was two statements.** `tools/style.py` had read only the first line of a
macro; reading all of it, `BRIDGE_SCHEMA_STRUCT` and the two beside it infringed rule H3: each
made a typedef and the string that describes it. An exception was proposed, since a type and
its descriptor must be one text, and Enkrid chose the rule: **two macros, `_TYPE` and `_DESC`,
and the author names the body once**, in a macro of their own that is handed to both. Rule H3
says that a macro which takes nothing may name what stands between a type's braces. 70 types
of the test modules were rewritten, and the descriptors they yield are the same bytes: every
case passes unchanged.

**The four macros for a text are typed constants**: `BRIDGE_NAMESPACE`, `BRIDGE_SHARED`,
`BRIDGE_PRIVATE` and `BRIDGE_NO_CONFIG`, each a `constexpr char []`. One length had been taken
with `sizeof` from a copy of one of them, and became the size of a pointer when the copy became
one: 69 cases said so at once, and the length is taken from the constant now. The wasm SDK's
import attribute takes a literal, and spells the namespace.

**What two builds differ by when their code is the same.** After the two renames the call of
`spawn` measured 4.8 to 4.9 ns where the tree before measured 4.5 to 4.7, in whichever order
the two were run. The four files a change goes through were compiled to assembly in both trees:
no instruction differs, only the names and paths in the debugging records. So 0.3 to 0.5 ns on
a line that times a call is what two builds of the same code differ by, in two directories, and
a line is called slower only beyond that. The call of `detach` that measured 0.5 ns slower
when `attach` lost its length is inside it.

**What was held first**, by `tools/style.py` with the tests: `include/bridge_schema.h` and
`include/bridge_abi.h`. In the second every layout keeps its asserts, one a line, and what a
member is for is said above the struct, since a line holds 80 columns.

**The workflows have not run since the commit that gave the seams their verb.** GitHub answers
that the job was not started for a reason of billing on the account. Every commit since was
built for Linux and macOS here and run on Windows only, in release and in debug.

**`include/` is held**, all four headers. `BACKEND_ENTRIES`, a macro that was a list of names,
is a table, `BACKEND_ENTRY_NAMES`, and the macro that stated the offsets of a module's
configuration is the asserts it expanded to, one a line.

**The operating system's layer is one**, `utils/os/`, and is held. The three copies are gone:
`src/os.c`, `backends/wasm/os.c`, `backends/native/os.c` and their headers. A platform is a
folder, `windows/` or `posix/`, that the build takes by its target, so the layer tests no
macro: with both halves in one file a file held ten functions, and rule O3 allows five.
Seventeen files, none over five functions. The wait of an event on POSIX was one loop with two
branches inside, and is two loops, one for a wait with an end and one for a wait without.

**The small files of `src/`, so far.** Each was cut by what it does, a function of twenty
lines at most and a file of five functions at most, and what several files of one unit share
is a header of that unit:

    backend.c       three functions. `backend_open` writes its reason in a `backend_err_t`,
                    where it took a buffer and its length
    pool.c          three files: the pool made and unmade, a job taken and run, one run. A
                    run is handed one `pool_batch_t`, where `pool_run` took five arguments
    schema.c        seven files and `schema_reader.h`: tokens, what a token is, a value, a
                    field's type, a struct, constants, the whole. `schema_parse` is handed
                    one `schema_source_t`, where it took seven arguments. Every refusal
                    keeps its words
    config.c        eight files and `config.h`. What writes a setting into a field is found
                    in a table by the field's kind, where it was a switch of eight arms in
                    a function of ninety lines, and the range of each integer kind is a
                    table too

    bridge.c        eight files, `say.h` and `bridge_inside.h`. `bridge_create` was 135
                    lines with twenty `goto refuse`: it is a list of parts, each answering
                    whether it was refused, and one place that destroys what was made. A
                    record of the library's own is `say_error(b, ...)`, or
                    `say_about(b, about, ...)` with a `say_t`, where `say` took five
                    arguments and more
    schedule.c      ten files and `schedule.h`: the matrix, the deployment's pairs, a
                    conflict, a phase's edges, its refusals, its levels, a dispatch, a
                    level's jobs, a run, the budgets. The stress run, which dispatches
                    through all of it, is where it was: 1382 to 1422 ms on four threads
                    beside 1378 to 1429

    block.c         ten files, `block.h` and `region.h`: a region's gaps, a piece taken,
                    the column region, what a module is given, the log ring, the barrier,
                    the budget's checks, the layout of the shared block, the block made,
                    the block unmade. `block_create` was 95 lines
    inside.h        what a bridge, a module, a system and a declared type are. Every member
                    has a line, and what each is for is said above its struct. The two
                    structs with no name have one: `system_term_t`, `module_mark_t`

A chain of `if` and `else if` over the kinds of a thing became, each time, a table indexed by
the kind, which is what rule C1 asks for.

**Storage, as it is cut.** `storage.c` had 97 functions in one translation unit, and is moved
out a group at a time, each group measured before the next:

    names             every function took its verb first, with no line of code moved
    what it holds     `storage_inside.h`, and the helpers for a set, a value, a row and what
                      an entity carries, in seven headers
    a table           its layout, its growth, how it is found and listed, made, freed and
                      stripped, in six headers

**What the cut cost, and what was found.** The lines that time a change of one entity did not
move. The four lines that time a table did, and `storage measure` printed them to a tenth of
a microsecond, which hid it: they are printed to a nanosecond now.

    a walk of a set       asking a set for "the next type after this one" each time, where
                          the old loop kept the word and cleared a bit: 20 ns a table, with
                          nine types to a table. `storage_walk.h` keeps the word
    growth                the function was 505 instructions where the old one was 318, by the
                          compiler's own count. A table's first block has a path of its own,
                          since nothing moves into it: 373
    in other .c files     tried first, and no faster than headers: undone, and all of it is
                          `static inline`
    not the cause         tried and measured, each alone: the helpers kept out of line, the
                          old form of the table lookup and of the two term checks, the code
                          shifted by an unused function, the `inline` keyword left off

    rows and values   a row added, removed and moved, a value found, a sparse set joined and
                      left, an entity gone, what observers are owed: four headers
    the barrier       `storage_apply.c`, five steps of a few lines each, over the fold, the
                      reservation, the placing and the queues in seven headers
    what is written   `storage_publish.c`, `storage_cells.c`, `storage_spans.c`. Publishing
                      was one function of 98 lines
    unload            `storage_drop.c` and two files beside it. Dropping was 71 lines
    the rest          declaring, terms, ids, carriers, reload, create: nine files

`storage.c` is gone. Storage is 23 headers and 17 source files, none over five functions and
no function over twenty lines, and `storage.h` states what is asked of it in the style.

**How it is measured now.** `storage measure` and `storage weigh` are run on the tree before
the stage and on this one in turn, five rounds, and the least of each line is kept: the
machine runs other things, and one round a tree showed 119 to 149 ns on a line that had not
changed.

**Where storage stands against the tree before the stage**, the least of five rounds, in ns:

                                            before    now
    attach, the first component             26.3      26.0
    attach, a second component              28.4      28.8
    detach                                  27.2      26.6
    destroy                                 14.5      14.5
    attach, a component it has              12.0      12.6
    a sparse set given                      26.6      27.5
    direct-indexed given                    17.1      17.6
    a table made                            277       286
    a table emptied                         240       247
    the stress run, 4 threads, ms           1404      1395

**Open.** Three lines are slower by about half a nanosecond, which is what two builds of the
same code differ by, and are not explained beyond that: an attach of a component the entity
has, a direct-indexed component given, and a table made or emptied, by 7 to 9 ns of 250. The
done-when asks that no line be slower, so they are looked at again before the stage closes.

**The entries a module calls in a phase** are out of `abi.c`: `abi_calls.c` for spawn, destroy,
attach and detach, `abi_alloc.c`, `abi_define.c` with the eight imports as a table, and
`abi_entry.h` for what they share, a change put on the queue and the check that the module may
make it. A change is handed to the queue as one `change_t`, where `queue` took five arguments.
The call itself got faster, the least of five rounds, in ns: attach 9.5 to 8.2, detach 7.8 to
6.3, destroy 5.9 to 5.1, a sparse set given 10.3 to 8.7. Nothing was done for it but the cut:
the entries are a translation unit of their own now, and the compiler puts the queueing
inside each.

**The entries a module calls at load** are out of it too, and `abi.c` is gone. `declare` is nine
files around one `declaring_t`: what is stated (`abi_stated.c`), names (`abi_names.c`), the type
of that name there already is (`abi_replace.c`, `abi_moved.c`), a struct laid out again at a
reload (`abi_relay.c`, `abi_relay_fields.c`, `abi_relay_run.c`), the descriptor read
(`abi_layout.c`) and the type made (`abi_make.c`), with the entry in `abi_declare.c`.
`add_system` and `observe` are five around one `subscribing_t`: `abi_subscriber.c`,
`abi_subscriber_room.c`, `abi_term.c`, `abi_subscribe.c`, and the two entries with `abi_hear` in
`abi_system.c`. Every refusal says what it said, in the order it was reached. Three things are
not as they were, and none shows in a transcript:

    a system or an observer refused at   its name, its full name and an observer's list were
    a change or a term                   left allocated. They are freed
    the types a field names              room was counted field by field. It is one a field,
                                         which is never fewer
    a count over 32 bits in a refusal    the statements carry 32 bits since decision 2, so
                                         the text is written from 32

Nothing here runs in a phase. Measured all the same, five rounds beside the tree before the
stage: no line moved beyond what the lines it does not touch moved.

**`lifecycle.c` is gone**: 1,011 lines and 29 functions, of which `checks` was 144 lines,
`bridge_reload` 180 and `bridge_load` 81. It is 24 files now, none of them with a function over
20 lines:

    module.h, reload.h          what a load, an unload and a reload share: `module_check_t`,
                                `module_start_t`, `module_call_t`, `reloading_t`
    module_check*.c, module_say.c
                                what is refused from a config alone, a group of keys a function
    module_record.c, module_discard.c, module_types.c
                                a module's record made and freed, and what it declared let go
    module_call.c, module_instance.c, module_start.c
                                its private memory, its instance, and the first call into it
    module_share.c, module_pointed.c
                                its share of the persistent arena, and who points into it
    bridge_load.c, bridge_unload.c, bridge_reload.c
                                the three entries
    reload_check.c, reload_share.c, reload_record.c, reload_rows.c, reload_fail.c
                                the steps of a reload

`checks` took four values out through pointers and `start` took six arguments; each is handed
one struct. `neither` took five arguments and takes four, since who is refused is in the struct.
The three `goto failed` of `bridge_load` are one function that is called. Every record says what
it said, in the order it was reached: the 237 cases pass unchanged, release and debug.

Nothing here runs in a phase either. The lines that moved in five rounds are the three that are
open above, with a table made at 0.263 to 0.275 us among them.

**The two backends.** `backends/wasm/wasm.c` (623 lines) is eleven files and
`backends/native/native.c` (288) is seven, each with a header of what its files share,
`wasm_inside.h` and `native_inside.h`. Three things are not as they were:

    a function Bridge-ASM700 offers    wasmtime called it with six arguments: the arguments and
    a wasm module                      the results, each an array of tagged values with its
                                       length. It is defined with
                                       `wasmtime_linker_define_func_unchecked` now, whose callback
                                       takes four: one array of raw values, the arguments where
                                       the results go. The kinds are the ones the signature
                                       stated, kept with the function
    the adopted block wasmtime is      it was a thread-local variable of the file. It is a member
    asked for                          of the backend, which wasmtime hands the creator back as
                                       its `env`: G4 asks for no global that is not constant
    the native backend built as        it was the same source compiled with a macro. The answer to
    another backend seam               `backend_get_abi` is a file of its own, and the test links
                                       `tests/support/native_other_abi.c` in its place

A call from a wasm module into Bridge-ASM700 was timed in scratch, twenty million `alloc(0)` in
one `init`, beside the tree before the stage: 48 ns a call before, 8.6 ns now. The entry of the
native backend, which every call from a native module goes through, is `static inline` in
`native_inside.h`, as decision 4 has it for what runs in a phase; the call columns of `storage
measure` are where the cut of the ABI left them.

**The one exception.** `make_memory` in `backends/wasm/wasm_adopted.c` takes seven arguments. Its
signature is wasmtime's, `wasmtime_new_memory_callback_t`, and wasmtime has no other way to be
answered for a memory, so there is nothing of ours to reshape. It is the first entry of
`style.md`, *Exceptions*, named with its function and its reason, and `backends/wasm` is held.
Refused: rewording F5 for any function handed to a library, since the checker cannot tell which
those are and the list would only be hidden; and a seven-argument shim in the `wasmtime` folder,
which O1 excuses, since that is our code where nothing checks it.

**The two SDKs.** `bridge_wasm.h` had 24 functions and `bridge_native.h` 11, and O3 allows five a
file, in a header as anywhere. Each is the header a module includes still, and includes the
others: `bridge_wasm_acc.h` for the imports, then `_scratch`, `_span`, `_entity`, `_kept`,
`_toggle`, `_observer` and `_log`; `bridge_native_api.h` for the table, then `_kept` and `_log`.
Every function is `static inline` as it was, so what a module compiles to is the same loads.
One macro went: `acc_row` was defined to `acc_row_unchecked` in a module built without the row
test, lower case, which V1 refuses. The function is declared under the one name either way now,
and the import it is bound to is the one that was stated.

**The second exception.** A test module keeps the slots `declare` and `add_system` answered, and
what it counts from one call to the next, in variables of its file: 69 of them, which G4 does not
allow. An export takes nothing, wasm's `() -> i32` and the native table alike, scratch is reset at
each barrier, and an offset into the persistent arena would itself have to be kept somewhere. So
`style.md` excepts G4 under `tests/modules`. A host program keeps none: what its sink counts is in
the fixture the sink is handed.
