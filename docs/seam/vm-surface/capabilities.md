# Capabilities, so the swap is honest

`vm_capabilities` is not decoration. A backend says what it can actually do:

    sandbox        the guarantee in element.md, *Three guarantees differ*
    meter          fuel, or an equivalent
    deadline       a running module can be interrupted
    reload         an instance can be replaced while the process runs

`sandbox` is about the **host boundary and nothing else**. It does not mean one module cannot
corrupt another's components: they share a memory by design - see `costs.md`, *What we give up*.
idTech3 drew the same line, `VMI_NATIVE` against `VMI_BYTECODE` per element with a cvar each and
shipped set to the safe one. It got module-against-module isolation only because its three VMs had
three blocks and shared no data.

Each flag is demanded by a **config key**, not by an element: `sandbox` by `trusted = false`,
`meter` by `fuel`, `deadline` by a timeout, `reload` by `hot_reload`. So a mismatch is not an
element arguing with a backend - it is **config contradicting config**, since the deployment chose
both. It is therefore one of BridgeC's own refusals and takes init's path under `on_error`, exactly
as a denied grant or an ambiguous order does: `failure.md`, *When something fails*.

## Nothing is downgraded

idTech3 does - asked for native with no DLL present, `vm.c:486` prints a line and falls back to
bytecode, and `fs_restrict` forces the safe path regardless of what was asked. It is sound there
because the two paths are two builds of one module with identical semantics. It is still
**implicit**, and a deployment that asked for a guarantee should be told it cannot have one rather
than quietly given something else.

The override is explicit instead: a developer who wants untrusted code on a native backend writes
`trusted = true` - a statement in a file somebody can read, rather than a warning nobody does.

A native backend answers no to `sandbox`, truthfully, and BridgeC then refuses to load anything
marked untrusted onto it. That makes a native backend a deliberate choice for code you wrote rather
than a hole you discover.

Without this the surface would let a backend silently stop enforcing the guarantees the documents
above promise, which is the failure worth designing out.
