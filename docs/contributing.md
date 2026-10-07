# Contributing

How work is done on Bridge, by whoever does it: what every decision is weighed against, what
always holds, how a decision is made and put, how a stage is planned, run and closed, and where
the rules for the code, the tests and the documents are. Method,
not design: what the design is, is `design.md`.

## The needs

What every option of every decision is scored on. They are this project's own: another project
states others. Each is a name, and what serving it means here.

    fast            nothing is added to what runs in a phase - a system over its rows, a
                    barrier - that a measurement has not paid for
    scalable        it holds as entities, components, modules, systems and threads grow: no
                    cost that grows with what a pass did not touch
    modular         one concern has one owner and one seam, and a part - a backend, a
                    module, a host - is replaced without the others knowing
    extensible      a capability is added from outside the library, by a module, a backend
                    or a host, and not by a change to it
    minimal         nothing is built that a measured need does not ask for, and one
                    mechanism does one job
    explicit        nothing behaves otherwise than its reader expected: nothing is clamped,
                    defaulted or downgraded in silence, and nothing means something by being
                    left out

## What always holds

For whoever does the work, at every step. Each has its reason, because a rule without one gets
bent.

**A stage is planned before it is built.** The plan is shown and agreed before a file changes:
*Planning a stage*. Reason: a stage begun without one is redone.

**One decision at a time, with the reasoning shown first.** Before a question is put, the need,
what each option actually does and the table of scores are in front of the owner: *Putting a
decision*. Reason: a question with no reasoning gets an answer nobody can check.

**Every option is scored on every need.** Never argued from the one need that favours it.
Reason: a decision taken on one need is wrong on the others without anyone seeing it.

**The feature is weighed before its options.** Is it needed at all, for what, and by what
measure; only then the variants. Reason: the best variant of a thing nobody needs is still
waste.

**A label is confirmed before it is acted on.** A short item or an undefined term is read
aloud, with the reading stated, and confirmed before the steps. Reason: a wrong reading costs
the whole item.

**Ordinary meaning first.** A term is taken in its plain sense before a new unit is invented
for it. Reason: an invented meaning has to be taught to every reader.

**One word, one meaning.** A term is defined once, in `readme.md`, *Words*, and never used in a
second sense. Reason: an ambiguity is a defect that nothing catches.

**Nothing by absence.** Nothing is optional, and nothing means something by being left out: it
is stated by whoever knows it. Reason: what is not stated is guessed, differently each time.

**No deferred work.** No shortcut, no path that was not run, no list of what is left behind a
stage: it is finished, or it is put as a decision. Reason: what is left is not come back to.

**Rule over exception.** A breach of a rule is reshaped until it fits, not excepted; the
reshape is recommended, and still asked. Reason: an exception is the next breach's precedent.

**Real mechanisms, no analogies.** An explanation says what the machine, the tool or the
program actually does. Reason: an analogy cannot be checked.

**The project is self contained.** Nothing is run from, or cited by path in, another project's
directory; what is needed of one is copied in. Reason: a project that needs another beside it
cannot be read or checked alone.

**No tool is needed to work here but an editor, git and Python.** Nothing in the tree is
written for one assistant or one editor, and the method is these documents: a person follows
them as they are, and a program that helps is told to read them. Reason: a project that only
one tool can work on is that tool's.

**What leaves the machine, or cannot be undone, is confirmed first.** A push to a new place, a
repository made, a branch or a tag deleted, a merge. Reason: the owner answers for it.

## How a decision is made

A sequence, not advice. Written down because it works when nobody is thinking clearly,
which is when it matters.

**1. State the problem, not a solution.** Split what got merged - most items that resisted
were two or three problems sharing a name. The log and the journal were one item until
idTech3 showed they were different mechanisms with different overflow policies. Separate
"cannot" from "may not": memory safety and authorisation have different answers.

**2. Establish the context from what is written.** Quote the documents, with file and line,
not from memory. Most defects found so far were documents contradicting each other rather
than being wrong. If a settled section already answers it, say so and stop - **five times now** a
question has been treated as open when another section had already decided it. Twice it was the
register; twice it was a signature with a parameter nobody had explained, where the explanation was
in the document that says what Bridge knows about a component.

