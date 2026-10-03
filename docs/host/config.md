# Config

**Draft.** Open items are tracked in OPEN.md; nothing here is settled until that register is empty.

Three files, because three parties know three different things. The host reads all of them; BridgeC
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
neither could: the player turned vsync off, somebody unloaded a module by hand. It has to outlive the
process, so it is a file rather than memory.

**Nothing is defaulted below layer 1.** BridgeC supplies no value of its own, so a key omitted by
all three is an error rather than a guess - *No unaccountable values*, and `memory.md`, *BridgeC
defaults none of them*, for what that means for a budget.

Each layer knows strictly more about the situation than the one below, which is why they are ordered
that way. `pinned` is the clearest case of a key the argument places in layer 1 and nowhere else -
`plugin-abi.md`, *Threads*.

## No unaccountable values

Carried forward from Bridge, restated for three layers: **every key is stated somewhere in the chain,
and you can always say which layer it came from.** Omitting one everywhere is an error rather than a
guess.

What that protects against is not a missing key but a value nobody can account for.

## One file, two consumers

`config.toml` mixes the host's settings and BridgeC's, which is natural - it is one deployment. The
host reads it, acts on its own keys, and resolves the rest into the structs it hands over. Which
capability belongs to which side is `DESIGN.md`, *The split*; this is only where the keys live.

    IdeaTek's                        BridgeC's
    ------------------------------   -------------------------------------------
    [content]                        phases     an ordered list of names
    [cache]                          [order]    before
    [debug]                          [budget]   entities, persistent, max_elements
    [pooling]                        [checks]   validate
    the log destination              [element.defaults] any per-element key
    what to load at boot
    where config.local.toml is       per element
                                       trusted, hot_reload, on_error
                                       max_system_failures
                                       fuel, memory_limit
                                       init/reload/shutdown timeouts
                                       log_level, scratch, log_ring
                                       read / write
                                       [.config]

**The two columns are listed differently on purpose.** BridgeC's are named key by key, because each
becomes a field in `host-api.md`, *The data*, and BridgeC's behaviour turns on it. IdeaTek's are
named by **table only**, because what is inside one is IdeaTek's to decide and to document -
`ideatek.md`, *Its own keys*. A host with its own config system has none of these tables at all and
is not wrong: `host-api.md`, *BridgeC parses nothing*.

`[cache]` is the host's because it is a directory: it hands the path to the backend and BridgeC never
opens it. `[pooling]` is the host's because it is a wasmtime instantiation policy the backend
applies. `[debug]` is the host's because it decides when to print a report, though the counters
behind one are BridgeC's.

**No table is split between the two**, and `validate` is why that rule needed stating: it sat under
`[debug]` and is BridgeC's outright, since the pass runs at a barrier and no host can reach one -
`diagnostics.md`, *The checksum pass, when asked*. A BridgeC key under a table whose master switch is the
host's would have the host silently disabling a BridgeC check, so it has its own table.

`phases` is **an ordered array of names and nothing more** - IdeaTek walks it, so there is no `after`
relation to declare, and a phase has no other property to carry: `bridge.md`, *A phase is a name and
nothing else*. `[order]` is the only ordering key, and only for two conflicting systems inside one
phase.

## An element's own settings are a component

`[.config]` **seeds a singleton component.** The element declares the struct in its schema header like
any other; BridgeC writes one row at init, matching the keys to field names from the reflection it
registered - `schema.md`, *Why a header and not a format*. A key the deployment set that the element
never declared is an error at init, by name, where today a typo is silently ignored. A string is
`(offset, length)` into the persistent arena, as in any component holding text.

**After init the component is the live state.** Changing `vsync` while running is a component write by
whoever holds the grant - not a call, not a reload, not a reconfigure entry. Config is the initial
value; the component is the running state.

That is why nothing needs a mechanism for runtime settings: **the settings that change constantly in a
running program were never BridgeC's.** The ones that are BridgeC's are administrative - a fuel
budget, a log level - and change by reload, which is cheap because storage survives it.

## Persistence is the deployment's

The player turns vsync off, the component changes, and it should survive a restart. A **plugin** reads
the component and writes whatever file it likes; the host reads that back on the next boot as its
local layer.

So `config.local.toml` is a convention between a plugin and a host. BridgeC never sees it.

## What the rewrite changed

    phases, [order]          new. bridge.md, *Phases*. phases is a list of names,
                             not tables: a phase has one property and it is its name
    [budget]                 new. memory.md, *Budgets*. no default: stating it is
                             the deployment's job
    [element.defaults]       new. the deployment covering a manifest that left a
                             per-element key out
    read, write              new. element.md, *Whose component it is*. Empty by default,
                             since an element may always declare in its own namespace
    trusted                  new. what vm_capabilities' `sandbox` is checked against
    max_system_failures      new. how many times a system may fail before its element
                             is stopped
    scratch, log_ring        new. the per-element sizes in memory 2
    pinned                   was thread. a manifest boolean, because only the author
                             knows. plugin-abi.md, *Threads*
    [checks] validate        new. the barrier's checksum pass, off by default. the
                             only optional check BridgeC has, and in a table of
                             its own so no host switch can reach it
    log_level                moved to per element, because filtering is at the writer
                             against a threshold in that element's memory-2 header
    fuel, memory_limit       moved from per store to per element - a limiter knows who
                             is executing. each now states its period, bridge.md,
                             *What bounds a run*: fuel per invocation, memory_limit
                             a ceiling, the three timeouts per call

## What the rewrite killed

A key that no longer governs anything is worse than a missing one: it reads as a setting and silently
does nothing.

    tick_order              there is no tick; phases order things
    autoload                not BridgeC's: the host decides what to load
    call_timeout            there is no call between elements to time out. a system's
                            run is bounded by fuel, bridge.md, *What bounds a run*
    restart_on_crash        automatic restart is implicit. the host calls
    max_restart_attempts    bridge_reload, and a stopped element stays stopped
    restart_delay
    [store]                 there is no store: the unit at the VM seam is the instance,
                            and grouping them is the backend's business. vm-surface.md,
                            *What the backend does not do*
    [debug] field_names     nothing crosses to name fields in
    [debug] crossing table  it ranked module-to-module and plugin crossings, which are
                            the crossings we removed. The report is per system
