# What was refused: Modules on the wasm backend

The record of `wasm.md`: `readme.md`, *Three layers, three seams*.

## A compiled module reaches them through accessors

Of `wasm.md`, *A compiled module reaches them through accessors*.

Refused: refusing every export `init` did not register, which would refuse the `memory` and
`__heap_base` a compiler emits unasked and make every toolchain strip to be loaded at all. The
condition that brings it back is an export becoming callable by something other than its own module.

Refused: a rewriter of our own doing the same substitution, which is Binaryen's job done twice. The
condition that brings it back is Binaryen being unable to inline or strip something this needs.

Refused: one memory per module with the shared and private blocks mapped into it, which makes a C
pointer enough and costs the TLB aliasing `costs.md`, *What we give up*, already declines. The
condition that brings it back is the opaque calls measurably costing the loop, or LLVM learning to
address a second memory, which retires the accessor module altogether.

## Exports

Of `wasm.md`, *Exports*.

Refused: leaving `reload` out to say "cannot", which is what these documents first had. It is a
statement made by an absence, and nothing can tell an export left out on purpose from one forgotten;
`capabilities.md` already holds that the absence of a guarantee is written down by somebody rather
than arrived at. No condition brings it back.

Refused: a `reload` that answers "no" when asked, which says it only by being tried - and on a
backend that cannot hold two instances the old one is shut down before the new one can answer. The
condition that brings that back is no backend taking that order.

## The ABI layouts

Of `wasm.md`, *The ABI layouts*.

Refused: calling them `reserved`, which is what they were called. That is the word for bytes kept
for a later use, and none is planned. Refused with it: Bridge refusing a module that left
something other than zero there, which is what an interface does when it means to give such bytes a
meaning later to programs already built. Here a changed ABI layout is a changed `BRIDGE_ABI`, and a
module built against another is refused whole. The condition that brings the check back is an ABI
layout changed under the same number.

Refused: widening those to two bytes so that a term and a log record need no `pad`. An observer's
record would keep its own, so the rule above would still be needed, and a value of 1 to 3 in sixteen
bits is the same empty bits without a name. The condition that brings it back is that no struct is
then left with a `pad`.