And look in the **general** rules, not only the sections that name the subject. Item 16 asked what
happens when a backend lacks a capability; the answer was in *When something fails*, which says
Bridge's own refusals take init's path and lists examples. Checking the documents that mention
capabilities found nothing, because the answer was in the document that says what happens when
anything is refused.

**3. Look for prior art, for its reasoning and not its answer.** `prior_art.md` is what this
project reads, and what it took from each. Check what is on disk before
describing it, and say which claims are checkable here and which are from memory; nobody can weigh
evidence they cannot reach. Reading the prior art beats reasoning about it - flecs' own iterator
serialiser answered in nine lines what an hour of argument had not.

**Prior art is evidence, not an oracle.** What you want from it is the *why*: which options its
authors faced, what they were trading against, and which of those forces act on us. That is how you
learn possibilities you had not thought of - including the ones they rejected, which are often the
more useful half.

Citing a choice without its reason is an appeal to authority. Item 14 nearly shipped on "the engine
with the most experience chose refusal" - and once the reason was actually asked for, Godot's turned
out to be **half ours and half not**: it refuses partly because a transparent proxy would make a
synchronous call asynchronous, which does not apply to us, and partly because a blocking proxy
deadlocks, which applies to us *harder* than to them. Neither half was visible from the choice
alone, and the second one is what decided the item.

So the question is never "what did they do". It is **"what were they weighing, and are we weighing
the same thing"** - and step 7 is where the answer gets used.

**4. List the options with pros AND cons, and show them before applying.** Every option gets both.
One with only pros means the work is not done, and the missing cons are where the real trade is.
Include what we have now. Shown *after* the edit it is an assertion with a justification bolted on -
item 15 was applied that way, steps 1 to 3 and then straight to the documents, five steps skipped.

**5. Refuse overlap.** Two things answering the same question is a cost with no gain, and it is how a
design rots: both drift, and neither is clearly wrong. Several ways to *store* data is useful; several
schemas is not. When an option's appeal is "we keep ours and also use theirs", check whether ours still
has a job - the schema format's only remaining job turned out to be one flecs' own macro did directly.
This applies to open questions too: one that overlaps another is a bad cut, not two problems.

**6. Weigh against every named need.** They are the six of *The needs*, above. Every option is
scored on all six, and the scores are shown: it serves the need, it fails it, or
the need does not apply, each with the reason. An option that fails one is refused, or the
conflict between two needs is itself the question to put. Which need decides is said only between
options that fail none. Check whether a cost lands on the hot path - that is the one budget this
project cannot spend, and it is what refused per-module page mapping.

**This step once said "say which need decides", and no more.** So an option was weighed against
the one need that favoured it. The memory handle of the backend seam was recommended away as
"minimal" when it loses on modular, extensible and explicit; nothing in the step asked for the
other five. Every decision taken that way was scored again - `stage10.md`, *The repair*.

**Explicit is the one that gets forgotten**, and it is the one that costs most when it is. Nothing
may behave differently from what the person reading it expected, which rules out more than it looks:
a value clamped instead of refused, a capability downgraded instead of declined, a cleanup whose
condition depends on what somebody else's module declared. Item 40 was decided wrong once for
exactly that - the chosen option made a module's arena cleanup depend on whether another module
had happened to declare the same component.

**7. Tailor rather than adopt.** Take placement from one project and naming from another if
that is the fit, and state every departure with its reason. Their constraint is often
not ours: idTech3 copies a payload inline because its record goes to a file, and ours is read
in-process.

**8. Choose, and record what was refused.** A refusal with no condition attached reads
identically to an oversight a year later. Write the condition that would change the answer, or
that none would. It goes in the record of the document the decision is in - `refused/`, the file
of the same name with `_refused` after it, under the same heading - and not in the document,
which says what the design is.

**9. Apply to the documents.** The decision goes in the document it belongs to, never in a list of
open questions. Such a list holds what is undecided and nothing else, or it becomes a second place
to read the design and a second place for it to be wrong.

**An open question waits in the plan of the stage it blocks** - `stagen.md`, in a section of its
own - and the stage is not done while one stands. Stage 0 kept a single register for all of them; it
was emptied and deleted when the stage closed, because a standing list outlives its questions.

**10. Scan.** Every time, and in order - see below.

