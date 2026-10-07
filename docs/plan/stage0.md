# Stage 0: how we decide things

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

**3. Look for prior art, for its reasoning and not its answer.** Check what is on disk before
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

**6. Weigh against every named need.** Fast, scalable, modular, extensible, minimal, **explicit**.
Every option is scored on all six, and the scores are shown: it serves the need, it fails it, or
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

## What no script can do

Steps 1 to 8 here, and steps 4 to 7 of the scan, are reading and judgement. The three checks exist so
that the **mechanical** checks are never what gets skipped - a count, a citation, a duplicated run, a
ragged line. They cannot weigh an option, re-read an open question against a decision just made, or
notice that an edit left `"Create or Call a module"` behind.

So the failure to watch for is not a missed grep. It is a decision that looks settled and was never
weighed.

## The scan

Run from `docs/`. The paths below are relative to that, not to this file -
`plan/` is where a stage's own document lives, and the scan is run over the
whole tree from one place. The two scripts walk subdirectories; the
shell one-liner takes its list from git, so a document in a new folder is covered without editing
it.

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
