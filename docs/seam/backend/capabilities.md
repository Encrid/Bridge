# Capabilities, so the swap is honest

`backend_capabilities` is not decoration. A backend says what it can actually do, and these flags
are how it states the guarantees in `module.md`, *Three guarantees differ* - metering is three flags,
`meter`, `timeout` and `memory_limit`, and reload is two. The last is about the backend itself:

                   wasm   native
    untrusted      yes    no      a module that is not trusted can be run here
    meter          yes    no      fuel, or an equivalent
    timeout        yes    no      a call can be given a timeout, and is abandoned past it
    memory_limit   yes    no      a module's own memory can be given a ceiling
    reload         yes    yes     an instance can be replaced while the process runs -
                                  cleanly on wasm, fragile on native
    side by side   yes    no      two instances of one module can exist at once
    unload         no     yes     its library can be taken out of the process

**`untrusted` is the deployment's `trusted`, answered.** They are one guarantee and so one word:
a deployment states whether a module is trusted, and a backend states whether it runs one that
is not. A module that is not trusted loads only on a backend that says so. How a backend keeps
that is its own, and is named in its own document and not at this seam: the wasm backend does it
with a sandbox, and another might with a process, or with nothing a sandbox describes.

What it promises is two things. A module cannot reach memory it was not given. And every export
the backend hands back has been checked to be one it can call - `signatures.md`, *A call takes
nothing and answers a status*. A backend that does not answer `untrusted` promises neither: the
native backend has a symbol from the loader, which carries no type, and checks nothing, and the
deployment that loads a module there has stated `trusted = true`.

Refused: `sandbox` as the capability's name, which is what it was. It named one backend's way of
keeping the guarantee, and gave one thing two words, one where it was asked and one where it was
answered. No condition brings it back. Refused: a capability of its own for checking exports,
with a key to demand it. `trusted` already states that nothing contains a module's code, and a
wrong signature is one more thing nothing contains. The condition that brings it back is a
deployment that must have exports checked on a backend that does not contain its modules.

The guarantee is about the **host boundary and nothing else**. It does not mean one module cannot
corrupt another's components: they share a memory by design - see `costs.md`, *What we give up*.
idTech3 drew the same line, `VMI_NATIVE` against `VMI_BYTECODE` per VM with a cvar each and shipped
set to the safe one. It got isolation between its VMs only because the three had three blocks and
shared no data.

Each flag is demanded by a **config key**, not by a module: `untrusted` by `trusted = false`,
`meter` by `fuel`, `timeout` by any of the three timeouts, `memory_limit` by the key of that
name, `reload` by `hot_reload`. **A guarantee has one name**, for the key that asks it and the
capability that answers: `timeout` was `deadline` at this seam and `memory_limit` was `limit`,
two words for one thing each, and a deadline is a moment where what the entry takes is a length
of time.
`side by side` is demanded by nothing: it is how Bridge-ASM700 learns which order a reload takes -
`lifecycle.md`, *The runtime lifecycle*. `unload` is demanded by nothing either: it is how
Bridge-ASM700 learns what to do with a backend's library at `bridge_destroy` - `create.md`,
*Destroy*. It says the library **may** go, and Bridge-ASM700 then closes it; whether it is then
unmapped is the operating system's. Windows and Linux unmap it. macOS does too, unless the image
has a thread-local variable: the native backend's had one and stayed, has none now and goes, and
`tests/stage4.c`, case `libraries`, asks the operating system on all three. So
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

The native backend answers no to `untrusted`, truthfully, and Bridge-ASM700 then refuses to load anything
marked untrusted onto it, before the library is opened. That makes the native backend a deliberate
choice for code you wrote rather than a hole you discover.

Without this the surface would let a backend silently stop enforcing the guarantees these
documents promise, which is the failure worth designing out.
