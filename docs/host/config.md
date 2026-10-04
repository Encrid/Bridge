# Config

Three files, because three parties know three different things. The host reads all of them; Bridge-ASM700
reads none - it is handed the result as data, `host-api.md`.

## The three layers

**1. The element's manifest** - the author's **stated values**, travelling with the artifact. Only
the author knows how much scratch space their code uses, how long its init takes, or what log level
is useful while developing it, and the answer is the same in every deployment. Not *defaults*: a
default is a value nobody stated, which is the thing this chain refuses.

**2. `config.toml`** - the deployment's, overriding the author. It knows what the author cannot: who
else is here, who is trusted, what is granted, how big the shared block is, what the phases are. A
mod's suggested `fuel` is a suggestion.

**3. `config.local.toml`** - written while running, read at boot, winning over both. It knows what
neither could: the player turned vsync off, somebody unloaded an element by hand. It has to outlive
the process, so it is a file rather than memory.

**Nothing is defaulted below layer 1.** Bridge-ASM700 supplies no value of its own, so a key omitted by
all three is an error rather than a guess - *No unaccountable values*, and `budgets.md`, *Bridge-ASM700
defaults none of them*, for what that means for a budget.

Each layer knows strictly more about the situation than the one below, which is why they are ordered
that way. `pinned` is the clearest case of a key the argument places in layer 1 and nowhere else -
`pinned.md`, *Threads*.

## No unaccountable values

The original Bridge's rule, restated for three layers: **every key is stated somewhere in the
chain, and you can always say which layer it came from.** Omitting one everywhere is an error rather
than a guess.

What that protects against is not a missing key but a value nobody can account for.

## One file, two consumers

`config.toml` mixes the host's settings and Bridge-ASM700's, which is natural - it is one deployment. The
host reads it, acts on its own keys, and resolves the rest into the structs it hands over. Which
capability belongs to which side is `split.md`, *The split*; this is only where the keys live.

    IdeaTek's                        Bridge-ASM700's
    ------------------------------   -------------------------------------------
    [content]                        phases     an ordered list of names
    [cache]                          [order]    before
    [debug]                          [budget]   entities, persistent, max_elements
    [pooling]                        [checks]   validate, profile
    the log destination              [backend]  one name and one library each
    what to load at boot             [element.defaults] any per-element key
    where config.local.toml is       per element
                                       trusted, hot_reload, on_error
                                       max_system_failures
                                       fuel, memory_limit
                                       init/reload/shutdown timeouts
                                       log_level, scratch, log_ring
                                       read / write
                                       [.config]

**The two columns are listed differently on purpose.** Bridge-ASM700's are named key by key, because each
becomes a field in `host-api.md`, *The data*, and Bridge-ASM700's behaviour turns on it. IdeaTek's are
named by **table only**, because what is inside one is IdeaTek's to decide and to document -
`ideatek.md`, *Its own keys*. A host with its own config system has none of these tables at all and
is not wrong: `host-api.md`, *Bridge-ASM700 parses nothing*.

`[backend]` is Bridge-ASM700's because Bridge-ASM700 opens those libraries - `create.md`, *Create*. What a
backend is configured *with* is not: `[cache]` is the host's because it is a directory, and
`[pooling]` because it is a wasmtime instantiation policy. The host hands both to the backend as its
settings and Bridge-ASM700 passes them through unread. `[debug]` is the host's because it decides when to
print a report, though the counters behind one are Bridge-ASM700's.

**No table is split between the two**, and `validate` is why that rule needs stating. It is
Bridge-ASM700's outright, since the pass runs at a barrier and no host can reach one - `validation.md`,
*The checksum pass, when asked*. A Bridge-ASM700 key under `[debug]`, whose master switch is the host's,
would have the host silently disabling a Bridge-ASM700 check. So it has its own table.

`phases` is **an ordered array of names and nothing more** - IdeaTek walks it, so there is no
`after` relation to declare, and a phase has no other property to carry: `schedule.md`, *A phase is a
name and nothing else*. `[order]` is the only ordering key, and only for two conflicting systems
inside one phase.

## An element's own settings are a component

`[.config]` **seeds a singleton component.** The element declares the struct in its schema header
like any other. Bridge-ASM700 writes one row at init, matching the keys to field names from the reflection
it registered - `schema.md`, *There is no schema format*. A key the deployment set that the element
never declared is an error at init, by name, where the original Bridge silently ignores a typo. A string is
`(offset, length)` into the persistent arena, as in any component holding text.

**After init the component is the live state.** Changing `vsync` while running is a component write
by whoever holds the grant - not a call, not a reload, not a reconfigure entry. Config is the initial
value; the component is the running state.

That is why nothing needs a mechanism for runtime settings: **the settings that change constantly in
a running program were never Bridge-ASM700's.** The ones that are Bridge-ASM700's are administrative
- a fuel budget, a log level - and change by reload, which is cheap because storage survives it.

## Persistence is the deployment's

The player turns vsync off, the component changes, and it should survive a restart. A **native element**
reads the component and writes whatever file it likes; the host reads that back on the next boot as
its local layer.

So `config.local.toml` is a convention between a native element and a host. Bridge-ASM700 never sees it.
