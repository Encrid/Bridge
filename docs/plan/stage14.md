# Stage 14: the documents

Built. `plan.md`, stage 14, is what it is for and when it is done. This document is the order
it was done in, what was counted before it began, each decision as it was taken, and every
finding with what was done of it. What a decision refused is `stage14_refused.md`.

## The order

    1  the stage         this document, with what was counted
    2  the decisions     each by the method of `contributing.md`, one at a time, before a document
                         changes
    3  the findings      every defect of the kinds the first decision keeps, listed once, by
                         script where a script can find it and by reading where it cannot
    4  the changes       each finding changed or refused on record, a document at a time
    5  the citations     a citation above each exposed name; `cover.py` fails on one with none
    6  the scan          the scan of `contributing.md`, whole
    7  close             by the done-when

## What there was, before any of it

Counted on `c1dd7f0`, the tree stage 13 left. The documents counted are the 39 that state what
the library does: every one but `plan.md`, `style.md`, `tests.md`, `readme.md`, and the folders
`plan/` and `refused/`.

    documents              39, of 4,428 lines, 145 headings and 296 claims in bold that open
                           a paragraph; 29 records under `refused/`, 1,028 lines
    citations              `xref.py` finds none broken
    said twice             `dup.py` finds 328 passages of eight words in two documents, 83 of
                           them between two documents that are neither a plan nor a record
    names with no          169 of 256 exposed names sit under no comment of their header that
    document cited         cites a document: 56 on the host seam, 81 on the module seam, 33 on
                           the backend seam. 47 citations exist
    messages               204 in the source, each printed by a case. Whether a document
                           states each is checked by nothing
    stages named in        14 of the 39 documents name a stage: stage 17 seven times, stage
    the design             16 five, stage 10 five. When a thing was built, in a document that
                           says what it is
    what is not built      39 lines name `validate`, the profile, `bridge_manifest_parse` or
                           the checksum pass, and 14 say "when they exist", "not yet" or the
                           like. `validation.md`, *The checksum pass, when asked*, reads as
                           built
    flecs                  11 documents name it, 26 times, after stage 10 replaced it
    cases pointed at       12 documents point at a case, and 27 at none

**One promise of stage 13 that was not built**, found while this stage was planned.
`stage13.md`, *Every refusal*, says that a message no document has a sentence for is reported
and does not fail. `tools/said.py` reports nothing of the kind. Whether a message is to be
found in a document, and what holds it, is the sixth decision below.

## The decisions

Seven, in this order, since each shapes the next.

    1  what "improved" covers: the kinds of defect this stage looks for
    2  what is said of what is not built
    3  a stage's number in a document of the design
    4  what is said twice: which document owns a conclusion
    5  the citations above exposed names: their form
    6  messages and the documents
    7  how a finding is recorded

### What "improved" covers

**Seven kinds of defect, in the 39 documents that state what the library does, and nothing
else.** "Improved" is made true, single and followable. It is not worded better, and it is not
new design.

    a  false against the tree       a claim the code or a case contradicts
    b  not built, said as built     what a later stage builds, in the present tense
    c  said twice                   a conclusion argued in two documents, which drift
    d  a stage named in the design  when a thing was built, in a document of what it is
    e  prior art that describes     a sentence that says what flecs does as what runs here.
       nothing                      Prior art cited for its reasoning stays: `contributing.md`, step 3
    f  not followable               a sentence that names what the documents never define: an
                                    item of a register that is gone, a person, a discussion.
                                    Not a sentence that could read better
    g  stated nowhere               an exposed name or a message that no document states

`plan.md` named three of them, c, f and what stage 13 showed claimed and not held. The count
turned up b, d, e and g, and a is the one no script finds. Each kind is a defect by a rule that
is written: `contributing.md`, step 9, for where a decision is said, and step 5, which refuses two
things that answer one question, for c.

