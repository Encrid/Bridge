# IdeaTek

The standalone binary. It links BridgeC and does the three things a library cannot do for itself:
own `main`, own the loop, and decide when to stop. Why it is a separate target is `README.md`.

## What it is

    main()        argv, signals
    config        read config.toml and config.local.toml, parse TOML
    layers        resolve manifest, deployment, local - with bridge_manifest_parse
    discovery     walk [content] paths, bridge_load per element
    the loop      walk the phases config lists, bridge_phase each
    the log       the sink BridgeC emits into, written to terminal or file
    --once        one pass, then exit
    --watch       watch the deployment's paths, bridge_reload on change
    --list        load the deployment, report each result, exit

**Three of those a library cannot do and the rest it declines**, which are different reasons.
`main`, the loop and the exit are impossible for a library - something has to own them. TOML, the
layer resolution, the directory walk and the log destination are all things BridgeC could have done
and does not, and `split.md`, *The split*, is the rule that sends each of them this way.

Thinner than idTech3's shell, which also polls input at `win_main.c:1240`. Input is a plugin here,
so IdeaTek creates, loops, destroys.

## Its own keys

`config.md` says which tables are IdeaTek's; what is inside one is decided here, and a host that
brings its own config system has none of them.

    [content]   paths    files and directories. a directory is walked; a file
                         named directly is taken regardless of ignore
                ignore   what to skip - a path, or a gitignore-shaped pattern
    [cache]     directory, handed to the backend
    [debug]     enabled, profile, profile_every_secs, profile_top
    [pooling]   enabled, and the instantiation limits behind it

**There is no `depth`.** Bridge had one - `-1` unlimited, `0` the root, `N` levels - and the only
deployment that ever used it set `-1` and did all its pruning with `ignore`. A second pruning
mechanism nobody reaches for, carrying a magic value for "off". One mechanism: walk, and prune by
pattern. A symlink loop is the walker refusing to revisit a directory it has already seen, which is
walker behaviour and not something a deployment should have to configure.

**`enabled` is a master switch, and setting a key under a disabled table is an error.** Bridge had
both conventions and they disagreed: `[debug]`'s keys were *"ignored unless enabled = true"* while
`[pooling]`'s *"may not be set"* when off. Ignoring a value somebody wrote is the implicit failure -
they changed a setting, nothing happened, and nothing said why. So the erroring rule, for every
optional table here.

## The pass is config's order

IdeaTek is generic: it cannot know a deployment's phases, and BridgeC has none to fall back on -
`schedule.md`, *BridgeC ships no phase names*. So it reads them from config and walks them in order.
TOML arrays keep their order, so **the `phases` list is the pass** - and a phase carries nothing but
its name, so there is nothing else here to read: `schedule.md`, *A phase is a name and nothing else*.

That is why having no `after` relation costs nothing in practice. `host-api.md` records the cost - a
host can call one phase every frame and another never, and BridgeC cannot detect it - and IdeaTek
cannot make that mistake, because it walks the list rather than naming phases. A host that wants a
fixed timestep calls `bridge_phase` itself and takes the responsibility with it.

## --list is a load check, not an inventory

It loads everything the deployment declares, prints what each `bridge_load` answered, and exits. The
information is **IdeaTek's own return codes**, not `bridge_elements`.

That call answers who is *still running* - `introspection.md`, *Introspection* - and nothing has been
stopped before the loop has run, so at startup it could only ever say "all of them". What a reader
of `--list` wants is whether this deployment loads at all, and only the shell can answer that,
because only the shell walked the paths.

**So IdeaTek never calls `bridge_elements`.** Its consumer is a host with something to show or
something to decide - a game greying out a stopped mod, a supervisor restarting one. A generic shell
that loops until a signal has nothing to do with the answer, and gets the diagnostic from the sink
anyway.

## When it stops

BridgeC has no notion of being done, so this is entirely IdeaTek's: a signal, `--once`, or a
deployment that asked for neither and runs until killed.

Nothing an element does ends the process. That is deliberate - `loop.md`, *BridgeC has no notion
of being done* - and it means the exit condition is always visible in the command line rather than
buried in a module.

## --watch is here, not in a plugin

Watching the filesystem is a capability, and capabilities are plugins - but **a plugin cannot call
`bridge_reload`**, which is host API. So the watcher lives here.

That is not a loophole in the no-builtins rule: the rule constrains what BridgeC exposes to
elements, not what a program linking it may do. And it gives hot reload the property that matters -
**ship without `--watch` and nothing in the process can ask for one.**