**11. Report what closed, what opened, what sharpened.** Including defects of your own found
on the way. Two of the worst were redundant fields added while claiming to copy prior art,
and the scan found both where thinking had not.

## Putting a decision

The sequence above, as it is put to the owner. One decision at a time: the next is not raised
until this one is recorded.

Before the question, and in this order:

    1  the need         is the thing needed at all, for what, by what measure. If not, say so
                        and stop
    2  what is written  what the documents already say, with file and line. If a settled
                        section answers it, say so and stop
    3  prior art        from `prior_art.md`: what each did and why, and whether that reason
                        acts here
    4  the options      what each actually does: which files change, what runs, what a reader
                        then sees. What there is now is one of them. Each has its cost
    5  the table        a row for each need of *The needs*, read from the file; a column for
                        each option; each cell serves, fails or does not apply, with why
    6  a recommendation and which need decided between the options that fail none. If every
                        option fails a need, the conflict is the question

Then the question: the options as they were named, the recommended one first.

After the answer:

    a question back     is answered first, and the decision put again. A clarification is not
                        a choice
    the decision        goes into the document it belongs to, and into the stage's record
                        under a heading of its own: what was ruled, by whom, over which
                        recommendation if it differed, and what it costs
    each option not     goes into the refused record under the same heading: what it was,
    chosen              why, and the condition that brings it back, or that none does
    prior art           what was taken from one, or refused, goes into `prior_art.md`
    a new word          goes into `readme.md`, *Words*, once
    then                the check, a commit of one line, a push

## How a stage is run

`plan.md` is the stages, and each ends in a claim that can be false. This is what is done with
one, every time.

**It is planned before it is begun.** The plan says what there is today, counted and not
remembered; the order the stage is done in; the questions it has to decide; what it reuses; and
how each claim is verified. It is agreed before a file changes.

**It has a record, written as it goes**: `plan/stagen.md`, which is the order, what was counted
before it began, and each decision as it is taken. What a decision refused is
`refused/plan/stagen_refused.md`, under the same heading, with the condition that brings it
back.

**Its decisions are put one at a time**, each by the sequence above, and before the files it
governs change. A label that can be read two ways is read aloud first, and confirmed.

**It is a branch of its own**, from `dev`, named for the stage. A commit and a push follow each
item closed. It is merged when that is asked for, and not before: into `dev`, then `dev` into
`main`, and the branch is deleted.

**A commit is one line.** Conventional Commits, a subject and nothing under it:
`type(scope): description`, where the type is one of `feat`, `fix`, `refactor`, `chore`, `docs`,
`test`, `build`, `perf`, `style`, `ci`, `revert`.

**Nothing is left behind it.** No list of what remains, no path that was not run, no shortcut
to come back to: a thing is finished, or it is put as a decision. A question that stays open
waits in the stage's record, and the stage is not done while one stands.

**It is closed by its done-when, line by line**: each clause met, or said not to be with what
it waits on. Every case passes, optimised and debug, alone and under six copies at once, and
every other target compiles and links.

## Planning a stage

    1  read the stage   in `plan.md`, with its done-when. If the done-when cannot fail, or
                        names something a later stage builds, that is the first question
    2  measure          what there is today, with commands, and write the numbers: files,
                        mentions, cases, whatever the stage will change. Nothing from memory
    3  write the plan   with the sections below, and change no file until it is agreed
    4  once agreed      the stage's branch from `dev`; `plan/stagen.md` with what it is for,
                        what was counted and the order; its refused record; the stage marked
                        "In progress" in `plan.md`; a commit and a push
    5  the decisions    one at a time: *Putting a decision*

A plan's sections, in this order:

    Context                 why the stage exists, the owner's words where there are any, and
                            the reading of any label in them, to be confirmed
    What there is today     the counts, each with what it was measured on
    The order               numbered steps: the record first, the decisions second, the close
                            last
    The decisions           the questions only, each with what makes it a question. No
                            answers: they are the owner's, one at a time
    What is reused          each tool, document or script already here that the stage uses
    Files this will touch
    Verification            a command for each clause of the done-when

## Closing a stage