**What is outside**: wording for taste; a decision of the design taken again; and the records
of `plan/` and `refused/`, which say what was decided and when, except where one is cited
wrongly.

    fast          no part of this runs
    scalable      a kind with a script is held from here on, and a kind without is read once
    modular       each kind is found its own way, and one finding is of one kind
    extensible    a document written later is held by the same scripts
    minimal       seven kinds, each by a rule that was there
    explicit      what is looked for is listed, and what is not is said

### What is said of what is not built

**One mark, in one form, and a script holds it.** Three things are designed and not built: the
diagnostics, which are `validate`, the profile and how full a budget is; the parser of a
manifest; and the seam for a language that is not C. The documents said so four ways: a
sentence at the top of `profile.md`, a sentence in bold beside the fields in `host_api.md`,
"when they exist" in `config.md`. And one had no mark at all and reads as built:
`validation.md`, *The checksum pass, when asked*.

**The form is the one `profile.md` has, without the stage.** A section that states what is not
built opens by saying so: "It is not built." Where it is built is the plan's to say, by the
next decision. A mention anywhere else cites that section, and says nothing of the thing in the
present tense. So a reader meets the mark before the design, and one sentence is removed the
day the thing is built.

**A script fails on an unbuilt name anywhere else.** It has the names that are not built, each
with the section that states it, and reads the documents and the comments of the headers: a
name outside its section that is not in a citation of it fails. The stage that builds a thing
takes its names out of the list, and the mark then fails until it is removed with them. Without
the script the next unmarked mention is found by nobody, as that of `validation.md` was not.

The design stays in the documents of the design: it is decided, and `contributing.md`, step 9, puts
a decision in the document it belongs to.

    fast          no part of this runs
    scalable      a thing designed later is a row of the list
    modular       the design stays where it belongs, and the plan says when
    extensible    the stage that builds a thing removes its rows, and is told what else to
    minimal       one sentence a section, and one list
    explicit      nothing unbuilt reads as built, and a slip fails

### A stage in a document of the design

**A document of the design names no stage, in any form.** Enkrid ruled it, over a
recommendation that would have kept a stage where a thing is not built, cited by its heading.
Not "stage 17", not `stage10.md`, not "`plan.md`, stage 3". The plan points at the design, and
the design never at the plan: what the library is does not depend on the order it was built
in, and that order moved once already, on 2026-10-07, with 29 references renumbered behind it.

There were 39 such lines in the 39 documents, of three kinds, and each has another answer.

    where a thing that is not    15   the mark says that it is not built, and no more. The
    built is built                    stage's part of `plan.md` cites the section, as it does
                                      for all else it builds
    when a thing changed         9    removed. What was there before is the record's: the
                                      stage's document, and `refused/`. Where the past is the
                                      reason for what is, the reason stays and the when goes
    where a measurement or       15   the figure, or the list, is said in the document that
    a list is                         rests on it, or in `costs.md`, and the stage's document
                                      keeps how it was measured

`plan.md` named as the build order, with no stage, is not a stage named: `design.md` and
`readme.md` say that much, and keep it.

**`xref.py` fails on one.** A stage's number, a stage's document, or the plan with a stage
beside it, in a document that is not under `plan/` or `refused/` and is not `plan.md`. Two more
are not held, and neither says what the library is: `readme.md` is the index of the tree, the
plan's folder with the rest, and `style.md` gives a stage's file as an example of a name. The
headers and the source name none today, and are held the same way. The tests, the tools and
the workflows are not documents of the design: eleven of them say which stage's measurement
they repeat, and keep it.

    fast          no part of this runs
    scalable      a document written later is held by the scan
    modular       the plan depends on the design, and the design on nothing of the plan
    extensible    a stage added, moved or renamed changes no document of the design
    minimal       one rule with no case of its own
    explicit      what a document says is what the library is, and a slip fails

### What is said twice

