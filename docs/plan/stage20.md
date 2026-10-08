# Stage 20: the licence

In progress. `plan.md`, stage 20, is what it is for and when it is done. This document is the
order it is done in, what was counted before it began, and each decision as it is taken. What a
decision refused is `stage20_refused.md`.

It is 20 and not 16: the numbers 16 to 19 are the four stages that were moved to a host's plan,
and a record that says one of them means that stage.

Enkrid, on 2026-10-07: "Adding LICENSE to Bridge ... (the same legacy/rust-components had, it
was a bit special you'l see. we'll have to adapt a bit maybe)".

**Nobody who worked on this stage is a lawyer.** What it does is make the text say what is
meant, name what exists, and hold that by a program. Whether the words do in law what they say
is for somebody qualified to read, and each decision says where that matters.

## What there was (measured on `801ed0b`)

    in the tree         no licence. The repository is private, and its host reports none
    every file of ours  468 files open with the header of `style.md`, rule G1, whose fourth
                        line is "All rights reserved." `tools/style.py` holds that line by an
                        expression. 153 are under `src/`, about 190 under `tests/`, 29 under
                        `backends/`, 4 in `include/`
    the holder named    "Heuretes-Workshop", in all 468: the name of an organisation on the
                        service the repository is hosted by
    what others get     `include/`: `bridge.h`, `bridge_abi.h`, `bridge_schema.h`,
                        `bridge_backend.h`. `backends/wasm/sdk/`: nine headers and one `.wat`.
                        `backends/native/sdk/`: four headers. Built: a shared library, and a
                        static one for a host that is to be one file
    somebody else's     wasmtime, under `backends/wasm/wasmtime/`, with its licence, which is
                        Apache-2.0 with the LLVM exception: headers, an import library and a
                        shared library for each target. And `src/text.c`, two functions after
                        musl's, named in its header as MIT

**What `legacy/rust-components` had.**

    LICENSE             the GNU General Public License, version 3, as published
    LICENSE-EXCEPTION   "Bridge Linking Exception 1.0": a module, a native plugin and a
                        backend that meet Bridge only through `sdk/` are not derivative works,
                        and may be under any terms, with no source owed. Changing Bridge
                        itself stays under the GPL
    CLA.md              the Apache contributor agreement, adapted: a contributor keeps the
                        copyright and grants a licence that may be passed on, so that the
                        whole can be offered where the GPL cannot go, a console or a store
    a workflow          that records who signed, on a branch of the repository
    the package         said "GPL-3.0-only"

**Where that text no longer matches what Bridge is**, which is why it is not copied over.

    a host is outside   that design was the program, and nothing linked it. This one is a
                        library a host links, and the exception names no host: as written,
                        every host would be a combined work under the GPL. Its readme had
                        said a product built on it stays its author's
    seam code is        `bridge_abi.h`, `bridge_schema.h` and the SDKs are compiled into a
    compiled in         module. The exception says code copied out of Bridge is still
                        Bridge's and is not covered, which would reach every module
    it names paths      `sdk/`, an interface language, a plugin ABI, a trait: none is here
    a static library    a host built as one file holds Bridge's object code inside it
    the header          "All rights reserved." says the opposite of a licence that grants
    the workflow        needs a service that has never run for this organisation

## The order

    1  the stage        this document, and stage 20 in `plan.md` with a done-when
    2  the decisions    ten, one at a time, before a file changes
    3  the texts        the licence, the exception as ruled, and the agreement if it comes
    4  the header       rule G1, the expression that holds it, and the 468 files, by a script
    5  the documents    what a host, a module and a backend may be under, said once; and
                        somebody else's code, each with its licence
    6  what holds it    a program fails on a file with the old line, on a path the exception
                        names that is not there, and on a licence that is not the published
                        text
    7  close            put to Enkrid by the done-when

## The decisions

### What must be open

**Everything built with Bridge: a host, a module, a backend. The GNU Affero General Public
License, version 3, with no exception; and other terms are bought from the holder.** Enkrid:
"I want things built with Bridge to be free and open source. If they want it closed they buy a
License." It is not what `legacy/rust-components` had, where what Bridge loaded could be closed
at no cost.

