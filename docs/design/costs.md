# What we give up

Against the original Bridge, which is the comparison point because this replaces it.

    wit-bindgen          a header both sides compile, plus a small file of accessors
    resource types       handles by hand, as godot:host/handles already does
    wac composition      the statically-linked call path
    string, list args    passed as (offset, len) where genuinely needed
    nine config keys     tick_order, autoload, call_timeout, the three restart
                         keys, [store], field_names and the profile's crossing
                         table
    column isolation     a module can write outside the columns it declared
    a synchronous answer no module can ask another a question and wait for the reply

**The last one is what dropping calls costs**, and it is a real loss. An engine callback that
consumes a return value inside its own loop cannot be served from storage: `Array.sort_custom` wants
a bool per comparison, now. So `Callable` round trips and `scripts.attach` behaviour lose their
synchronous half. The two probe checks that assert it - *"a Callable the adapter owns reaches a
module and answers"* and *"a plain Node3D answers get_speed, because a module is behind it"* - are
the two the Godot adapter does not get back. Everything an engine asks that is **state** rather than a
question is served from storage with no crossing: `engine-callbacks.md`, *An engine callback answers from
storage*.

Column isolation is the ECS bargain, and it is **partly** recovered. Read and write declarations
exist so the scheduler can keep two writers apart, and they are not policed. Why no arrangement of
memories could express it instead: `wasm.md`, *Memories*. A module that writes a component
it declared read is a bug in that module, and `validation.md`, *What a bad write costs, and what
finds it*, is how it gets a name.

What is recovered is everything that is not a shared column. A wasm module's spans, its `column_base`,
its scratch arena and its log ring sit in a private memory no other module imports, so the `base`
and `count` its arithmetic reads cannot be corrupted from outside - `memory.md`, *Where the seven
regions live*. What stays exposed is the columns and the entity index. One thing would change that,
and we decline it: reserve address space per component and map only a module's own columns into its
memory, which `wasmtime_config_host_memory_creator_set` makes possible. What it costs can be
counted. A mapping is made of whole pages, and on Windows a view begins on a boundary of 64 KB,
where a table today is one packed block with its columns at multiples of eight bytes: every column
of every table would need pages of its own, which for twenty thousand tables is gigabytes of
address space around megabytes of data. A column that grows or moves would be unmapped and mapped
again in every module that sees it, by system calls, at a barrier that otherwise costs tens of
nanoseconds a change. It is one more rule for each storage layout, it helps only the wasm
backend, and it is not the one block a backend adopts - `backend.md`, *Memory is adopted, not
created*. It was first refused for the entries one page mapped N times takes in the TLB; that was
never measured, and is not the reason.

**The condition that would change the answer: Bridge-ASM700 being asked to run code the operator did not
choose.** Modding is not that - a player installing a mod chose it - and no engine we looked at
isolates mods from each other. Minecraft has the largest modding ecosystem there is with none at
all. What that ecosystem suffers is unattributable crashes and mod conflicts, which attribution and
the grants address instead.

This is a regression against the original Bridge, where separate component memories made it
impossible. The ECS is what buys it.

The boundary that matters is untouched either way - `data.md`, *Access mode*.

Kept: sandboxing from the host, traps caught, fuel, lifecycle timeouts, memory limits, hot reload,
the manifest section, threading.

Hot reload improves in one direction and gains a refusal. In the original Bridge a reloaded module loses
its state; with storage in Bridge-ASM700's region **the data survives the swap**, which is what idTech3's
`VM_Restart` was for too. A layout change is caught by the fingerprint and refused by name rather
than silently reinterpreted, and the refusal cannot be waived: starting a module over with its
data gone is `unload` then `load`. See `lifetime.md`, *Lifetime*.
