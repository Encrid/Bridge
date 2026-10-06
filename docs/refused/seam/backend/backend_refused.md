# What was refused: The backend surface

The record of `backend.md`: `readme.md`, *Three layers, three seams*.

## What a backend is asked for

Of `backend.md`, *What a backend is asked for*.

Refused: the backend stating which it needs so that Bridge-ASM700 could refuse first, which has
Bridge-ASM700 inspect what it is built not to and answers once more a question the backend already
answers. The condition that brings it back is a backend for which "which one" is not obvious.

Refused: one tagged source, which makes a host that has both choose for a backend it does not know.
The condition that brings it back is the one above: a backend for which "which one" is not obvious.

## Memory is adopted, not created

Of `backend.md`, *Memory is adopted, not created*.

Refused: 65,536 written into Bridge-ASM700, to which it rounded every block, which is what this had:
one backend's number in the library, applied to another backend's modules, with no reason written.
No condition brings it back.

Refused: the largest shape for every block, which is what this had. The wasm backend asks for four
gigabytes and a guard, and beside it every native module's private block reserved that much address
space for nobody: nothing committed, and a load refused all the same where a process's address space
is capped. The condition that brings it back is a module's private block mapped by a backend other
than its own.

Refused: one backend creating the block and the others adopting it, which works for exactly one
backend that insists on allocating and makes "who owns the memory" depend on which backends a
deployment has. The condition that brings it back is a backend that cannot run over memory it did
not allocate, whatever its shape.

Refused: no handle at all, with `instance_create` taking a pointer and a length. It is two entries
fewer, and that is all it is. A backend owns what a memory is to it, which a pointer and a length at
the seam would decide for every backend; a handle with a counterpart states a lifetime, where "the
block outlives the instance" would be held by nothing; and the wasm backend has something to keep a
block: the module that holds it for wasmtime, built once and not for every instance - `wasmtime.md`,
*Memory*. On the native backend the handle is the two numbers. The condition that brings it back is
every backend keeping nothing in it but the two numbers. This paragraph said the handle was "kept
for the backend" that did not exist yet, when one that did had a use for it nobody had looked for.

Refused: the handle dying with the instance that used it, which is wrong for the shared memory -
every instance uses it - and hides a count inside each backend. No condition brings it back while a
memory is shared by instances.