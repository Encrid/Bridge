# Config: what the rewrite changed

Against Bridge's config. What the keys mean now is `config.md`.

## What the rewrite changed

    phases, [order]          new. schedule.md, *Phases*. phases is a list of names,
                             not tables: a phase has one property and it is its name
    [budget]                 new. budgets.md, *Budgets*. no default: stating it is
                             the deployment's job
    [element.defaults]       new. the deployment covering a manifest that left a
                             per-element key out
    read, write              new. grants.md, *Whose component it is*. Empty by default,
                             since an element may always declare in its own namespace
    trusted                  new. what vm_capabilities' `sandbox` is checked against
    max_system_failures      new. how many times a system may fail before its element
                             is stopped
    scratch, log_ring        new. the per-element sizes in memory 2
    pinned                   was thread. a manifest boolean, because only the author
                             knows. plugin-threads.md, *Threads*
    [checks] validate,       new. the barrier's checksum pass and the per-system
      profile                counters, both off by default. the only two things
                             BridgeC does only when asked, and in a table of
                             their own so no host switch can reach them
    log_level                moved to per element, because filtering is at the writer
                             against a threshold in that element's memory-2 header
    fuel, memory_limit       moved from per store to per element - a limiter knows who
                             is executing. each now states its period, limits.md,
                             *What bounds a run*: fuel per invocation, memory_limit
                             a ceiling, the three timeouts per call

## What the rewrite killed

A key that no longer governs anything is worse than a missing one: it reads as a setting and
silently does nothing.

    tick_order              there is no tick; phases order things
    autoload                not BridgeC's: the host decides what to load
    call_timeout            there is no call between elements to time out. a system's
                            run is bounded by fuel, limits.md, *What bounds a run*
    restart_on_crash        automatic restart is implicit. the host calls
    max_restart_attempts    bridge_reload, and a stopped element stays stopped
    restart_delay
    [store]                 there is no store: the unit at the VM seam is the instance,
                            and grouping them is the backend's business. vm-surface.md,
                            *What the backend does not do*
    [debug] field_names     nothing crosses to name fields in
    [debug] crossing table  it ranked module-to-module and plugin crossings, which are
                            the crossings we removed. The report is per system
