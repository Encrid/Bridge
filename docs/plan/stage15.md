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
    6  outside the tree  this history into the repository the earlier ones are in, and those
                         kept under names of their own: *Outside the tree*. Each action
                         confirmed before it is made
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

Six. The fourth was taken while the stage was planned, and taken again when it was begun.

    1  what the design before this one is called, in the documents
    2  "Bridge" and "the bridge"
    3  the records
    4  outside the tree: one repository, and what the earlier iterations are called
    5  the folders on this machine
    6  what holds it

### Outside the tree

**One repository: this history becomes `main` and `dev` of `Heuretes-Workshop/Bridge`, and what
was there is kept beside it under `legacy/`.** Enkrid ruled it on 2026-10-07, over what had
been decided an hour before, which was to rename the original's repository to free the name.
Two things were wrong with that. "Legacy" named one thing where there had been many: the
repository holds four designs, of which the Rust one is the last. And another project has that
repository as a submodule, which is an address and a commit: under a rename, the address would
have answered with a repository that has no such commit.

A repository holds any number of histories that share no commit, and a branch is a name for
one commit. So the 307 commits of this library go in beside the 681 that are there, and none
is rewritten. No repository is renamed, no address changes, and the commit the submodule names
stays reachable.

**What was there, and what each is called.** It was kept as branches numbered 0.35 to 0.38,
which read as versions of one thing and are neither: the number counted iterations, and the
package says 0.1.0 throughout. Read one by one, they are four designs, and two of the numbers
mark the same one on its way:

    0.35          a plugin framework in C++17, of      the tag `legacy/cpp-plugins`
                  native libraries loaded dynamically
    0.36          a module system in C17 with a        the tag `legacy/c-abi`
                  stable C ABI
    0.37          the first in Rust: a supervisor      the tag `legacy/rust-supervisor`
                  of WebAssembly components
    0.38          the same, with backends, plugins     the tag `legacy/rust-backends`
                  and an SDK
    main, dev     the same, finished: a host for       the branch `legacy/rust-components`,
                  modules of the Component Model       at what `dev` was
    dev-web-      its web backend, kept apart          the branch `legacy/rust-web-backend`
    backend

A point that never moves is a tag, and the first four are that: three were plain ancestors of
`main` with nothing of their own. The last Rust line is a branch, since it has four commits
`main` had not, issues open against it, and the submodule's commit on it. No name has a
number: the count was not kept, and a name that needs none cannot be wrong. The dates git holds
say the order.

**`Heuretes-Workshop/Bridge-ASM700` is archived, and not deleted**, once its history is in the
other. The folder on this machine is decided with the actions, below.

    fast          no part of this runs
    scalable      another design, if there is one, is one more name under `legacy/`
    modular       each design is whole under its own name, and shares no commit with another
    extensible    nothing that points at the repository has to change
    minimal       one repository, and no rename
    explicit      a name says what a design was, and no name says a version it never had

**The actions, each asked before it is made**, in the order that loses nothing if one stops:
the six names made in the repository, at the commits above; the old branches `0.35` to `0.38`
and `dev-web-backend` removed, once their tags and branch are there; this history pushed as
`main` and `dev`, which moves `main` to a commit that is no descendant of what it was and so
is forced; the repository's default branch and description; this repository archived; and the
two clones on this machine.

### What the design before this one is called

**By the name of its branch, `legacy/rust-components`, wherever it is meant.** Enkrid ruled
it, over a recommendation of a name in prose, "the Rust Bridge", said once beside its branch.
The documents call it "the original Bridge" 33 times, and it is not: three designs came before
it. It is cited as prior art, for what it measured, what it did and what it got wrong, and a
reader who wants the source of a claim looks for a name the repository has. One name, and it
is the one that finds it.

A sentence built on the old words is built again where the new name will not carry it: a
possessive becomes "of", so that no sentence hangs an apostrophe on a name in backticks.
`readme.md` says once what the branch is.

    fast          no part of this runs
    scalable      another design cited later is cited by its name under `legacy/`
    modular       the documents and the repository have one name for it
    extensible    a design that follows this one changes no sentence
    minimal       one name, and none in prose to keep beside it
    explicit      the name is where it is found
