# Stage 13: tests, by capability

Built, with one clause of its done-when waiting on a thing outside the tree: *The close*.
`plan.md`, stage 13, is what it is for and when it is done. This document is the order it was
done in, what was counted before it began, each decision as it was taken, and what was built.
What a decision refused is `stage13_refused.md`.

## The order

    1  the stage        this document, with what was counted
    2  the decisions    each by the method of `stage0.md`, one at a time, before a case moves
    3  the list         every capability the documents state, written down once
    4  the check        a script and a case that hold the list against the cases
    5  the cases        renamed for what they cover; the programs with them
    6  the gaps         a case written for each capability the list shows has none
    7  the load         every case run with several copies at once
    8  close            by the done-when

It was built in another order: 5, then the refusals of 6, then 3 and 4 together, then the rest
of 6, then 7. A list whose rows are the names of cases could not be written before the cases had
their names, and which message has no case is found by running them.

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

**A capability is what Bridge exposes: a thing somebody outside the library can interact
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

**One document, `tests.md`: a row a capability, with the document and the heading that
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

**One host program, `tests/host`, and a case is named for what is exposed and what it shows**:
`declare-past-types-refused`, `bridge_reload-smaller-share-refused`, `config-trusted-unset`. A
behaviour of several entries is named for the behaviour: `order-unordered-conflict-refused`. The
name is its row in `tests.md`, word for word, so a capability and its case are called the
same.

It was first taken as the document that states the capability, then the case. When a capability
became what is exposed and the list was grouped by seam and by exposed thing, the names followed:
a list grouped one way and named another would be two orders for one thing.

The five programs named for stages differed in how `main` configures the host, one backend or
two, one phase or two, which sink: about fifteen lines each since stage 12. They become one
`main`, with the cases in files by what is exposed. `storage`, `schema` and `adopt` are named for what
they test and reach inside the library; they stay. Several names may run one function with
different expectations, as three do today: one run can show three capabilities, and each has
its case. The 22 cases run again on the program linked statically stay that, named `static-`
and the case's name.

    fast          one program linked where there were five
    scalable      a new case is a function and a row
    modular       cases are grouped by what is exposed, as the list is
    extensible    a new entry is a file of cases
    minimal       one `main`, one row of `build.py`
    explicit      a name says what is exposed and what is shown, and no name says a stage

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

### Under load

**`python build.py test --copies 6`: each case is started six times at once, and every copy is
judged as a case is.** A case is a process, so six of one case compete for the processors, for
memory, and for what a process shares with its neighbours: the backend's library on disk,
wasmtime's own threads. Six is the number that found stage 9's crash, which had passed a hundred
and fifty runs alone and had been there since stage 2. Nothing in a plain run would find the
next one, since cases run one at a time.

**A case that measures time is run once, and is marked so with its reason**: under load it gives
another number, and that is no defect. The measurements of storage, the row test timed, and the
two that decide whether two naps overlapped. A case that only bounds time, a shutdown that has a
tenth of a second, stays in: if load breaks it, that is found. The mark is a word and a reason
in `CASES`, and not an absence from a list.

**Six copies once is a sample**, and stage 9's crash was one run in sixty. The workflows run
plain and then ten loaded rounds; a hundred are run here once, before the stage closes.

    fast          a plain run costs what it did
    scalable      a new case is under load with no line written for it
    modular       one judge for a copy as for a case
    extensible    the number is an argument
    minimal       one flag, and a mark on the few cases that measure
    explicit      what is left out of the load is named, with why

### The platforms no workflow runs

Not a decision of this stage: Enkrid ruled for stage 12 that the workflows not starting does not
hold a stage, and it stands. The done-when says every case passes on Windows, Linux and macOS,
alone and under load. Windows is run here. The rest is met when the jobs run, and until then the
close says which half is known.

## What was built

### The list, and what holds it

`tests.md`, and not `capabilities.md` as it was decided: that name is the document of what a
backend can do, a citation names a document by its file alone, and one word is not to mean two
things.

`tools/cover.py` reads what is exposed from the headers: a function, an entry of a table, a
macro that takes arguments, a constant, and a struct. A struct a host fills is read key by key,
`config.threads`, `module_config.trusted`, since each key is stated and refused in its own
words; any other struct is a layout and is one name. That is 256 names: 92 on the host seam, 120
on the module seam, 44 on the backend seam. `tests.md` has a heading for each and eleven more,
for what is no one name's: the order of two that conflict, the library linked statically, the
four programs that reach inside, and the five checks of the tree. The script fails on an exposed
name with no heading, a heading with no case, a heading a seam's headers do not expose, a case
under no heading, a case listed that is not run, and a document that points at a case that is
not run. It is the case `cover`.

**Which document states each name is not in the list, and it was decided that it would be.**
The decision was taken while a capability was a sentence of a document. Found for every name by
a script, the pointer was wrong more often than right: the documents say what the library does
in their own words, and `kind` in backticks is under five headings. Where it goes was decided
again: *Where a name's document is said*.

### One host program

`tests/host`: fifty-two sources, one `main`, a table of 371 rows. A row is a case's name, the
function that runs it, what the function is handed, and its fixture. The five programs differed
in what `main` set before a case ran, so those five settings are five fixtures, each a function:
the wasm backend with one phase and the sink that prints a level; the wasm backend with both
phases; both backends; and both again with the sink that counts, and with the sink that notes.
A case that read its own name to know what to load is handed that as its argument.

The 371 rows are 251 runs. A run that loads two modules and reads a sum shows a grant, a query,
an accessor, a log and a layout at once, and each has its case: the same function and argument,
judged by the words that are its own. A capability that no run showed has a run of its own.

### Every message

