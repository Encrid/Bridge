# Threads

A plugin's systems run on whichever thread BridgeC schedules its element on, and different elements
run at once - `threads.md`, *Threads*. For most plugins that is the end of it.

For the rest it is not, and the case is not theoretical: **Godot must be driven from the thread that
created it.** So a plugin says so, with one key in its manifest:

    pinned = true     every system and observer of this element runs on the thread
                      bridge_init ran on

**It is the author's key, not the deployment's**, which is the *who holds the knowledge* rule
`config.md` is ordered by. A plugin author knows their library must be driven from one thread; a
deployment author has no access to that fact, so declaring it there would be the deployment
guessing, and a wrong guess is a race nobody will find quickly. It reaches BridgeC as a field in
`bridge_element_config`, like every other manifest value.

**The cost is that pinned elements serialise with each other.** They are all on one thread by
construction. That is not a flaw - elements all driving one thing that is not thread-safe **must**
serialise - and it is visible at load rather than as a race.

**Nothing is proxied and nothing is deferred.** Godot's answer to a wrong-thread call is
`call_deferred`, a queue it drains itself. Ours is that the wrong thread never happens, because the
element was placed on the right one before it ran. There is no call to proxy in the first place.

A plugin appends to a log ring of its own, exactly as a module does, through a real pointer rather
than a wasm memory - `log.md`, *The log*. Not a direct call into BridgeC, even though one
would cost two nanoseconds here: a plugin may be running on a thread of its own, and a ring each is
what keeps every writer single and lock-free.
