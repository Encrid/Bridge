# Stage 13: tests, by capability

In progress. `plan.md`, stage 13, is what it is for and when it is done. This document is the
order it is done in, what was counted before it began, and each decision as it is taken. What a
decision refused is `stage13_refused.md`.

## The order

    1  the stage        this document, with what was counted
    2  the decisions    each by the method of `stage0.md`, one at a time, before a case moves
    3  the list         every capability the documents state, written down once
    4  the check        a script and a case that hold the list against the cases
    5  the cases        renamed for what they cover; the programs with them
    6  the gaps         a case written for each capability the list shows has none
    7  the load         every case run with several copies at once
    8  close            by the done-when

## What there was, before any of it

Counted on `7344b59`, the tree stage 12 left with the stages reordered.

    cases                238 run; 216 are named rows of `CASES` in `build.py`, and 22 are
                         every case of `stage2` again, on the program linked statically
    named for a stage    `stage2` 22, `stage3` 39, `stage4` 11, `stage5` 35, `stage6` 62,
                         and the 22 named `static`
    named for a thing    `storage` 38, `alloc` 4, `schema`, `adopt`, `c23`, `style`, `deps`
    programs             `tests/stage2` to `tests/stage6`, `tests/storage`, `tests/schema`,
                         `tests/adopt`: each a directory with a table of its cases, since
                         stage 12
    refusals in source   209 places that write one: `abi_refuse`, `module_refuse`,
                         `reload_refuse`, `say_error`, `say_about`, `module_fail`,
                         `config_refuse`, `schema_refuse`

    documents            40 that state what the library does: every one but `plan.md`,
                         `style.md`, and the folders `plan/` and `refused/`. 4,672 lines
    claims in bold       313 that open a paragraph as a sentence. 520 bold spans in all, of
                         which about 108 are one word stressed and many are a reason
    refusals stated      138 sentences, 95 of them "is refused". Nearly all say where it is
                         refused and how it is named, and not the text: the text is in the
                         patterns of `build.py` and nowhere in a document
    claims in tables     about 20 rows with nothing in bold: `budgets.md`, *What running out
                         does*, and the two tables of `lifetime.md`
    cases a document     34 of the 216: 28 as `tests/x`, case `y`; 4 by their name; 2 as a
    points at            program with one case. 182 cases have no document that points at them
    documents that       19 of the 40. Among those with none: `wasm.md`, `query.md`,
    point at any case    `lifecycle.md`, `create.md`, `limits.md`, `threads.md`, `pinned.md`,
                         `module.md`

So the capabilities are some hundreds of sentences, the cases are 216, and 34 pairs are joined.

**What is stated and not built**, and so is no capability of this stage: the profile and how
full each budget is, stage 17; `bridge_manifest_parse`, stage 16; the seams for a language that
is not C, stage 18; a clock and a shared set of phases, stage 19. One of them reads as built:
`validation.md`, *The checksum pass, when asked*, is in the present tense, and the key that
turns it on is not built. A scan that took its bold claims would take them wrongly.

**What is stated and cannot be tested from outside**: what a document says is a contract and
not a guarantee. `lifecycle.md`, that the three calls are legal only between phases;
`module.md`, that none of four things is enforced; `native.md`, *What a native module must not
do*. Whether each is a capability is the first decision's to say.

Found while counting, and corrected: `readme.md` said the plan has eighteen stages. It has
twenty, 0 to 19.

## The decisions

Six, in this order, since each shapes the next.

    1  what a capability is
    2  where the list lives
    3  what a case is named, and what the five host programs become
    4  every refusal: whether the list of them is read from the source
    5  under load: how several copies at once are run
    6  the platforms no workflow runs

### What a capability is

**A capability is what Bridge-ASM700 exposes: a thing somebody outside the library can interact
with, on one of its three seams.** Those are the three there are, and all three count:

    the host seam       `bridge.h`: what a host calls, and the configuration it hands over
    the module seam     `bridge_abi.h` and a backend's SDK: the entries a module calls, and
                        what it reads and writes in the two memories
    the backend seam    `bridge_backend.h`: the entries a backend answers

