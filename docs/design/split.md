# The split

Every capability belongs to exactly one side. This is the list; the seam documents describe their
own corner of it and point here.

    Bridge - the library      the host
    ------------------------------   ----------------------------------------
    storage: tables, columns, spans  reading files, and every format in them
    the barrier and the republish    its config, in whatever form
    the scheduler: phases, the       the manifest of a module, and reading it
      conflict graph, the refusal
    the module ABI, all eight        discovery: walking paths, deciding what
    the registry: components,          to load and when
      slots, grants, identities      the loop, and when the program ends
    the memories, arenas, rings      watching for changes, and asking for
    module lifecycle: running          a reload
      init, shutdown, reload         the log sink: what to do with records
    failure policy: on_error,        the compilation cache directory
      the failure budget, a          the journal file, if there is one
      deck that is stopped         the meaning of its own config keys
    the backend seam, and opening
      the backend libraries
    what it counts and checks when
      asked, and the two pulls
      that hand the answers back

**The rule behind it**: Bridge owns what no host could do differently. The host owns everything
that is a format, a filesystem, or a policy about time. A second host must be able to replace its
whole column without touching the library.

**The compilation cache** is a directory, so it is the host's: it hands a path to the backend
as one of its settings, and Bridge never opens it. **The journal** is the host's for the same
reason: Bridge never writes a file. It does not travel the log's path, though - a log may drop a
record and a journal may not: `schedule.md`, *What a pass depends on*.

Two more, each on its side because nobody else could hold it. **The counters and the checksum
pass** are Bridge's: `config.md`,
*The keys*. **The two pulls** that hand their answers back follow the state - only
Bridge has it, `introspection.md`, *Introspection*. And **the meaning of a host key** is the host's:
`config.md`, *The keys*, lists Bridge's, and everything else is the host's to define.
