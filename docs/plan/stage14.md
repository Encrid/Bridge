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