So the list begins from the surface and not from the prose: every function, every entry, every
key of a configuration, every layout a module reads. That is finite, and a script can count it.
For each, what it does and what it refuses is what a case shows.

**A behaviour that is no one entry's is a capability too, and hangs on the entries it is reached
through**: the order within a phase, what an unload leaves behind, what running out of a budget
does, one deployment giving the same result on one thread and on four. Somebody outside
interacts with each of these, through several entries at once.

It was first decided as a sentence of a document that could be observed to be false, and Enkrid
corrected it the same day, 2026-10-07: the unit is what is exposed, and a sentence is how a
document states it. The difference is where the list starts. From the documents it is some
hundreds of sentences with no end until all are read, and it misses an exposed thing no document
describes. From the surface it is bounded, and a thing with no case is found by counting.

**What is not one.** A reason and a cost. What a later stage builds. And a contract, which is
stated and which nothing enforces - "legal only between phases" - so it has no case and cannot
have one: it is listed apart as that, so that its missing case is not read as a gap.

    fast          no part of this runs
    scalable      a new entry or key is a capability the day it is exposed
    modular       the edge of the list is the three seams, which is where the library ends
    extensible    stages 17 and 18 add to the surface, and so to the list
    minimal       the surface is counted, and not found by reading
    explicit      a thing is exposed or it is not, and each that is has its cases

### Where the list lives

**One document, `capabilities.md`: a row a capability, with the document and the heading that
state it, and its case.** A script reads it and fails three ways, each of which is mechanical: a
row with no case, a case that is in no row, and a pointer anywhere in the documents to a case
that is no row. The documents point at a case 34 times today, in prose where the claim is made,
and nothing checks them; they are checked from here on, and one at every claim is not asked for.

**What the script cannot do** is find a capability nobody listed. That is the reading of step 3,
once. After it the rule of `plan.md`, stage 13, holds: a capability is added with its row.

    fast          no part of this runs
    scalable      some hundreds of rows in one file
    modular       the list is one thing, apart from the prose it points into
    extensible    a new capability is a row
    minimal       one document and one script
    explicit      every capability is a row, and a row with no case fails

### What a case is named, and the host programs

**One host program, `tests/host`, and a case is named for the document that states its capability
and what it shows**: `schedule-unordered-conflict-refused`. The name is its row in
`capabilities.md`, word for word, so a capability and its case are called the same, and whoever
reads a failing case knows which document to open.

The five programs named for stages differed in how `main` configures the host, one backend or
two, one phase or two, which sink: about fifteen lines each since stage 12. They become one
`main`, with the cases in files by document. `storage`, `schema` and `adopt` are named for what
they test and reach inside the library; they stay. Several names may run one function with
different expectations, as three do today: one run can show three capabilities, and each has
its case. The 22 cases run again on the program linked statically stay that, named `static-`
and the case's name.

    fast          one program linked where there were five
    scalable      a new case is a function and a row
    modular       cases are grouped by the document they prove
    extensible    a new document is a file of cases
    minimal       one `main`, one row of `build.py`
    explicit      a name says a document and what is shown, and no name says a stage

### Every refusal

**The messages are read from the source, and each is held against what the cases printed.** 209
places write one, each with a format string. `build.py test` keeps what every case said, which
it captures already, and after the run a script makes a pattern of each format string and asks
whether some case printed it. One that no case made appear fails the build, by its file and
line. So a message is proven printed and not proven expected: many cases expect a fragment only.

What it turns up is one of three things. A message no case reaches has a case written for it in
step 6, or is code nothing can reach and goes. A message for want of memory appears where every
allocation is failed in turn, `alloc-nomemory`. A message that is not a refusal, a share that
stays, a loss that was accepted, is held the same way: the library says it, and a case makes it.

**Printed is this check's, and stated is the list's.** A message that no document has a sentence
for is reported and does not fail: it is a gap in the documents, and stage 14's.

    fast          some milliseconds after a run
    scalable      a new message is held the day it is written
    modular       it reads `src/` and the run's output, and nothing of a case
    extensible    stages 17 and 18 have nothing to do for it
    minimal       one script, and output `build.py` has already
    explicit      a message with no case is named by file and line