**One owner for a fact: the document whose concern it is states it, and another cites that
heading and says only what is its own.** `readme.md` gives each document one concern, and that
is what says who owns a fact. `dup.py` finds 48 passages of eight words or more that two
documents of the design share. Eight are two documents citing one heading, which is what a
citation is for. Forty are not, and of those about five are the line every document opens its
record with, about five a signature or an example shown twice, and about thirty one fact stated
in two places: `query.md` and `storage.md` share five, among them that a system walks tables or
one sparse set alone.

    a citation shared            stays
    the line that names the      stays
    record of a document
    a signature or an example    shown in the document that owns it, and cited from the other
    a fact                       stated by its owner, and cited from the other

**Each of the forty is a finding**, changed or refused by name. No script says who owns a fact,
so none is asked to: `dup.py` stays where it is, in the scan of `contributing.md`, and is read.

    fast          no part of this runs
    scalable      a document written later is read by the same scan
    modular       one concern a document, and each held to it
    extensible    a fact that changes is changed in one place
    minimal       a rule and the citation there is already
    explicit      which document is right is never a question

### The citation above an exposed name

**Every declaration cites, in its own comment.** Enkrid ruled it, over a recommendation that
would have let a header of one concern cite once, at its top, for every name in it. 114
comments stand above the 256 exposed names: a struct's comment covers its keys, and an enum's
its constants. 24 of them cite a document, and 90 are to be written.

The form is the one in use: the last lines of the comment, a document and a heading, as
`budgets.md`, *Budgets*, is above the budgets. `xref.py` resolves it already. A name cites the
document that owns what it does, by the decision before this one; a second citation is allowed
where a second document adds to it, and is not asked for.

**`cover.py` fails on an exposed name whose declaration has no comment that cites.** So a name
carries its pointer wherever it is read or copied, and one added later fails the build until it
says where it is stated. What it costs is one citation said by each small function of an SDK
header, and a heading renamed touching each of them: `xref.py` names every one.

    fast          no part of this runs
    scalable      a name carries its own citation
    modular       a declaration is whole without the top of its file
    extensible    a new name fails until it cites
    minimal       one rule, with no second level to it
    explicit      a name stated in no document is named, and fails

### Messages and the documents

**Each message is read against the documents once, and then each function that writes one
cites the heading that states the rule it enforces.** A message is the one place a host or the
author of a module meets a rule, so a rule that is enforced and stated nowhere is a defect of
the documents: kind g. 204 messages, written by about 113 functions, of which 16 sit under a
citation today. The documents have about 90 sentences that state a refusal, and they state the
rule and where it is enforced, never the words.

**Read once.** Each message is held against the documents in step 3. One whose rule no document
states is a finding: its owner gains the sentence, or it is refused on record.

**Then held.** The style allows no comment inside a function, so the pointer is above the
function: the form and the checker of the citation above an exposed name, again. `tools/said.py`
reads every call that writes a message already, and fails on one in a function whose comment
cites no document. A function that refuses under two rules cites both. So a refusal written
later fails the build until it says where its rule is stated, which is what stage 13 said this
stage would have and did not build.

    fast          no part of this runs
    scalable      a message is held by the function it is in
    modular       the rule is in its document, and the pointer at what enforces it
    extensible    a refusal added later fails until it cites
    minimal       the form and the script there are already
    explicit      no rule is enforced that the design does not state

### How a finding is recorded

**By its kind: a kind a script holds by the script's count, and a kind found by reading a row
each.** The done-when asks that each thing listed be changed or refused on record, so there is
a list, and it is exact where only reading could make it.

    b  not built, said as built     the script's count before, and none at the close
    d  a stage named in the design  the same
    g  stated nowhere               the same, for names and for messages
    a  false against the tree       a row each, in this document: where, what was false, what
    c  said twice                   was done
    e  prior art that describes
       nothing
    f  not followable

