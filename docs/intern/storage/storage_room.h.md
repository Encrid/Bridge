# src/storage/storage_room.h

This header finds room for each row that a barrier adds to a table, before anything moves.
`reserve_all` of `src/storage/storage_apply.h` uses it: `docs/intern/storage/storage_apply.h.md`, *Functions*.

## Running out

A barrier knows what each changed entity will carry before it moves a row: `docs/intern/storage/storage_apply.c.md`, *The steps of a barrier*.
So here it finds a shortage before any row moves: a table past `[budget] tables`, or rows past `[budget] columns`.
Then storage drops all the changes asked of that entity at that barrier, and not only some of them.
The entity keeps the components and the values it had, and no observer hears of it.
Storage places the other entities as usual.
A destroy that leaves a husk can be dropped too, since the husk moves to the table of what it keeps.

When a table cannot take all the rows that arrive, it takes as many as it finds room for.
It asks the column region again for `table_retry` percent of the rows of its last ask, until a number fits: `get_retry`.
Then it searches back up, between that number and its first ask:

```pseudo
PROCEDURE size_table(t : U32)
   most <- tables[t].incoming
   WHILE tables[t].incoming > 0 AND NOT grow_table(t, tables[t].rows + tables[t].incoming)
      tables[t].incoming <- get_retry(tables[t].incoming)
   ENDWHILE
   WHILE most - tables[t].incoming > 1
      between <- tables[t].incoming + (most - tables[t].incoming) DIV 2
      IF grow_table(t, tables[t].rows + between) THEN
         tables[t].incoming <- between
      ELSE
         most <- between
      ENDIF
   ENDWHILE
   tables[t].sized <- TRUE
ENDPROCEDURE
```

With room for 4 rows of 6 and a `table_retry` of 50, the search places 4, where the asks alone place 3.
The search halves its range at each step. That is a bisection, the strategy of the search, so it is no key.
`table_retry` sets only the asks before it: `docs/extern/bridge.h.md`, *The tables of storage*.

Storage takes room for the entities in the order of their first change, which is the order the calls ran.
So the first entities asked are the ones placed, and the same ones on each run.

An entity left as it was is a failure of each module that asked a change of it, each for its own changes.
`write_value` of `src/storage/storage_runs.h` counts them, and `say_unapplied` of `src/storage/storage_settle.h` says them.
The barrier then counts against the budget of the module: `docs/intern/schedule/schedule.h.md`, *The failure budget*.
Two modules that changed one entity both hear of it, since both lost what they asked.

Nothing comes from outside the column region.
Storage takes no column from the operating system, and it does not stop the world for a shortage.
It names who asked, and nothing has begun when it finds the shortage, so nothing needs to be undone.

## Functions

| Function | Result |
| --- | --- |
| `get_retry` | Gives the rows of the next ask, after an ask of `rows` did not fit: `table_retry` percent of them, in 64 bits. Below 100 percent, each ask is smaller, so the asks end at 0. |
| `size_table` | Finds room in table `t` for all the rows it gains at this barrier, or for as many as the column region holds. Sets `incoming` to that number. |
| `make_room` | Takes room for one more row that table `t` gains. The first time, it sizes the table. Gives false if no room is left. |
| `room_joins` | For the entity at `index`, takes room in each sparse set that it joins. Gives false at the first set with no room. |
| `room_one` | Takes room for the entity at `index` where it is to be, or leaves it as it was for want of columns. An entity that gets no new row needs no room. Most of what a barrier sees is an attach to an entity that has the component already. |
