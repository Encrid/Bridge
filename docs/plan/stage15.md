# Stage 15: Bridge

Built. *The close*.
`plan.md`, stage 15, is what it is for and when it is done. This document is the order it was
done in, what was counted before it began, each decision as it was taken, and what the library
and two other things were called before. What a decision refused is `stage15_refused.md`.

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
                           taken
    on this machine        the folder `Bridge-ASM700`, and beside it `Bridge`, which is the
                           original
    who names the          two files of one other project, by path or by repository
    original

## The decisions

Six. The fourth was taken while the stage was planned, and taken again when it was begun.

    1  what the design before this one is called, in the documents
    2  the library and the object a host creates: two words that would differ by a capital
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

    0.35          a plugin framework in C++17, of      `legacy/cpp-plugins`
                  native libraries loaded dynamically
    0.36          a module system in C17 with a        `legacy/c-abi`
                  stable C ABI
    0.37          the first in Rust: a supervisor      `legacy/rust-supervisor`
                  of WebAssembly components
    0.38          the same, with backends, plugins     `legacy/rust-backends`
                  and an SDK
    main, dev     the same, finished: a host for       `legacy/rust-components`, at what
                  modules of the Component Model       `dev` was
    dev-web-      its web backend, kept apart          `legacy/rust-web-backend`
    backend

**Each is a branch.** The first four were made as tags, on the reasoning that a point which
never moves is one, and Enkrid had them made branches the same day: they were branches, a
branch is what a design is looked for under, and one kind of name for all six is one thing to
know. No name has a number: the count was not kept, and a name that needs none cannot be
wrong. The dates git holds say the order.

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

### The library, and the object a host creates

**The object is a deck, wherever a person reads of it, and Bridge is the library alone.**
Once the library is Bridge, "the bridge" is a capital and an article from it, and one sentence
has both: that `fail` stops the bridge, since the library cannot stop the program. Enkrid
ruled that the object's word changes, over a recommendation that both stay with a rule on the
article and a check.

**A deck**: what `bridge_create` makes and `bridge_destroy` unmakes, with its storage, its
modules and its threads. It is the part of a bridge that things are on and move over: modules
are loaded onto a deck, and a deck is what stops. The word is used nowhere in the documents
today, and it stays with the library's name where every other word on offer left it. It means
nothing until it is said once what it is, and `readme.md` says it.

"Instance" is not free, being a module's, 49 times; nor "deployment", which is what a deck is
made of; nor "runtime", which is wasmtime. "Session" was taken first and given up the same
hour: it reads as somebody logging in.

**Everywhere it is read**: 33 places of the documents, 122 comments, the four messages that
say it and what the cases expect of them, and what the test programs print. A host reads the
documents and the log, and they are to say one word. No identifier changes: the type is
`bridge_t` and the functions are `bridge_create` and the rest, named for the library they are
of.

    fast          no part of this runs
    scalable      does not apply
    modular       the library has a name and the object a word, and neither is the other's
    extensible    a deck of another deployment in the same process is another deck
    minimal       one word, where two differed by a letter
    explicit      a sentence cannot be read of the wrong one

### The records

**Every record takes the three new names, and the old ones stand in this document and its
refused record alone.** The documents under `plan/` and `refused/`, and `plan.md`, say what was
decided and when, and stage 14 left them outside what it changed for that reason. A name is
another matter: a reader goes from `storage.md` to the record of the stage that built storage,
and is not to meet three words there that mean something else, or nothing, where they came
from.

What it was called is a fact, and is kept once, where a reader looks for it: here. So the
done-when of `plan.md` is reworded to what is held: nothing in the tree says the old name but
the record of the stage that changed it.

    fast          no part of this runs
    scalable      a record written later has the one vocabulary to write in
    modular       one vocabulary in the tree, and the history of it in one document
    extensible    a name changed again is changed everywhere, and said in that stage's record
    minimal       one exemption, of two files, by name
    explicit      what a word means does not depend on the folder it is read in

