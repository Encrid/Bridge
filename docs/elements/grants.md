# Whose component it is

A component name is namespaced - `mymod:Explosion`. **An element may always declare a component in
its own namespace, and reaching another namespace's needs a grant from the deployment.** Refused at
init, by name: one check per declaration and nothing per frame.

    [element."mods:bigger-explosions"]
    write = ["game:Transform"]
    read  = ["game:Health"]

This is not a defence against mods. A mod writing `game:PlayerHealth` may be exactly what its author
intends and what the game means to allow. It is the game deciding **what its mod API is**, instead
of that defaulting to every component in the deployment - which is what global identity gives you
without it. A mod's own components need no config at all; touching the game needs one line, at the
point where somebody should be asked.

It composes with `schedule.md`, *Order within a phase is never implicit*: grant two elements write on
`game:Transform` and their systems conflict, so Bridge-ASM700 refuses to start until the deployment orders
them.
