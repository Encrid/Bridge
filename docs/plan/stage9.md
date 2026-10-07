# Stage 9: C23, used fully

Done, on Windows, on Linux and on macOS.

`plan.md` stage 9 was "C99, and the context passed". This document is why it is not, and what the
stage builds in its place.

## What compiling it showed

The decisions of stage 7 were made by argument: C99 for the whole tree, and with it no
thread-local storage, flecs' log to every deck, and typed accessors without `_Generic`. Before
planning them, each was built in a scratch directory, and four things came out that the argument
had not seen.

**wasmtime's headers are C11.** `_Alignof`, anonymous structs and `static_assert`: the wasm
backend cannot be compiled as C99 with them in it. So "one number for the whole tree", which was
all C99 offered, was not on offer.

**Two of the nine patched flecs functions have no world in scope**, `flecs_table_fast_append` and
`flecs_table_merge_vec`, and the functions that allocate see only an allocator. Passing the
context meant changing their signatures and their callers', in a patch whose value is that it is
small.

**The alignment idiom is not C99.** `offsetof(struct { char c; T t; }, t)` was written into
`build.md` as the standard replacement for `_Alignof`. The compiler refuses it: a type defined
inside `offsetof` is C23.

**The accessors did work.** All forty-four test modules built as C99 with the temporary in place
of `_Generic`, forty-two of them to the same code, the row loop included. Three reads in two
modules became a load and a reinterpret. This one would have held.

Three of the four were stated as facts in stage 7 and were wrong until compiled.

## What was settled

    the standard         each piece at its own: flecs C99, wasmtime's headers C11, ours C23
    how much of C23      all of it that replaces an older spelling: `build.md`, *The standard*
    the five headers     C23 as well, the three that declare the seams included

Withdrawn, since nothing forces them: the context passed in place of thread-local storage, flecs'
log to every live deck, the accessors through a temporary, the alignment helper. The code keeps
`thread_local`, `_Generic` and the native ABI it had.

## What it built

The rule of `build.md`, *The standard*, over about eight thousand lines, and `tools/c23.py` to
hold it there: it runs with the tests and fails on an older spelling, and was shown to.

    nullptr for NULL                      266
    f() for f(void)                       156
    [[maybe_unused]] for (void)x;          46
    large numbers with separators          40
    enums named, with a width              27, and the fields and parameters that hold them
    numbers with a name, as constexpr      24
    = {} for { 0 }                         22
    auto where a cast names the type       11
    thread_local, and its macro gone        3

Every layout `bridge-abi.h` states is asserted in it: size, alignment and each offset of the
eight structs. All of them held. A compiler's own attribute is written in the standard brackets,
and `ECS_ENUM` and `ECS_BITMASK` state the four bytes Bridge lays such a field out as,
where a module's compiler chose them before.

**`[[nodiscard]]` found thirty-six dropped answers**, in the library and the tests. None was a
failure being ignored. Five were a schedule rebuilt after something was taken out of it, which
cannot be refused, and say so now where the answer is dropped. Two functions lost the attribute
again: what they answer is information, not a status.

Nothing compiled differently. Of the fifty-five test modules, fifty-four are the same code byte
for byte as before the stage with their debug sections stripped, and the other had a line of its
source changed. The row loop reads 0.39 ns a row, as it did.

## What changed that a host can see

**A stated size no offset reaches is refused by its key.** `[budget] entities`, `persistent` and
`columns` are each checked against 32 bits before any sum is made of them, where one too large
used to be reported as a total that had wrapped. And a module's `scratch`, `log_ring` and `spans`
are laid out with checked additions: together past 32 bits they are refused at load. Before, the
offsets were truncated and wasmtime answered "failed to parse WebAssembly module".

The other sizes computed in the library are counts times a small size in 64 bits, or are bounded
by a check already made, and cannot overflow: they are left as they were.

**A backend's library is closed only if the backend says it can be**: a seventh capability,
`unload` - `capabilities.md`, and `create.md`, *Destroy*.

## What it found

**Unloading wasmtime crashed, and had since stage 2.** `[[nodiscard]]` was being applied when one
test failed once with no output. Run alone it passed a hundred and fifty times. Six copies at
once made it fail about one run in sixty, by a segmentation fault or by never ending, and the
tree before this stage did the same, so it was not the stage's.

`bridge_destroy` closed every backend's library. The wasm backend's took wasmtime out of the
process with it, and wasmtime keeps threads of its own for compiling and a handler registered
with the operating system for faults, with no call that stops either. The test that failed is
the one that makes three decks in a row: the second loaded wasmtime again beside what the first
had left pointing into unmapped memory.

    six copies at once, a hundred runs each     crashed    never ended
    every library closed at destroy                  10              2
    the wasm backend's library left loaded            0              0

It was asked what should close a library, and the answer was that the backend says. Never closing
any was the recommendation, and it lost to not deciding on behalf of backends that can decide for
themselves.

**macOS does not unmap a library that has a thread-local variable.** The native backend's has
one, so there it is closed and stays in the process, where Windows and Linux take it out. The
test asks the operating system and not Bridge, so it saw this, and it now states what each
system does. The cause is what that system's loader is known to do, not something measured here.

**One hundred and twenty-eight passing tests did not find the crash.** A busy machine did. The
build's test runner says what a failed case ended with now, where it said only what was missing
from its output.

## Tests

One hundred and thirty-three: the hundred and twenty-eight, and

    c23                 no older spelling in what is ours
    stage4-libraries    what a destroyed deck leaves in the process, by backend and by system
    stage3-beyond       a budget no offset reaches: refused by its key
    stage3-toobig       budgets each in reach and together not: refused. This refusal existed
                        and had no test
    stage3-bigprivate   a module's regions past 32 bits: refused at load

In a release build and a debug one on Windows, and in a release build on Linux and on macOS.
