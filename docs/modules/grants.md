# Whose component it is

What was weighed here and refused is `grants_refused.md`, *What was refused: Whose component it is*.

A component name is namespaced - `mymod:Explosion`. **A module may always declare a component in
its own namespace, and reaching another namespace's needs a grant from the deployment.** Refused at
init, by name: one check per declaration and nothing per frame.

    [module."mods:bigger-explosions"]
    write = ["game:Transform"]
    read  = ["game:Health"]

This is not a defence against mods. A mod writing `game:PlayerHealth` may be exactly what its author
intends and what the game means to allow. It is the game deciding **what its mod API is**, instead
of that defaulting to every component in the deployment - which is what global identity gives you
without it. A mod's own components need no config at all; touching the game needs one line, at the
point where somebody should be asked.

It composes with `schedule.md`, *Order within a phase is never implicit*: grant two modules write on
`game:Transform` and their systems conflict, so Bridge refuses to load the second until the
deployment orders them.

## What a module may destroy

**An entity it has a stake in: one that carries at least one component it may write, or one that
carries nothing.** `destroy` takes every component on the entity with it, whoever declared them,
so it is a write to all of them, and a module that may write none of them is reaching another
namespace's with no grant. A module with a read grant on `game:Position` can find every entity
of the game in its spans; it may not remove them.

It is checked at the barrier, where what the entity carries is known, against what it carried before
that barrier. A destroy that fails it leaves the entity as it was, the log says how many, and it is
the module's failure, counted as a change that could not be placed is - `failure.md`, *The budget*.
`tests/host`, cases `destroy-with-a-read-grant-left` and `module_config.write-destroy-goes`, are one
module under the two grants.

This is the sentence `lifetime.md`, *What unload leaves behind*, already had - a module may
destroy an entity "it only partly populates" - enforced where it had been wider: any module could
destroy any entity.