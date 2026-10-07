# What was refused: Threads

The record of `threads.md`: `readme.md`, *Three layers, three seams*.

## Threads

Refused: data parallelism, one system run on several threads with each over a part of its spans. It
is the one model that would speed up a single loop, and it needs an instance of the module for each
thread, so that spans, the queue of changes and the count of failures all become an instance's, and
a system that keeps anything between rows breaks. Nothing measured asks for it. The condition that
brings it back is a deployment, measured, in which one system takes most of a phase while other
threads have nothing to run: `plan.md`, stage 19, measures that.

Refused: task parallelism inside a module, its systems that do not conflict run at once. Each would
need an instance of its own for the same reason, and it helps a module of many small systems and not
the heavy one. The same condition.

Refused: concurrency inside a module, a system paused part-way for another. It gains no time, since
nothing a system calls ever waits - `limits.md`, *What bounds a run* - and a module that wants work
spread over several phases keeps where it is in a component. This section said the model "cannot be
worked around", which was not so: it can, at the price above. The condition that brings it back is
an ABI entry that waits.

Refused: asking the operating system how many cores it has, which is a value nobody stated and the
wrong one inside a host with a thread pool of its own. The condition that brings it back is a host
with no way to state a number, which would then state "as many as there are", by a name for it.

Refused: borrowing the host's threads through a callback, a new seam nothing asks for yet. The
condition that brings it back is a host whose job system Bridge-ASM700 has to run on.

Refused: one lock around every entry, which is what this replaced. It serialised modules that had no
reason to wait for each other, and it made the ids two modules were given in one level a matter of
which thread arrived first. No condition brings it back.

Refused: a queue inside storage for each thread, which is what an ECS offers for this: whose change
is applied first would then follow which thread ran whom. No condition brings it back.

Refused: declaring that a system mints, as a write is declared, so that two such never share a level
- no key to state, and two modules could then never spawn at once. The condition that brings that
one back is the two keys below proving harder to state than the parallelism is worth.
