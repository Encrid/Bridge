# Capabilities, so the swap is honest

`backend_capabilities` is not decoration. A backend says what it can actually do, and these flags
are how it states the guarantees in `module.md`, *Three guarantees differ* - metering is three flags,
`meter`, `deadline` and `limit`, and reload is two:

                   wasm   native
    sandbox        yes    no      a module cannot reach memory it was not given
    meter          yes    no      fuel, or an equivalent
    deadline       yes    no      a running module can be interrupted
    limit          yes    no      a module's own memory has a ceiling
    reload         yes    yes     an instance can be replaced while the process runs -
                                  cleanly on wasm, fragile on native
    side by side   yes    no      two instances of one module can exist at once

`sandbox` is about the **host boundary and nothing else**. It does not mean one module cannot
corrupt another's components: they share a memory by design - see `costs.md`, *What we give up*.
idTech3 drew the same line, `VMI_NATIVE` against `VMI_BYTECODE` per VM with a cvar each and shipped
set to the safe one. It got isolation between its VMs only because the three had three blocks and
shared no data.

Each flag is demanded by a **config key**, not by a module: `sandbox` by `trusted = false`,
`meter` by `fuel`, `deadline` by a timeout, `limit` by `memory_limit`, `reload` by `hot_reload`.
`side by side` is demanded by nothing: it is how Bridge-ASM700 learns which order a reload takes -
`lifecycle.md`, *The runtime lifecycle*. So
a mismatch is not a module arguing with a backend - it is **config contradicting config**, since the
deployment chose both. It is therefore one of Bridge-ASM700's own refusals and takes init's path
under `on_error`, exactly as a denied grant or an ambiguous order does: `failure.md`, *When
something fails*.

**A key stated as "no limit" demands nothing.** `fuel`, a timeout and `memory_limit` each take `-1`
for that, which is how a module on the native backend states them at all. It is the same move as
`trusted = true`: the absence of a guarantee is written down by somebody rather than arrived at.
Refused: letting zero mean "not applicable" on a backend that cannot meter, which would give one
field two readings. `config.md`, *No unaccountable values*.

## Nothing is downgraded

idTech3 does - asked for native with no DLL present, `vm.c:486` prints a line and falls back to
bytecode, and `fs_restrict` forces the safe path regardless of what was asked. It is sound there
because the two paths are two builds of one program with identical semantics. It is still
**implicit**, and a deployment that asked for a guarantee should be told it cannot have one rather
than quietly given something else.

The override is explicit instead: a developer who wants untrusted code on the native backend writes
`trusted = true` - a statement in a file somebody can read, rather than a warning nobody does.

The native backend answers no to `sandbox`, truthfully, and Bridge-ASM700 then refuses to load anything
marked untrusted onto it, before the library is opened. That makes the native backend a deliberate
choice for code you wrote rather than a hole you discover.

Without this the surface would let a backend silently stop enforcing the guarantees these
documents promise, which is the failure worth designing out.
