# The module model

What was weighed here and refused is `module_refused.md`, *What was refused: The module model*.

A **module** is code Bridge runs through a backend. There is one model, whatever the backend, and
Bridge has no opinion about which backend you build for - choosing is the deployment's, because the
trade is about guarantees rather than about where code belongs.

Two backends exist: **wasm**, which runs a core wasm module in a sandbox, and **native**, which runs
a shared library in Bridge's process. "A wasm module" and "a native module" in these documents
say which backend runs it and nothing more. They are not two kinds: the ABI, the rules and the
scheduler are the same for both.

This file is what every module shares. The calls are `abi.md`; who may name a component is
`grants.md`. How a module is written for a backend is that backend's SDK, and is said in that
backend's documents, which are not here.

## Identity

`namespace:name@version` plus the **ABI version**, and **four things use it**: it is the config key,
the grant key, the namespace components are declared in, and how `[order]` names a system -
without the version there: `schedule.md`, *Order within a phase is never implicit*. A module without
one is refused.

Bridge is handed it as data, so **a host that cannot supply an identity cannot load a
module**. The convention is a named section in the artifact - a wasm custom section on one backend,
an object-file section on the other - which a host reads, or has read for it: *The parser of a
manifest*. A host sourcing it elsewhere is not doing anything wrong. Where the bytes sit and how
they are read is the only part that differs, and is each backend's to say.

### The manifest

That section is the module's **manifest**: the identity, then whatever the author states, as
NUL-separated `key=value`.

    godot:adapter@0.1.0\0abi=1\0pinned=true\0scratch=65536

**It carries what only the author knows**, so that it travels with the artifact: the ABI version the
module was built against, whether it must stay on one thread, how much scratch space its code uses,
how long its init takes. Any per-module key of `config.md`, *The keys*, may be stated there.
Combining those values with a deployment's own is the host's business, and Bridge sees
one flat struct either way.

It is the shape of a Linux kernel module's `.modinfo`, for one reason: the values have to be
readable before any of the code runs.

### The parser of a manifest

It is not built.

`bridge_manifest_parse` is a helper that ships beside the library and not inside it. Handed a
module's bytes or its library, it answers the identity and the values the author stated. What
it walks differs by format: a wasm module's custom section, ELF's `shdr` and `shstrtab`, PE's
section table after the optional header, Mach-O's load commands. One implementation for every
host that wants it, and none for a host whose module metadata comes from elsewhere.

## What no module may do

Four rules, and they hold on every backend:

    write a component it declared read
    hold a reference into storage across a barrier - a row number, a span
      pointer, whichever word applies
    hold a scratch-arena offset across a phase - including one it logged
    rely on iteration order

**Two of them are about not keeping anything**, and both follow from the barrier - `schedule.md`,
*Phases*. Rows move there, so a reference outlives its meaning the moment one passes. The fourth is
different: iteration order is the order of tables and of rows in them, which changes as entities
come and go, so there was
never an order to keep.

**None of the four is enforced**, on either backend. Access mode is a scheduling input and nothing
else - `data.md`, *Access mode* - and the other three are things no check could see: two are about
not keeping something, and one is about not assuming an order. A module that breaks one gets wrong
answers rather than a diagnostic, and what finds it afterwards is `validation.md`, *What a bad write
costs, and what finds it*.

Nothing on this list is a wall. The one refusal that is, is the wasm backend's, and it is not a
shared rule: it is the first guarantee below, stated as a rule. Each backend adds exactly one
refusal of its own, in its own SDK.

## Three guarantees differ

    untrusted        on the wasm backend a module cannot reach memory it was not
                     given, so one that is not trusted can be run. on the native backend
                     it is code in Bridge's process, and nothing contains it
    limits           a work limit, timeouts and a memory limit apply on the wasm backend -
                     `limits.md`, *What bounds a run*. native code keeps none of them
    clean reload     on the wasm backend a reload is a fresh instance over surviving
                     storage. on the native backend a module can be unloaded and
                     reloaded, but static state and held pointers make it fragile
                     rather than routine

Those are **guarantees a backend gives, not capabilities a module has.** A backend states them as
flags, and a config key that demands one the backend lacks is refused - `capabilities.md`,
*Capabilities, so the swap is honest*. Nothing in the list is something one module can do and
another cannot.

## So what decides

The native backend is for what cannot be reached from inside wasm, which that backend's
documents list. The wasm backend is for when the three guarantees above are worth having.

Pure computation can run on either. A physics solver on the wasm backend pays the wasm execution
penalty and gets bounds checks, a work limit, a trap instead of a crash and a clean reload; on the native
backend it runs native and gets none of that. **That is a trade, not a rule**, and the deployment
makes it.

How a module is written is the backend's business and nobody else's. The
scheduler cannot tell which backend a module is on, which is the property worth having.
