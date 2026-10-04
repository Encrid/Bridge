# The split

Every capability belongs to exactly one side. This is the list; the seam documents describe their
own corner of it and point here.

    Bridge-ASM700 - the library      the host - IdeaTek, or an embedder
    ------------------------------   ----------------------------------------
    storage: flecs, columns, spans   reading files, and every format in them
    the barrier and the republish    TOML, and the three config layers
    the scheduler: phases, the       the manifest, via bridge_manifest_parse
      conflict graph, the refusal      (a helper beside the library)
    the element ABI, all eight       discovery: walking paths, deciding what
    the registry: components,          to load and when
      slots, grants, identities      the loop, and when the program ends
    the memories, arenas, rings      watching for changes, and asking for
    element lifecycle: running         a reload
      init, shutdown, reload         the log sink: what to do with records
    failure policy: on_error,        the compilation cache directory
      the failure budget, the        the journal file, if there is one
      escalation guards              the meaning of its own config keys -
    the backend seam, and opening      [content], [debug], [cache], [pooling]
      the backend libraries
    the flecs fork: the patch, the
      pin, and carrying both
    what it counts and checks when
      asked, and the two pulls
      that hand the answers back

**The rule behind it**: Bridge-ASM700 owns what no host could do differently. The host owns everything
that is a format, a filesystem, or a policy about time. A second host must be able to replace its
whole column without touching the library.

**The compilation cache** is a directory, so it is the host's: it hands a path to the backend
through `backend.md` and Bridge-ASM700 never opens it. **The journal** is the host's for the same
reason: Bridge-ASM700 never writes a file. It does not travel the log's path, though - a log may drop a
record and a journal may not: `schedule.md`, *What a pass depends on*.

Three more, each on its side because nobody else could hold it. **The flecs fork** is Bridge-ASM700's:
`flecs.md`, *How it is carried*. **The counters and the checksum pass** are Bridge-ASM700's: `config.md`,
*One file, two consumers*. **The two pulls** that hand their answers back follow the state - only
Bridge-ASM700 has it, `introspection.md`, *Introspection*. And **the meaning of a host key** is the host's:
`config.md` says which tables are whose, and `ideatek.md`, *Its own keys*, settles what is inside
one.