`tools/said.py`: 204 places in `src/` and in the two backends that write a message. The count
before the stage, 209, was a search for the functions' names and took their declarations for
calls. Three of the 204 pass on a message written elsewhere, and have no words of their own.
`build.py test` keeps what every case said, a file a case, and begins a run of every case with
none, so that a case that is gone holds nothing. The script is the case `said`.

**Eighty-six of them were printed by no case.** Not all for want of a case: the reader of
descriptors was checked by the word a refusal stops at, and the refusal was never printed; the
case that answers every allocation null counted the refusals and printed none. Those two print
now. What was left was written:

    a key of the deployment or of a module     41 cases, most off two tables of a key and
                                                a value
    an entry of the ABI, misused                30 cases, off one module built 30 ways
    a backend, called as the library calls it  5 checks added to `tests/adopt`
    a library that is no backend                2 cases, off two libraries built for them
    the operating system refusing memory        the build that counts allocations counts
                                                what is reserved as well
    a reload that lays a component out again   a case that refuses each of its allocations
    the rest                                    a case each

### Under load

`python build.py test --copies 6`, as decided, and `--rounds N` beside it, so that ten loaded
rounds are one command on each of three systems. Eleven cases are started once whatever the
flag says, each with why, in `ALONE`: four that measure, two that decide by the clock whether
two naps overlapped, and five that read the tree or what the run said.

## What the stage found

**A reload that ran out of memory blamed the budget.** A reload that moves a component's rows
to another storage layout takes memory to carry the values across. With none, it said that
`[budget] tables` or `[budget] columns` had no room: one answer, false, for two causes. Found by
the case that refuses each allocation of such a reload in turn, which had one refusal that no
record said. `storage_convert` answers which of the two it was, and the reload says it.

**The wasm backend lost a block between two threads.** wasmtime asks for every linear memory
and is not told which, so the backend marks which block the next one is, makes the instance
that asks, and unmarks it. Until stage 12 the mark was a variable of the thread. Stage 12 held
that file to the style, which has no variable of a file that is not constant, and moved the
mark into the backend, where two threads share it: one that unmarked it between the other's
marking and wasmtime's asking left that other a memory of wasmtime's own. Its module ran, every
call returned, and nothing it wrote was in the shared block. Bridge makes instances one
at a time, so no case of the library reached it. `tests/adopt` makes two on two threads, and in
the first loaded run of the finished tree one copy of six said that a whole page was not what
was written. The mark is set under the backend's lock now. It is the second defect that six
copies found and one did not, and the first that this stage's own load found.

**A deployment with no backend was created.** `backend_count` of zero was not refused, where
`phase_count` of zero is. No module could be loaded into it, and each load was refused by the
name of the backend it asked for. Found by the case written for it, which printed nothing but
that the process was alive. Decided afterwards: *A deployment with no backend*.

## The close

**The done-when, line by line.**

    every capability has a case named for it,    met: 256 exposed names, each a heading with
    and the build fails on one that has none     its cases; the case `cover`
    no case is named for a stage                 met: 471 cases, none
    every case passes on Windows, Linux and      Windows: met, optimised and debug, alone and
    macOS, alone and with several copies         under six copies; a hundred loaded rounds
                                                 run here, 47,000 cases, none failed, and
                                                 ten more with the case added since. Linux
                                                 and macOS: not met, since no workflow has run

**The workflows.** As for stage 12: GitHub Actions starts no job while the account's billing
refuses them, and Enkrid ruled that this does not hold a stage. Each of the three workflows
runs the cases plain and then ten loaded rounds. The five other targets compile and link here.
When the jobs run, what they find is this stage's to fix.

**What was Enkrid's to rule on.** Two things, neither of which a case waited for, each decided
after the cases were done, on 2026-10-07: *Where a name's document is said*, and *A deployment
with no backend*.

## Decided after the cases

### Where a name's document is said

**In the header, in the comment above the name, and it is written in stage 14.** Decision 2 put
the document and the heading that state a capability in its row of the list. Two questions were
one there: where a name's cases are, which is `tests.md`, and where a name is stated, which is
this.

The headers answer the second already, for some: 47 comments cite a document and a heading,
`budgets.md`, *Budgets*, above what they describe, and `tools/xref.py` fails on one that names
no heading. By a count of the nearest comment, 87 of the 256 exposed names sit under such a
citation and 169 do not. So the pointer has a place and a check, and the list repeating it would
be a second place for one fact.

**What stage 14 builds of it**: a citation above each exposed name, and `tools/cover.py` failing
on a name that has none, so that a name added later says where it is stated the day it is
written. It is stage 14's because that stage reads every document to say what of them is to be
improved, and a citation written before it would be written twice.

    fast          no part of this runs
    scalable      a name carries its own citation, wherever it is declared
    modular       the header says where a name is stated, and the list where it is shown
    extensible    a new name fails the build until it cites its document
    minimal       one place, and the check there is already
    explicit      a name stated in no document is named, and fails

### A deployment with no backend

**It is refused at create, in its own words, as a deployment with no phase is.** The host seam
reaches storage through modules and through nothing else, and a module is run by a backend. So
a deck with no backend can load nothing and run nothing, which is what a deck with no phase
is refused for. It was created, and the first sign was a load refused for the name of a backend,
which says what is missing and not why.

A count of zero is not unset everywhere: no pair of an order, no grant, no setting are each a
deployment that works, and are accepted. What decides is whether what is left can do anything.

    fast          one test, at create
    scalable      does not apply
    modular       the check is where the backends are opened
    extensible    a count added later is asked the same question
    minimal       four lines, one message, one case
    explicit      refused where it is wrong, by the rule a reader of the phases expects

The case is `config.backends-none-refused`.
