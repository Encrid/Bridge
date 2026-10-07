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
"when they exist" in `config.md`, and nothing at all over *The checksum pass, when asked* of
`validation.md`, which reads as built.

**The form is the one `profile.md` has.** A section that states what is not built opens by
saying so and by naming where it is built. A mention anywhere else cites that section, and says
nothing of the thing in the present tense. So a reader meets the mark before the design, and
one sentence is removed the day the thing is built.

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
