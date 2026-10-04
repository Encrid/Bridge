# Config: coming from the original Bridge

For a deployment whose config was written for the original Bridge: which keys changed, and which
are gone. What the keys mean is `config.md`.

## Keys that changed

    phases, [order]          new. schedule.md, *Phases*. phases is a list of names,
                             not tables: a phase has one property and it is its name
    [budget]                 new. budgets.md, *Budgets*. no default: stating it is
                             the deployment's job
    [backend]                new. the backends Bridge-ASM700 opens, by name and library.
                             an element names the one that runs it
    [element.defaults]       new. the deployment covering a manifest that left a
                             per-element key out
    read, write              new. grants.md, *Whose component it is*. Empty by default,
                             since an element may always declare in its own namespace
    trusted                  new. what a backend's `sandbox` flag is checked against
    max_system_failures      new. how many times a system may fail before its element
                             is stopped
    scratch, log_ring        new. the per-element sizes in memory 2
    pinned                   was thread. a manifest boolean, because only the author
                             knows. pinned.md, *Threads*
    [checks] validate,       new. the barrier's checksum pass and the per-system
      profile                counters, both off by default. the only two things
                             Bridge-ASM700 does only when asked, and in a table of
                             their own so no host switch can reach them
    log_level                moved to per element, because filtering is at the writer
                             against a threshold in that element's memory-2 header
    fuel, memory_limit       moved from per store to per element - a limiter knows who
                             is executing. each now states its period, limits.md,
                             *What bounds a run*: fuel per invocation, memory_limit
                             a ceiling, the three timeouts per call

## Keys that are gone

A key that no longer governs anything is worse than a missing one: it reads as a setting and
silently does nothing.

    tick_order              there is no tick; phases order things
    autoload                not Bridge-ASM700's: the host decides what to load
    call_timeout            there is no call between elements to time out. a system's
                            run is bounded by fuel, limits.md, *What bounds a run*
    restart_on_crash        automatic restart is implicit. the host calls
    max_restart_attempts    bridge_reload, and a stopped element stays stopped
    restart_delay
    [store]                 there is no store: the unit at the backend seam is the instance,
                            and grouping them is the backend's business. backend.md,
                            *What the backend does not do*
    [debug] field_names     nothing crosses to name fields in
    [debug] crossing table  it ranked module-to-module and plugin crossings, which are
                            the crossings we removed. The report is per system
