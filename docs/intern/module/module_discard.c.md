# src/module/module_discard.c

This file frees the record of a module and what the record holds.
It also takes the systems and the observers of a module out of storage.

## The order

`module_discard` frees the parts of a module in this order:

1. Its systems and its observers.
2. Its instance.
3. Its image.
4. Its private memory, as its backend sees it.
5. The block of its private memory.
6. Its record.

The order is the order of the backend seam: a memory is released after the last instance that it was handed to.
An instance can hold the memory for as long as it runs, so the memory is never released before the instance goes.

`module_discard` calls no export of the module. Each caller runs the `shutdown` first, when the instance gets one: `docs/intern/module/module_shutdown.c.md`, *Functions*.

## The systems

`module_drop_systems` takes each system and observer of the module out of the list of the world, and keeps the order of the others.
First, `schedule_untrigger` takes its observers off the list of the triggered observers.
A system leaves storage with its terms.
A parked system also lowers the count of parked systems of the world.

## Functions

| Function | Result |
| --- | --- |
| `free_system` | Takes a system out of storage, lowers the count of parked systems if it was parked, and frees it. |
| `module_drop_systems` | Takes the systems and the observers of a module out of storage, and frees them. |
| `free_list` | Frees a list of texts. |
| `free_record` | Frees the record of a module and every list that it holds. |
| `module_discard` | Frees a module and what it held: its systems, its instance, its image, its private memory and its record. |
