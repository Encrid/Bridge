# src/block/block_work.c

This file keeps the work list: the list of the modules that the next barrier must visit.
A barrier walks the work list, and not each loaded module: `docs/intern/block/block_barrier.c.md`, *The modules with work*.

## The list

`work` of the world is the first module of the work list, and `next_work` of a module is the next one.
`working` tells if a module is on it, so a module is on it one time at most.
The fields are in the record of the module, so the list needs no memory.

`block_note_work` puts a module at the head of the work list, and notes that the list is out of order.
It takes only a module that `linked` marks: a module that the world lists among its modules.
These callers note a module, each on the thread that calls the world:

| Caller | Why the barrier must visit the module |
| --- | --- |
| `account_job` of `src/schedule/schedule_run.c` | A system or an observer of the module ran. It can have queued changes, spawned ids, written log lines or failed. |
| `module_call` | A lifecycle call of the module ran, with the same results. |
| `abi_hear` | Something triggered an observer of the module, so its lists must be published. |
| `abi_park`, `abi_unpark` | The queries of the module left storage or came back, so its spans must be published. |

## A module that comes or goes

A module that loads or reloads runs its lifecycle call before the world lists it.
Its changes wait for the barrier after it joins the modules, as each listed module sees it.
So `block_join_work` gives a loaded module the next place, marks it `linked`, and notes it.
`block_take_place` gives the new module of a reload the place of the old one, in both lists: `docs/intern/bridge/bridge_reload.c.md`.
`block_drop_work` takes a module off the work list before `module_discard` frees it: `docs/intern/module/module_discard.c.md`.

## The list after a barrier

`block_keep_work` keeps each module that still has work for the next barrier, in the same order:

- a module with changes in its queue, which its observers queued at this barrier;
- a module that published triggers, since the next barrier must publish its lists empty: `docs/intern/storage/storage_publish.c.md`;
- a module whose ids are still short, since the next barrier tops them up again: `docs/intern/block/block_module.c.md`, *The ids set aside*.

It takes each other module off the work list.

## Functions

| Function | Result |
| --- | --- |
| `block_note_work` | Puts a listed module at the head of the work list, if it is not on it. |
| `block_keep_work` | Takes off the work list each module that has no work left. |
| `block_drop_work` | Takes a module off the work list. |
| `block_join_work` | Gives a loaded module the next place, marks it `linked`, and notes it. |
| `block_take_place` | Gives the new module of a reload the place of the old one, links it, and notes it. |
