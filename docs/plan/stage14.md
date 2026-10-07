# Stage 14: the documents

In progress. `plan.md`, stage 14, is what it is for and when it is done. This document is the
order it is done in, what was counted before it began, each decision as it is taken, and every
finding with what was done of it. What a decision refused is `stage14_refused.md`.

## The order

    1  the stage         this document, with what was counted
    2  the decisions     each by the method of `stage0.md`, one at a time, before a document
                         changes
    3  the findings      every defect of the kinds the first decision keeps, listed once, by
                         script where a script can find it and by reading where it cannot
    4  the changes       each finding changed or refused on record, a document at a time
    5  the citations     a citation above each exposed name; `cover.py` fails on one with none
    6  the scan          the scan of `stage0.md`, whole
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
       nothing                      Prior art cited for its reasoning stays: `stage0.md`, step 3
    f  not followable               a sentence that names what the documents never define: an
                                    item of a register that is gone, a person, a discussion.
                                    Not a sentence that could read better
    g  stated nowhere               an exposed name or a message that no document states

`plan.md` named three of them, c, f and what stage 13 showed claimed and not held. The count
turned up b, d, e and g, and a is the one no script finds. Each kind is a defect by a rule that
is written: `stage0.md`, step 9, for where a decision is said, and step 5, which refuses two
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

The design stays in the documents of the design: it is decided, and `stage0.md`, step 9, puts
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
beside it, in a document that is not under `plan/` or `refused/` and is not `plan.md`. The
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
so none is asked to: `dup.py` stays where it is, in the scan of `stage0.md`, and is read.

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
