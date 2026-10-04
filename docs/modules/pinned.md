# Threads

A module's systems run on whichever thread Bridge-ASM700 schedules it on, and different modules run at
once - `threads.md`, *Threads*. For most modules that is the end of it.

For the rest it is not, and the case is not theoretical: **Godot must be driven from the thread that
created it.** So a module says so, with one key in its manifest:

    pinned = true     every system and observer of this module runs on the thread
                      its init ran on

**It is the author's key, not the deployment's**, because a key belongs to whoever holds the
knowledge. An author knows their library must be driven from one thread; a
deployment author has no access to that fact, so declaring it there would be the deployment
guessing, and a wrong guess is a race nobody will find quickly. It travels in the manifest -
`module.md`, *The manifest* - and reaches Bridge-ASM700 as a field in `bridge_module_config`.

The key is not the native backend's: it is a module's, on any backend. Native modules are the
ones with a reason to set it, because they are the ones that hold something outside storage.

**The cost is that pinned modules serialise with each other.** They are all on one thread by
construction. That is not a flaw - modules all driving one thing that is not thread-safe **must**
serialise - and it is visible at load rather than as a race.

**Nothing is proxied and nothing is deferred.** Godot's answer to a wrong-thread call is
`call_deferred`, a queue it drains itself. Ours is that the wrong thread never happens, because the
module was placed on the right one before it ran. There is no call to proxy in the first place.