A stage is not done because its work exists. It is done when each clause of its done-when is
shown true.

    1  the clauses      read the done-when in `plan.md` again, and split it
    2  each is run      now, on the tree as it is. A result from earlier in the stage does
                        not count once anything has changed since
    3  each can fail    break the thing a check holds, see it fail, put it back
    4  the scan         whole: the scripts, then the reading
    5  what was left    an open question in the record, a path not run, a "later". Each is
                        finished now or put as a decision
    6  the record       "The close", as a table: each clause, and met with what showed it, or
                        not met with what it waits on. And what was found false on the way,
                        and any commit that went in with a check failing
    7  the plan         the stage marked "Built", naming the record and any clause that waits
    8  then             the check, a commit of one line, a push, and a report: what is met,
                        what waits and on whom, what the owner should look at
    9  the merge        when it is asked for, and not before: the stage's branch into `dev`,
                        the check on `dev`, the branch deleted here and on the remote, `dev`
                        into `main`, and back on `dev`

## Where the other rules are

Each is held by a program where a program can hold it, and each program is a case of
`python build.py test`.

    style.md                 how the C is written, and the line every file begins with:
                             `tools/style.py`
    tests.md                 what is exposed on the three seams, and the case for each. A
                             thing exposed is added with its case: `tools/cover.py`
    design/build.md          the compiler, the platforms, the standard: `tools/c23.py`
    design/dependencies.md   every tool and every import, with why: `tools/deps.py`
    a message                each is printed by a case, and what writes one cites the
                             document that states it: `tools/said.py`
    a document               every citation names a heading that is there; a document of the
                             design names no stage; what is not built says so; a name that
                             was retired is not written: `tools/xref.py`
    what the project         its needs, above; its prior art, in `prior_art.md`; its words,
    defines                  in `readme.md`. `tools/xref.py` fails on one that is not there

Nothing in the tree is written for one assistant or one editor: *What always holds*.

## What no script can do

Steps 1 to 8 of a decision, and steps 4 to 7 of the scan, are reading and judgement. The three
checks exist so that the **mechanical** checks are never what gets skipped - a count, a citation, a
duplicated run, a ragged line. They cannot weigh an option, re-read an open question against a
decision just made, or notice that an edit left `"Create or Call a module"` behind.

So the failure to watch for is not a missed grep. It is a decision that looks settled and was never
weighed.

## The scan

Run from `docs/`, over the whole tree. The two scripts walk subdirectories; the shell
one-liner takes its list from git, so a document in a new folder is covered without editing it.

Steps 1 to 3 are scripts because a listed check gets skipped and a script gets run. Steps 4 to 7 are
reading, in this order, because each one has caught something the one before it could not.

**1. Citations resolve.**

    python ../tools/xref.py

Every `doc.md`, *Section* pointing at a heading that exists, and no "item N" left: the numbers
belonged to stage 0's register, which is gone. This has caught six citations that resolved to
nothing, every one in text written minutes earlier.

The script reads unwrapped text, because a citation wraps onto the next line as often as not and a
line-at-a-time grep never sees its two halves together. Two dangling citations survived several
clean scans that way.

**2. Duplication.**

    python ../tools/dup.py

Read the hits rather than chasing the number down. A signature in both binding documents is fine;
a document re-arguing a conclusion that lives elsewhere is not.

**3. Formatting.**

    awk 'length>108 {print FILENAME":"FNR": "length}' $(git ls-files '*.md')

108, not 100: the documents are hand-wrapped near 100 to 102, so a tighter threshold reports a
hundred normal lines and the check gets ignored.

Fix the block, not the line. Rewrapping one long line pushes the overflow onto the next.

**4. Counts against their lists.** Count the rows; do not trust the word in front of them.
"Eight regions" has been wrong twice, both times because a row was added and the sentence was
not.

**5. Every open question, re-read against the decision just made.** Some are sharpened, some
absorb a case that would otherwise be a new one, some lose a sub-question. This is where most of the
movement comes from - more than from closing things.

**6. New questions the decision raises.** A decision that answers one thing usually asks
another. Write it into the stage plan rather than leaving it in the margin, and prefer sharpening an
existing question to adding one.

**7. Read the sections the greps pointed at.** A grep finds a stale word, not mangled grammar.
One edit left `"Create or Call a module; if something needs to"` behind, and no pattern would
have found it.