**Every refusal is by name**, of whatever kind, in `stage14_refused.md` under the heading of
its kind, with the condition that would bring it back. So nothing that was found and left is
silent, and several hundred lines that a script counts are not written out by hand to say what
a diff says.

**A claim that is false because the code is wrong** is a defect of the library and not of the
document: fixed in the stage, with a case, and said under what the stage found, as the two of
stage 13 were.

    fast          no part of this runs
    scalable      hundreds of findings, and a row only for what a script cannot count
    modular       a kind is recorded by what finds it
    extensible    a kind that gains a script later moves to its count
    minimal       no second document, and no row a script prints already
    explicit      what reading found is by name, and so is all that was refused

## Decided while the findings were worked

### A load refused before the module exists

**It stops nothing, whatever `on_error` says, and the documents say so.** `failure.md` and
`capabilities.md` said that every refusal of Bridge's own takes init's path, so that a
module stated as `fail` stops the deck when a capability its backend lacks refuses it. The
code has two moments. Before the module has a record, its config is held against the
deployment and the backend, and a refusal there is answered by `bridge_load` and no more. After,
a failure goes by `on_error`. The messages had the two phrases already: "is refused", and "did
not load".

The code is right, and for a reason that makes the other reading impossible to hold whole:
`on_error` is a key of the config being checked, and one of the checks is that it is set.

    fast          no part of this runs
    scalable      does not apply
    modular       a config is checked before anything is made of it
    extensible    a check added later falls on one side by where it runs
    minimal       no line of the library changes
    explicit      two phrases, two outcomes, and a case that shows the first

The case is `BRIDGE_FAIL-refused-before-it-exists-stops-nothing`.

## What was found

By kind, as decided: a count for what a script holds, and a row for what reading found. Every
one was changed, but those named in `stage14_refused.md`.

### What a script holds

    b  not built, said as built     15 mentions in 9 documents and one header, in four forms.
                                    Four headings state what is not built now, each opening
                                    with "It is not built.", and `xref.py` finds no mention
                                    elsewhere without a citation of one
    d  a stage named in the design  39 lines in 14 documents; none, held by `xref.py`
    g  a name with no document      90 of the 114 comments above exposed names cited none;
                                    none, held by `cover.py`
    g  a message with no document   109 of the 125 functions that write a message cited none,
                                    and some twenty rules were stated in no document. Each has its
                                    sentence and each function its citation, held by `said.py`

### a: false against the tree

Each was read against the code by one of three readers, and then checked. Where the document
was right and the code was not, it is under *What the stage found*.

    storage.md           the list of free ranges is not as long as there may be tables; a
                         query's spans are rewritten with every other of the module's, and
                         not those after it; a module reads four things, and not three
    query.md             the entry is `add_system`; what is refused is two components walked
                         in different ways, and not kept in different storage layouts
    reference.md,        `column_base` has a row for each sparse set as well as each table
    budgets.md
    budgets.md           the list of kept settings is shortened, when a share is given back;
                         `work_limit` and `memory_limit` have no floor
    failure.md           `bridge_list_modules` does not say which module stopped the deck;
                         the allocations refused in turn are three hundred and some
    failure.md,          a load refused before the module exists does not go by `on_error`,
    capabilities.md      which was decided on the way, above
    validation.md        the running system is stored in the module's header, and not in the log
    schedule.md          a module whose own order a pair reverses is not refused: the pair
                         stands
    abi.md               a later module to declare a component is answered a slot of its own
    schema.md            the row test can be left out, by the module's author
    wasm.md              which enums are four bytes and which one; where the imports, the
                         choice of the row test and `bridge_api_t` are declared
    host_api.md          one host program, and the static one runs 22 cases again; one SDK
                         uses `_Generic`; the library does parse a module's descriptors; the
                         listings of the functions and the structs had another type's name,
                         and lacked two budgets, the losses and a name for the union. They are
                         tables now, and `bridge.h` states each
    config.md            the phases are a list and not an ordered one; the table lacked
                         `changes`, `after` and a module's phases
    create.md            more causes refuse a deployment than the seven that were listed; a
                         null deployment is refused with no record, as a null sink is
    lifecycle.md,        a changed component layout is carried across, and refused only for
    costs.md             a loss the deployment did not accept; grants are checked as `init`
                         declares, and not before the module has a record
    design.md            the library has types; the entry is `add_system`
    build.md             no variable is a thread's own, and `thread_local` is not used; four
                         headers; six targets; sixteen functions of the C library, without
                         `memchr` and `strpbrk`
    no_builtins.md       the library keeps no clock
    capabilities.md      what macOS does with the native backend's library

