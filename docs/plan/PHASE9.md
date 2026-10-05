# Phase 9: C23, used fully

In progress.

`PLAN.md` phase 9 was "C99, and the context passed". This document is why it is not, and what the
phase builds in its place.

## What compiling it showed

The decisions of phase 7 were made by argument: C99 for the whole tree, and with it no
thread-local storage, flecs' log to every bridge, and typed accessors without `_Generic`. Before
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

Three of the four were stated as facts in phase 7 and were wrong until compiled.

## What was settled

    the standard         each piece at its own: flecs C99, wasmtime's headers C11, ours C23
    how much of C23      all of it that replaces an older spelling: `build.md`, *The standard*
    the five headers     C23 as well, the three that declare the seams included

Withdrawn, since nothing forces them: the context passed in place of thread-local storage, flecs'
log to every live bridge, the accessors through a temporary, the alignment helper. The code keeps
`thread_local`, `_Generic` and the native ABI it had.

## What it builds

The rule of `build.md`, *The standard*, applied to about eight thousand lines, and
`tools/c23.py` to hold it there.

One thing changes that a host can see: **a sum of stated sizes that overflows is refused, naming
the key.** Those sums were unchecked.

## What would make us stop

- wasmtime's or flecs' headers failing as pedantic C23 when one of our files includes them: then
  their directories are given to the compiler as system headers, which is how `build.py` already
  treats flecs' own source.
- The row loop compiling to other code, or reading another figure: the cause is found first.
- Anything beyond the checked sums that would change what a host or a module sees: a question.