What that licence does with nothing added to it: a host links Bridge, so the two are one work,
and the host is published under it or under a licence compatible with it; a module compiles
the seam headers and its SDK into itself, so the same holds for a module, and for a backend.
The Affero licence was chosen over the plain one for one case the plain one leaves: a host
that is never handed to anybody and only runs as a service, a company's servers, which under
the plain one could stay closed and pay nothing.

**Three of this stage's questions go with it.** There is no exception, so there is nothing to
say of seam code compiled into other work, of how an exception names an interface, or of what
a static link owes: the licence answers each the same way for everything.

**What it asks for.** Selling other terms takes holding the rights to all of Bridge, which one
author does today and which only a contributor's agreement keeps true after the first
contribution from outside; and it takes a holder who can sign a contract. Both are questions
below. What is somebody else's in the tree, wasmtime and two functions after musl's, is under
terms that allow either.

Where a qualified reader matters: that a module is one work with what it compiles in and is
loaded by is how these licences are generally read, and is not something this stage can show.

### Who the holder is

**Ruled twice. What stands is the second ruling, at the end of this section: the notices say
"Bridge Owner".**

The first: the name in the notices stays, and one document says whose it is. "Heuretes-Workshop"
was in all 468 notices. Enkrid: "Heuretes-Workshop is the name of a github organization I created and
I work under". So it is a name its author works under and not a person in law: the rights are
the author's, and the document that says what may be done with Bridge says that this is the
name they are held under, gives where other terms are asked for, and says that an entity formed
later to hold them takes its place. No file changes for this.

The repository itself was moved the same day from that organisation to its author's own
account, for reasons of the hosting service and not of this stage; the name is unchanged by
that.

**The second ruling: the notices name a role, "Bridge Owner", and no name of a person is in
the tree.** Once the repository was on its author's own account and the organisation kept for
work shared with others, a notice in the organisation's name read as a group holding the
rights, which is the opposite of what selling other terms needs. Four ways were weighed with
what others do: a real name, as most single authors; the account's name, which exists only
because another was taken; the name the commits are made under; and a role. Enkrid first chose
his real name, saw that it would tie that name to the one on every commit, in public and for
good, and ruled the role.

    it never changes     an entity formed later, or the repository moved to another account,
                         changes one sentence of `licence.md` and no notice
    owner, not author    a notice names who holds the rights, which may come to be somebody
                         who wrote nothing; and a contributor is an author of their part and
                         not an owner of Bridge
    not an account       `licence.md` and `cla.md` say the owner is the author and whoever the
                         author transfers the rights to in writing, and that who administers
                         an account does not decide it
    one way only         a name can be added to the tree later in a minute, and can never be
                         taken out of a repository that has been public

What it costs: a notice that names nobody is weak evidence taken alone. What shows who wrote
Bridge is its history and what its author keeps privately, and the author's name in law is
given in a contract, to whoever takes other terms.

Where a qualified reader matters: what a notice that names a role is worth as evidence, where
the author lives.

### The contributor's agreement

**A licence and not a transfer, signed through a workflow of our own, and nothing by hand.**
Enkrid ruled each. A contributor keeps the copyright of what they wrote and grants the holder
a licence that may be passed on under any terms, and the agreement says in so many words that
Bridge is also offered under terms that are paid for and closed, and that a contribution may be
part of what is offered so. The agreement of `legacy/rust-components` said a contribution would
never be taken private, which would be false here, so it is not reused as it is.

The signing is a program of ours that the hosting service runs on a pull request: it reads who
opened it, looks the name up in a list kept on a branch of the repository, asks in a comment
for a stated sentence when the name is not there, records the name when that sentence is
answered by that person, and fails the pull request until then. It never runs anything from
the pull request, which it treats as data: a workflow that can write to a repository and
answers to strangers must not.

The program the earlier design used for this was archived by its authors, which is why this
one is ours.

**It cannot be shown to run today.** On 2026-10-07 every workflow of the organisation the
repository was in failed before its first step, for a failure of payment on that
organisation's account. The repository has since moved to another account, and the first push
after that shows whether one runs there. Until one has, this holds nothing, and the close of
this stage says which it is.

