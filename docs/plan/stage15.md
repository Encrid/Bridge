# Stage 15: Bridge

In progress. `plan.md`, stage 15, is what it is for and when it is done. This document is the
order it is done in, what was counted before it began, and each decision as it is taken. What a
decision refused is `stage15_refused.md`.

No identifier changes: the code says `bridge.h`, `bridge_create` and `BRIDGE_ABI` already. What
is renamed is prose, the header every file of ours begins with, and what holds the tree from
outside.

## The order

    1  the stage         this document, with what was counted
    2  the decisions     each by the method of `stage0.md`, one at a time, before a file changes
    3  the name          every "Bridge-ASM700" in the tree, by the decisions; the header of
                         every file, and the expression of `tools/style.py` that holds it
    4  the original      one name for the original Bridge wherever it is meant
    5  the check         a script fails on the old name anywhere in the tree
    6  outside the tree  four actions, in this order, each confirmed before it is made: the
                         original's repository renamed, with its remote; the original's
                         folder; this repository renamed, with its remote; this folder, last
    7  close             by the done-when

## What there was, before any of it

Counted on `e635bf0`, the tree stage 14 left.

    "Bridge-ASM700"        823 times in 543 tracked files, and in no other spelling
    the file header        468 files of ours begin "This file is part of Bridge-ASM700.", and
                           `tools/style.py` holds that line by an expression
    by folder              `src` 153 files, the tests about 240, the backends 30, the
                           documents about 60, `utils` 12, the tools and `build.py` a handful
    in the records         55 times under `plan/` and `refused/`: what was decided, and when,
                           under the old name
    "the original Bridge"  33 times in 22 files, eight of them a possessive; once "the
                           original's"; and once a bare "Bridge" that means it, in `config.md`
    "Bridge" alone         as a name of ours, only the heading of this stage in `plan.md`
    "the bridge"           in lower case, the object a host creates, throughout
    the repository         `Heuretes-Workshop/Bridge-ASM700`
    the original's         `Heuretes-Workshop/Bridge`, in the same organisation: the name is
                           taken, and is freed by renaming that one first
    on this machine        the folder `Bridge-ASM700`, and beside it `Bridge`, which is the
                           original
    who names the          two files of one other project, by path or by repository
    original

## The decisions

Six. The fourth was taken while the stage was planned.

    1  what the original Bridge is called from now on, in the documents
    2  "Bridge" and "the bridge"
    3  the records
    4  outside the tree: what is renamed to what
    5  what follows the original's rename
    6  what holds it

### Outside the tree

**The original is renamed first, and this one takes the name it leaves.** Enkrid ruled it on
2026-10-07, when the plan had only asked whether the repository was to be renamed at all. The
original's repository is in the same organisation under the name this one is to have, so the
name was not free.

    the original    the repository `Heuretes-Workshop/Bridge-Legacy`, the folder `Bridge-Legacy`
    this library    the repository `Heuretes-Workshop/Bridge`, the folder `Bridge`

Four actions, in the order that never leaves two things with one name: the original's
repository, the original's folder, this repository, this folder. Each is asked before it is
made, since each is outside the tree and seen by whoever has the old address.