### c: said twice

Thirty-four passages of eight words that two documents of the design shared and that were no
citation. Four stated a fact its owner states, with no pointer, and cite the owner now:

    native.md            when rows move: `schedule.md`
    validation.md        how the row test is stated: `wasm.md`
    costs.md             that a write to what was declared read is a bug: `data.md`
    config.md            how a module reaches its settings: `memory.md`

The other thirty are left, and are named with why in `stage14_refused.md`.

### e: prior art that describes nothing

    validation.md        what the entity index needed while flecs kept the rows
    budgets.md           that `terms` was a constant, flecs' default
    build.md             the paragraph on C99 chosen for a number that matched flecs
    schema.md            that the macro had flecs' name once

flecs is still cited in ten documents, each time for its reasoning.

### f: not followable

    lifetime.md          "This document said that", of a text that is gone
    strings.md           two files cited with no project; what "was decided before" a thing
    failure.md           a heading cited with no document
    abi.md               "since it was called a cost"
    schema.md            "Both engines", with none named before
    native.md            a project named nowhere else
    capabilities.md      what three keys were called once
    costs.md             two checks of a host's, quoted; what a refusal "was first" for
    split.md             the escalation guards

Four more are left, and are named in `stage14_refused.md`.

## What the stage found

**A tag handed a value was accepted.** `storage.md`, *A tag*, says an attach that hands a tag a
value is refused. Nothing refused it: a tag's size is zero, so any place in the scratch arena
passed as a value of no bytes. `attach` refuses it now, and the case is
`attach-tag-with-a-value-refused`.

**Three structs of the backend seam stated no layout.** `host_api.md` says every struct of a
seam is laid out where it is stated, with no padding left to a compiler, and that
`bridge_backend.h` holds to it. `backend_source_t` had four bytes of padding no member named,
and it, `backend_entry_t` and `backend_err_t` asserted nothing. Each asserts its size and every
offset now, and the gap is a member.

**The list of what is free in the column region did not count singletons.** A gap lies between
two blocks, and the list has room for one beside every block there may be. A singleton has a
block, and was not counted, so a deployment with many of them could have a freed range dropped
from the list and its bytes lost to the deck for good. The room is by `types` now, which
bounds them.

**Stage 13 promised a report it did not build**: a message that no document states. It is built
here, as a failure and not a report: *Messages and the documents*.

## The close

**The done-when, line by line.**

    each thing the first question listed is    met: seven kinds. Three are held by a script
    changed or refused on record               and count none; four have a row each above,
                                               and what was left is named in the record
    `xref.py` finds no broken citation         met: none, with two checks more than it had
    every case passes                          met: 473, optimised and debug, alone and under
                                               six copies for ten rounds; the five other
                                               targets compile and link

**What holds the documents from here on.** A stage named in a document of the design, and a
mention of what is not built with no citation of where it is stated: `xref.py`. A declaration
of a header that cites no document: `cover.py`, the case `cover`. A function that writes a
message and cites no document: `said.py`, the case `said`. What is false against the tree, and
what two documents say twice, are held by nothing but the scan of `contributing.md`, read.

**Linux and macOS** are as stage 13 left them: no workflow has run, and that holds no stage.
