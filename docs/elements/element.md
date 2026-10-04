# The element model

An **element** is code Bridge-ASM700 runs through a backend. There is one model, whatever the backend, and
Bridge-ASM700 has no opinion about which backend you build for - choosing is the deployment's, because the
trade is about guarantees rather than about where code belongs.

Two backends exist: **wasm**, which runs a core wasm module in a sandbox, and **native**, which runs
a shared library in Bridge-ASM700's process. "A wasm element" and "a native element" in these documents
say which backend runs it and nothing more. They are not two kinds: the ABI, the rules and the
scheduler are the same for both.

This file is what every element shares. The calls are `abi.md`; who may name a component is
`grants.md`. `wasm.md` and `native.md` describe only how an element binds on each backend.

## Identity

`namespace:name@version` plus the **ABI version**, and **four things use it**: it is the config key,
the grant key, the namespace components are declared in, and how `[order]` names a system. An
element without one is refused.

Bridge-ASM700 is handed it as data, so **a host that cannot supply an identity cannot load an element**.
The convention is a named section in the artifact - a wasm custom section on one backend, an
object-file section on the other - and a helper beside the library reads it. A host sourcing it
elsewhere is not doing anything wrong. Where the bytes sit and how they are read is the only part
that differs: `wasm.md` and `native.md`, *Identity*.

## What no element may do

Four rules, and they hold on every backend:

    write a component it declared read
    hold a reference into storage across a barrier - a row number, a span
      pointer, whichever word applies
    hold a scratch-arena offset across a phase - including one it logged
    rely on iteration order

**Two of them are about not keeping anything**, and both follow from the barrier - `schedule.md`,
*Phases*. Rows move there, so a reference outlives its meaning the moment one passes. The fourth is
different: iteration order is archetype order, which changes as entities come and go, so there was
never an order to keep.

**None of the four is enforced**, on either backend. Access mode is a scheduling input and nothing
else - `data.md`, *Access mode* - and the other three are about not keeping something, which no
check could see. An element that breaks one gets wrong answers rather than a diagnostic, and what
finds it afterwards is `validation.md`, *What a bad write costs, and what finds it*.

Nothing on this list is a wall. The one refusal that is lives in `wasm.md`, *What a wasm element
cannot do*, and it is not a shared rule: it is the sandbox guarantee below, stated as a rule. Each
backend adds exactly one refusal of its own, in its own binding file.

## Three guarantees differ

    sandbox          on the wasm backend an element cannot reach memory it was not
                     given. on the native backend it is code in Bridge-ASM700's process,
                     and nothing contains it
    metering         fuel, timeouts and memory limits apply on the wasm backend -
                     `limits.md`, *What bounds a run*. native code is not meterable
    clean reload     on the wasm backend a reload is a fresh instance over surviving
                     storage. on the native backend an element can be unloaded and
                     reloaded, but static state and held pointers make it fragile
                     rather than routine

Those are **guarantees a backend gives, not capabilities an element has.** A backend states them as
flags, and a config key that demands one the backend lacks is refused - `capabilities.md`,
*Capabilities, so the swap is honest*. Nothing in the list is something one element can do and
another cannot.

## So what decides

The native backend is for what cannot be reached from inside wasm - `native.md` says what that
covers. The wasm backend is for when the three guarantees above are worth having.

Pure computation can run on either. A physics solver on the wasm backend pays the wasm execution
penalty and gets bounds checks, fuel, a trap instead of a crash and a clean reload; on the native
backend it runs native and gets none of that. **That is a trade, not a rule**, and the deployment
makes it.

How an element binds is the backend's business and nobody else's: `wasm.md` and `native.md`. The
scheduler cannot tell which backend an element is on, which is the property worth having.
