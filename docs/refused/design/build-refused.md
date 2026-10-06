# What was refused: The build

The record of `build.md`: `README.md`, *Three layers, three seams*.

## One compiler

Of `build.md`, *One compiler*.

Refused: clang-cl beside `cl`, which keeps Visual Studio as something to install. The condition that
brings it back is zig unable to build a Windows target that is needed.

Refused: each platform's own compiler, which is where this started. The condition that brings a
second compiler back is a class of bug that only one would have caught.

## One script

Of `build.md`, *One script*.

Refused: `build.zig`, which has the cache and the targets built in, and is written in a language
whose build interface changes between releases. The condition that brings it back is zig at 1.0.

## Platforms

Of `build.md`, *Platforms*.

Refused: x86-64 as well, a second job and a second library for a platform that is leaving. The
condition that brings it back is a deployment on one.

## The standard

Of `build.md`, *The standard*.

Refused, then: C99 for everything. The condition that brings an earlier standard back for the source
is zig no longer compiling C23.

Refused: C11, which is what the code already was and takes nothing more. The same condition.

Refused: C23 for the source with the headers held to C99, which was the first recommendation: the
ABI header is where a stated width and an asserted ABI layout are worth most. The condition that
brings an earlier standard back for the headers is a host or a backend that has to be built by a
compiler without C23.

## The C library

Of `build.md`, *The C library*.

Refused: a whole C library in the tree. musl is one for Linux only, and would make every native
module a musl module. Cosmopolitan covers three systems by being a way to build programs, with its
own compiler and file format. The condition that brings it back is `PLAN.md`, stage 12, finding
routines that cannot be carried one at a time.

Refused: writing the routines here, a `printf` and an allocator that C already has. The condition
that brings a whole library back is native modules being required to be built against it anyway.
