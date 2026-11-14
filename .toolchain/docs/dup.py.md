# .toolchain/tools/dup.py

`dup.py` checks that each passage of the documents is in one document only.
A passage in 2 documents is one fact in 2 places, and one of the 2 places can become false.

```sh
python .toolchain/tools/dup.py [--root <folder>]
```

## The scopes

`dup.py` reads the documents of `prose.documents`: `readme.md`, and each file `.md` of `docs` and of
`.toolchain/docs`, as `ours` gives them.
It checks 2 scopes apart:

1. The documents of the project: `readme.md` and the files of `docs`.
2. The documents of the toolchain: the files of `.toolchain/docs`.

It never compares a document of one scope with a document of the other.
A project does not change `.toolchain/docs`, so it cannot fix such a pair.

## Passages

A passage is a run of words that 2 documents of one scope share, inside one sentence of each.
It has at least the number of words of `[dup] words` of `.toolchain/toolchain.toml`.
`units` gets the sentences of a document in these steps:

1. It replaces each fenced block with empty lines. Thus a block of examples or of quotes holds no passage.
2. It removes each citation whose code span names a file `.md`. A citation points to a fact, and does not state it.
3. It keeps each other code span as one piece: the words of its text, with no backticks and no `|`.
4. It cuts the text into units, and each unit into sentences: `.toolchain/docs/prose.py.md`, *Sentences*.

The form of a citation is in `.toolchain/docs/xref.py.md`, *The checks*.
`words` takes the words of a sentence as `prose.py` counts them, then compares them in lower case, with no
punctuation.
Thus 2 sentences that follow each other never make one passage, and a table cell is a sentence of its own.
A code span holds no end of a sentence.

`shared_runs` finds each run of `[dup] words` words that 2 documents or more hold.
In each sentence, it joins the runs that follow or overlap each other into one passage.
The places of a passage are the places of each of its runs, so a document can be there 2 times.
A passage inside a longer passage does not count if the longer passage names each of its places.

## Quotes

A passage inside a code span at each of its places is a quote, for example of a message of the code.
Such a passage counts only if 2 of its places hold the same code span.
A quote keeps the words that it quotes. Thus 2 different messages that share 8 words make no fault.
One message in 2 documents is a fault, and so is a sentence that repeats the words of a quote.

## Exceptions

An exception of `[[dup.exception]]` lets a group of documents share passages:
`.toolchain/docs/project.toml.md`, *[[dup.exception]]*.
A passage makes no fault if each of its places is in a document that `paths` of one exception matches.
A passage that also has a place in a different document is still a fault.

`dup.py` reports an exception that states no reason: `the exception for '<paths>' states no reason`.
It also reports an exception that excuses no passage: `the exception for '<paths>' excuses nothing`.
`<paths>` is the list of `paths`, with a comma and a space between 2 paths.

## Output

The output follows `.toolchain/docs/shared.py.md`, *The report of a check*.
Its faults follow the places of the passages, and the count names documents.
A fault is `<document>:<line>, <document>:<line>: <passage>`, with each place of the passage.
The line is the first line of the unit of the passage. The passage is in lower case, with no punctuation.

## Functions

| Function | Result |
| --- | --- |
| `unwrap(line)` | Gives the line with no citation, and with the text of each other code span. |
| `words(sentence)` | Gives the words of a sentence, in lower case and with no punctuation. With each word, it gives the words of the code span that holds it, or an empty text. |
| `units(lines)` | Gives the sentences of a document: for each one, the first line of its unit, its words, and the code span of each word. |
| `quoted(places)` | Tells if each place holds the passage inside a code span, and no 2 of these code spans are the same. |
| `shared_runs(units_by_file, size)` | Gives each passage of `size` words or more that 2 documents share, with each of its places, as the sections *Passages* and *Quotes* tell. |
| `excused(places, exceptions)` | Tells if an exception excuses the passage of these places, and counts the passage for each such exception. |
| `exceptions(settings)` | Gives each exception of `[[dup.exception]]`, with its count of passages at 0. |
| `stale(exceptions)` | Gives a fault for each exception that states no reason or that excuses nothing. |
| `scopes(documents)` | Gives the 2 scopes: the documents of the project, and the documents of the toolchain. |
| `main()` | Reads the options, checks each scope, and writes the faults. |