### What holds it

**`tools/xref.py` fails on an old name, and is a case from here on.** Three words are not to
come back: the old name of the library, "the original Bridge", and the object called a bridge.
`xref.py` is the script that reads names in text, and it reads for these in every tracked file
of ours that is text: the documents, the sources, the tests, the tools, `build.py` and the
workflows. Not what is under `backends/wasm/wasmtime`, which is not ours; and not this document
and its refused record, by name.

**It was run by hand, in the scan of `stage0.md`, and failed no build.** So a citation of a
heading that is gone, a stage named in a document of the design and a mention of what is not
built with no citation, which stage 14 gave it to hold, were each held by whoever remembered
to run it. It is the case `xref` now, and all of them fail the build.

    fast          one more case, of under a second
    scalable      a file added to the tree is read with the rest
    modular       names in text are one script's, whatever the file
    extensible    a word retired later is a row of its list
    minimal       no script added, and one case
    explicit      a slip fails the build, and says the file and the line

### The folders on this machine

**They are Enkrid's to rename, by a script written for it, when no session runs in them.** The
clone of `legacy/rust-components` becomes `Bridge-rust-components` and this one becomes `Bridge`.
The operating system will not rename a folder a process is running in, and the tool this was
built with keeps a project's memory under the folder's path, so the script renames those two
with them. It is not in the tree: it is of one machine.

## What was renamed

    the library      Bridge-ASM700, 830 times in 551 files, is Bridge. The line every file of
                     ours begins with says so, and `tools/style.py` holds it
    the object a     "the bridge", 284 times in the documents, the comments, four messages,
    host creates     what eight cases are named and what the test programs print, is a deck.
                     One constant and one file of the tests were named for it, and are
    the design       "the original Bridge", 35 times, is `legacy/rust-components`. Eleven
    before this      sentences that hung a possessive on it were built again
    the repository   `Heuretes-Workshop/Bridge-ASM700`: its history goes into
                     `Heuretes-Workshop/Bridge`, and it is archived

No identifier of a seam changed: `bridge.h`, `bridge_t`, `bridge_create`, `BRIDGE_ABI`.

`readme.md` says once what a deck is, and what the branch is. `tools/xref.py` fails on any of
the three old words in a tracked file of ours, and is the case `xref`: 474 cases.

## Outside the tree, as it was done

In `Heuretes-Workshop/Bridge`, on 2026-10-07, with Enkrid's word for each step:

    made       the branch `legacy/rust-components`, at a commit of the Rust line that had
               not been pushed; the branch `legacy/rust-web-backend`; and the branches
               `legacy/cpp-plugins`, `legacy/c-abi`, `legacy/rust-supervisor` and
               `legacy/rust-backends`, each at the commit its numbered branch was at
    removed    the branches `0.35`, `0.36`, `0.37`, `0.38` and `dev-web-backend`, once each
               commit had its new name

And three once this stage was merged, since what is pushed is the merged tree:

    pushed      this history as `main` and `dev`, over the Rust line's, which was forced, and
                made only if both were still where they had been read. What they were is
                reachable as `legacy/rust-components`, the commit another project's submodule
                names with it
    described   the repository; and `Heuretes-Workshop/Bridge-ASM700` archived
    pointed     the remote of each clone on this machine at the one repository; in the clone
                of the Rust line, its branch renamed to what it is called there

The tag `v0.1.0` of the Rust line is as it was.

## The close

**The done-when, line by line.**

    nothing in the tree says the name the     met: none, but in this document and its
    library was built under, but the          refused record, and `xref` fails on one
    record of this stage
    the design before it has one name         met: `legacy/rust-components`, 35 times, and
    wherever it is meant                      `xref` fails on what it was called
    every case passes                         met: 474, optimised and debug, alone and under
                                              six copies; the five other targets compile and
                                              link

**Linux and macOS** are as stage 13 left them: no workflow has run, and that holds no stage.
