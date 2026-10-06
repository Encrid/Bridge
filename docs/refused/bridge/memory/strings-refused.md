# What was refused: Variable length

The record of `strings.md`: `README.md`, *Three layers, three seams*.

## It is given back when nothing points into it

Of `strings.md`, *It is given back when nothing points into it*.

Refused: giving the share back always, and setting every text that pointed into it to empty. It is
one rule with no condition, and it has the library write into rows of a component the leaving module
did not own, where another module then reads nothing. The condition that brings it back is a string
that must never outlive the module that allocated it.

Refused: keeping a share without saying so, which is what this did. No condition brings it back.

Refused: never giving anything back, which was the rule. The condition that brings it back is the
reading at unload proving too slow for a deployment that unloads often.

Refused: `mark` and `reset` in the ABI. The condition is a module that must free strings in mid-life
and can show nobody else holds them.

Refused: giving back each string a row no longer holds, and not only whole shares, which is a
general allocator inside the arena. The condition is shares kept for one surviving string being seen
to pile up.