### Which versions

**Version 3 only.** Enkrid ruled it, as recommended. The terms are the text in the tree, and a
later version applies only if the holder chooses it, which the agreement of contributors lets
the holder do. It is what `legacy/rust-components` said of its own.

### The line in every file

**A line of words: "Under the GNU Affero General Public License, version 3 only: see
license."** Enkrid ruled words, over a recommendation of the registered identifier for these
terms, which a program can read. The words first ruled began "Licensed under", and were 85
columns in a comment where rule F3 of `style.md` allows 80; they were made one word shorter,
at his word, and no rule was excepted. It takes the place of "All rights reserved." in rule G1 of
`style.md`, in the expression of `tools/style.py` that holds the header, and in each file, so no
file grows. The cost is that no tool outside this tree reads it.

### What the files are called

**In lower case: `license` at the root, `cla.md` beside it, and `licence.md` among the
documents for what may be done with Bridge.** Enkrid ruled it, as recommended. Rule O4 of
`style.md` asks it of every file of ours, and no rule is excepted. The file at the root has the
spelling the hosting service looks for, and the document the spelling these documents use for
the noun. If the service does not recognise the name in lower case, this is put again.

### What else is under it

**Everything in the tree that is ours: the library, the backends, their SDKs, the tests, the
modules of the tests, the tools and the documents.** Enkrid ruled it, as recommended. What is
somebody else's keeps its own licence, named beside it. A module compiles the seam headers into
itself and is loaded by the library, so it is under these terms whatever an example it was
copied from was under, and a second licence for the examples or for the prose would be a
boundary to find and nothing gained.

### Where other terms are asked for

**Of the author's account on the hosting service, `Encrid`, and nowhere else.** Enkrid ruled
it. No address and no name of a person is in the tree.

## What was found on the way

**The line first ruled was too wide.** It is in *The line in every file*, above.

**The workflows had never run, and then ran all at once.** Until 2026-10-07 every workflow of
this repository failed before its first step, for a failure of payment on the account of the
organisation it was in: 51 runs. The repository was moved that day to its author's own account,
for reasons that were the hosting service's and not this stage's, and the next push started
them: three workflows on every push to any branch, six machines each time, two of them macOS,
whose minutes are counted as ten. Three pushes of this stage were running at once. All were
cancelled, at Enkrid's word, and he ruled when they run:

    for what reaches     `dev` and `main`, and for a pull request. A stage's branch is held
                         by the cases run where it is written, as it always was
    a newer push         stops the run of the one before it on the same branch
    documents alone      start nothing: a change under `docs/`, to a markdown file or to the
                         licence text

So no run of Linux or of macOS has finished yet. The first is the merge of this stage.

**What `legacy/rust-components` promised of a contribution would be false here.** Its agreement
said one would never be taken private. Under terms that are sold it may be, inside a buyer's
product, so `cla.md` is version 2.0 and says so before anything else.

**musl's licence asks that its notice be kept with what is taken from it**, and the tree held
only a line naming it in the header of `src/text.c`. The notice is in `licence.md`.

### A module, and a backend, as a host

**The licence is not worded for them, and what is meant is said in two places.** Enkrid asked
whether a module and a backend need wording of their own, since they use Bridge as a host
does. Nothing is added to the licence: it treats every work combined with Bridge alike, and it
lets a recipient ignore a restriction somebody adds to it. What it reaches is decided by
copyright and not by what is written beside it.

The firm ground is the same for all three: each compiles Bridge's own headers into itself. The
soft place is a module written without them, from the documents, and run in a sandbox: whether
being loaded by a work and sharing its memory makes one work with it has been argued both ways.
So what is meant is stated, which does not extend the licence and does tell every reader how it
is read here:

    `licence.md`       a module is under it however it was written, and so is a backend
    each seam file     the four headers of `include/` and the fourteen files of the two SDKs
                       say, where they open, that what is compiled with them is a work based
                       on Bridge, under the same licence. `tools/licence.py` fails on one that
                       does not

Where a qualified reader matters most in this stage: whether that reading of a sandboxed
module holds.
