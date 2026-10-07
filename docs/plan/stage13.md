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
