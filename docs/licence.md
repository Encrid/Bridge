# The licence

What Bridge is under, what that asks of what is built with it, whose it is, where other terms
are asked for, and what in the tree is somebody else's. This says what is meant, in plain
words. What binds is the text of `license`, and where the two differ, that text holds.

## What Bridge is under

**The GNU Affero General Public License, version 3, and no other version.** Its text is
`license`, at the root, as its authors publish it. Nothing is added to it and nothing is
excepted from it.

**Everything in the tree that is ours is under it**: the library, the tests, their backend and
their modules, the tools and these documents. A source file says so in the line its
header has for it - `style.md`, *G1 - File header*.

`tools/licence.py` holds this, and is the case `licence`.

## What is built with it

**It is open too.** A host links the library, so the two are one work. A module compiles the
seam headers and its backend's SDK into itself and is loaded by the library, and a backend is
linked with it, so the same holds for each. Whoever passes such a work on, or lets others use
it over a network, offers its source under this licence or under one that may be combined with
it.

    a host      under this licence, or one compatible with it, with its source
    a module    the same
    a backend   the same
    a change    to Bridge itself: under this licence

**A module is under it however it was written.** One that compiles the seam headers holds
Bridge's own code. One written without them, from these documents alone, is still made to be
loaded by Bridge and to work on Bridge's storage, through the layouts and the entries Bridge
defines, and does nothing by itself: it is meant here as a part of the work it is loaded into,
and whoever wants it otherwise asks for other terms. The same is meant of a backend. Each file
of the three seams says so where it opens, since that is where whoever
writes a module looks.

That a work which links or compiles in another is one work with it is how licences of this
family are generally read; whether one that is only loaded is, has been argued both ways. This
says what is meant. Nobody who wrote this document is a lawyer.

## Other terms

**Whoever wants to keep closed what they build with Bridge asks for other terms.** They are had
from the holder, for a price, and they are the only way a host, a module or a backend may be
passed on without its source.

They are asked of the account `Encrid`, on the service this repository is hosted by.

## Whose it is

**The notices say "Bridge Owner", and that is one person: the author of Bridge.** The owner is
whoever holds the rights in it, which is its author until the author transfers them in writing,
to a person or to an entity formed to hold them, and then whoever they were transferred to. The
notices stay true through that.

**It is not decided by an account.** Where the repository is published from, and who
administers that account or organisation, says nothing of who the owner is: a repository is
moved in a minute, and the rights do not move with it.

No name of a person is in the tree. Who the owner is in law is said to whoever takes other
terms, in the contract that grants them.

**A contribution is taken with an agreement, `cla.md`.** A contributor keeps the copyright of
what they wrote and grants the holder a licence that may be passed on, under these terms and
under the others. Without that, the first change from somebody else would be a part of Bridge
the holder could not offer under other terms.

It is signed by a sentence, as a comment on the pull request, and `tools/cla.py` asks for it,
records who signed on the branch `cla-signatures`, and fails a pull request whose author has
not. The holder is not asked.

## What is somebody else's

Each keeps its own licence.

    `src/text.c`   two functions after musl's. The MIT licence, whose notice is below

What a build needs installed, and what the built files import, is `dependencies.md`.

**musl's notice**, which its licence asks to be kept with what is taken from it:

    Copyright (C) 2005-2020 Rich Felker, et al.

    Permission is hereby granted, free of charge, to any person obtaining a copy of this
    software and associated documentation files (the "Software"), to deal in the Software
    without restriction, including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
    to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or
    substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
    INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
    PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
    FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
    OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
