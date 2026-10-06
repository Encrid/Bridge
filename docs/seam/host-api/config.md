# Config

What was weighed here and refused is `config-refused.md`, *What was refused: Config*.

Bridge-ASM700 reads no file. A host hands it values as structs - `host-api.md`, *The data* - and
where those values came from is the host's business: a file, a database, compiled-in constants. This
document is what Bridge-ASM700 requires of them.

Keys are spelled here, and throughout these documents, the way a TOML file would spell them -
`[budget] entities`, `[checks] validate` - because a key needs a name and that one is short.
Bridge-ASM700 sees only the struct field.

## No unaccountable values

**Every value is stated by somebody, and Bridge-ASM700 supplies none of its own.** A key the host
leaves unset is an error naming the key rather than a guess - at create for the deployment's keys,
at load for a module's. `budgets.md`, *Bridge-ASM700 defaults none of them*, for what that means for
a budget.

**Unset is zero.** Bridge-ASM700 sees a struct, so a numeric or enum field left at zero is the unset
one, and zero is never a legal value for it: a budget, `work_limit`, a timeout, `on_error`. `log_level` is
no exception, though its numbers are syslog's and zero is one of them: zero is unset there too,
and the least it can be stated as is 1 - `log.md`, *The log*. Where "none"
is a real answer it is a value of its own, never the field left alone, and there are two because
they say opposite things. A limit stated as none is `BRIDGE_NO_LIMIT`, -1: `work_limit`, a timeout,
`memory_limit`, `max_system_failures` - `capabilities.md`, *Capabilities, so the swap is honest*. A
reservation stated as none is `BRIDGE_NONE`, -2: `spawns`, `persistent`, `changes` - `budgets.md`,
*Budgets*. A
module with no limit may use all there is; one with no reservation gets nothing. Each key takes
its own, and the other, like any other negative number, is refused at load by the key's name.

**A key that is yes or no is stated as either, and left alone it is refused.** It is not a
boolean, which has no third state to be found in: it is `enum bridge_flag`, `BRIDGE_TRUE` or
`BRIDGE_FALSE`, and its zero is `BRIDGE_UNSET`, the one place the unset state has a name, since
here it sits beside the two answers. The keys are `trusted`, `reloadable`, `hot_reload` and
`pinned`, and `validate` and `profile` when they exist. `tests/stage2.c`, case `unsetflag`.

What that protects against is not a missing key but a value nobody can account for. A host may layer
its sources however it likes - a module author's stated values under a deployment's, say - as long
as what it hands over is one stated value per key. The original Bridge had the same rule.

## The keys

    the deployment's                           a module's
    ----------------------------------------   ------------------------------
    phases     an ordered list of names        trusted, reloadable, hot_reload
    threads    how many run a phase            on_error, max_system_failures
    [order]    before
    [budget]   entities, tables, types,        work_limit, memory_limit
               systems, terms, modules,        init/reload/shutdown timeouts
               persistent, columns,            log_level, scratch, log_ring,
               sparse_sets, direct_indexed     spans, spawns, persistent
    [checks]   validate, profile               pinned, config, losses
    [backend]  a name, a library and its
               settings, each
                                               read / write
                                               [.config]

Each becomes a field in `host-api.md`, *The data*, and Bridge-ASM700's behaviour turns on it.
Anything else a host configures - where modules are found, where records go, when a report is
printed - is the host's and has no key here: `split.md`, *The split*.

`[backend]` is Bridge-ASM700's because Bridge-ASM700 opens those libraries - `create.md`, *Create*.
What a backend is configured *with* is not: a cache directory, an instantiation policy, a tick
period. The host hands those over as that backend's settings and Bridge-ASM700 passes them through
unread.

`[checks]` is Bridge-ASM700's outright, since both run at a barrier and no host can reach one -
`validation.md`, *The checksum pass, when asked*. A host that put them behind a switch of its own
would be silently disabling a Bridge-ASM700 check.

`phases` is **an ordered array of names and nothing more** - the host walks it, so there is no
`after` relation to declare, and a phase has no other property to carry: `schedule.md`, *A phase is
a name and nothing else*. `[order]` is the only ordering key, and only for two conflicting systems
inside one phase.

## A module's own settings are a component

`[.config]` - the `settings` a host hands over for a module - **seeds a singleton component.** The
module declares the struct in its schema header like any other, and states it a singleton -
`storage.md`, *A singleton*. Bridge-ASM700 gives it its value at init, matching the keys to the
field names its descriptor states - `schema.md`, *There is no schema format*. A key the
deployment set that the module never declared is an error at init, by name, where the original
Bridge silently ignores a typo.

**Which component is the author's to say**, as `config`, a key of the manifest like `pinned`: only
the author knows which of a module's structs holds its settings, which is the reason a Linux module
marks its own parameters. A module with no settings component says that too: `config = none`, which
no component is called, since every component's name has a namespace. Left out, `config` is unset
and refused like any other key, so that an author who forgot it is told rather than taken to have
meant none. Settings handed to a module that stated none, or a `config` that `init` never declared,
are refused by name.

**The component `config` names is a singleton, or the load is refused by name.** It is one
value on no entity, so there cannot be a second, which a row on an entity could not promise: a
module that declared the component could attach it to another. The module reads it where it
is, through the cell its header has for each type it declared, and it goes by its `on_unload`
when the module does. And a component is the settings of one loaded module at a time.

**A setting is written by what its field is.** A bool into a bool, an integer into any integer
field it fits, a float into a float. Text has three places to go, and the field's type says which:

    char name[N]      the bytes, inline, and a zero after them. text that does not fit is refused
    bridge_text       `(offset, length)` into the persistent arena - `strings.md`, *Variable
                      length*. the pair is an ABI layout `bridge-abi.h` gives, usable as a field's type
    an enum           the constant of that name

That is `strings.md`'s own split, bounded text in the column and unbounded text in the arena, left
to the author to choose by declaring the field. Anything else - text into a float, a number too
large for its field, a name the enum lacks - is refused naming the key.

**After init the component is the live state.** Changing `vsync` while running is a component write
by whoever holds the grant - not a call, not a reload, not a call to reconfigure. Config is the initial
value; the component is the running state.

That is why nothing needs a mechanism for runtime settings: **the settings that change constantly in
a running program were never Bridge-ASM700's.** The ones that are Bridge-ASM700's are administrative
- a work limit, a log level - and change by reload, which is cheap because storage survives it.

Keeping a changed setting across a restart is between a native module, which can write a file, and
the host, which reads it back. Bridge-ASM700 never sees it.
