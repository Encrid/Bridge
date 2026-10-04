# The element model

An **element** is a module or a plugin. There is one model for both, and BridgeC has no opinion
about which you use for what - choosing is the deployment's, because the trade is about guarantees
rather than about where code belongs.

This file is what they share. The calls are `abi.md`; who may name a component is `grants.md`.
`module-abi.md` and `plugin-abi.md` describe only how each kind binds to it.

## Identity

`namespace:name@version` plus the **ABI version**, and **four things use it**: it is the config key,
the grant key, the namespace components are declared in, and how `[order]` names a system. An
element without one is refused.

BridgeC is handed it as data, so **a host that cannot supply an identity cannot load an element**.
The convention is a named section in the artifact - a wasm custom section for one kind, an
object-file section for the other - and a helper beside the library reads it. A host sourcing it
elsewhere is not doing anything wrong. Where the bytes sit and how they are read is the only part
that differs: `module-abi.md` and `plugin-abi.md`, *Identity*.

## What no element may do

Four rules, and they hold for both kinds:

    write a component it declared read
    hold a reference into storage across a barrier - a row number, a descriptor
      pointer, whichever word applies
    hold a scratch-arena offset across a phase - including one it logged
    rely on iteration order

**Two of them are about not keeping anything**, and both follow from the barrier - `schedule.md`,
*Phases*. Rows move there, so a reference outlives its meaning the moment one passes. The fourth is
different: iteration order is archetype order, which changes as entities come and go, so there was
never an order to keep.

**None of the four is enforced**, for either kind. Access mode is a scheduling input and nothing
else - `data.md`, *Access mode* - and the other three are about not keeping something, which no
check could see. An element that breaks one gets wrong answers rather than a diagnostic, and what
finds it afterwards is `validation.md`, *What a bad write costs, and what finds it*.

Nothing on this list is a wall. The one refusal that is lives in `module-abi.md`, *What a module
cannot do*, and it is not a shared rule: it is the sandbox guarantee below, stated as a rule. Each
kind adds exactly one refusal of its own, in its own ABI file.

## Three guarantees differ

    sandbox          a module cannot reach memory it was not given. a plugin is native
                     code in BridgeC's process, and nothing contains it
    metering         fuel, timeouts and memory limits apply to a module - `limits.md`,
                     *What bounds a run*. native code is not meterable
    clean reload     a module's reload is a fresh instance over surviving storage. a
                     plugin can be unloaded and reloaded, but static state and held
                     pointers make it fragile rather than routine

Those are **guarantees BridgeC can make about a module and cannot make about a plugin.** Nothing in
the list is a capability one has and the other lacks.

## So what decides

A plugin exists when something cannot be reached from inside wasm - `plugin-abi.md` says what that
covers. A module exists when the three guarantees above are worth having.

Pure computation can be either. A physics solver as a module pays the wasm execution penalty and
gets bounds checks, fuel, a trap instead of a crash and a clean reload; as a plugin it runs native
and gets none of that. **That is a trade, not a rule**, and the deployment makes it.

## Three mechanics differ

A different three from the guarantees above. A guarantee is something BridgeC can promise about one
kind and not the other; these are two surfaces onto one set of operations. **No capability is in
either list.**

    storage      a module reads spans published into its private memory; a plugin is
                 handed a pointer and a descriptor table
    registering  a module's systems and observers are wasm exports BridgeC resolves by
                 name; a plugin passes a function pointer
    lifecycle    three wasm exports, or three exported symbols

The scheduler cannot tell the two apart once either is registered, which is the property worth
having.
