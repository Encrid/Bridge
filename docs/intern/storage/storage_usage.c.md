# src/storage/storage_usage.c

This file reports how many entities and tables a world holds now, and the most it has held, for `bridge_usage`.
It also tells the publish whether the columns of a table moved.

## What it counts

The entity index grows to `high` and never shrinks: a freed slot goes on a list and comes back before `high` rises.
So the peak entities is `high` less the one slot that is no entity. The live entities is that peak less the slots on the free list.
A husk is on no free list until it goes, so it counts among the live entities.
Tables keep a live count and a watermark of their own, so this file reads both without a walk.

## Functions

| Function | Result |
| --- | --- |
| `free_entities` | Walks the free list of the index and counts the slots on it. |
| `storage_usage` | Gives a world's entities and tables, each as a live count and a peak. |
| `storage_is_reshaped` | Gives whether a touched table has `reshaped` set: its columns moved since the last publish. |
