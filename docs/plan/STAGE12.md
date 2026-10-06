# Stage 12: one style, every platform, few dependencies

In progress. `PLAN.md`, stage 12, is what it is for and when it is done. This document is the
order it is done in, what is measured, and each decision as it is taken.

## What it replaced

Stage 12 was to be "self contained": the C library's routines and an allocator carried in the tree,
and no C runtime linked on Windows. It was planned, its sources were read and one experiment
was made, and then it was dropped for this. What the experiment found is in the record of
`build.md`, *The C library*, with the condition that would bring it back.

## The order

    1  the style         `STYLE.md`: the Epitech C Coding Style 3.1, made the project's own
    2  the stage         this document and `PLAN.md`
    3  the checker       `tools/style.py`, from scratch, run with the tests over the paths
                         that are clean, a list that only grows
    4  the decisions     what shapes files, before any is rewritten: one operating-system
                         layer; the module ABI at four arguments; the names; how storage is
                         cut with its helpers still inline; the test programs as tables of
                         cases; where a comment goes
    5  the refactor      a directory at a time, from what ships to what tests it, every
                         commit measured
    6  dependencies      the list, the case that holds it, and what can leave it
    7  platforms         a Windows workflow; Linux arm64, macOS x86-64, Windows arm64
    8  close             by the done-when

## What was decided before it began

    what the style covers     all C of ours, the tests and their modules included. Not Python
    its numbers               hard: a breach fails, and an exception is an entry of
                              `STYLE.md`, *Exceptions*, with its reason
    the seams                 not excepted: `_t` on the public types, four arguments an entry.
                              Excepting the module ABI's arguments was proposed and had no
                              reason: the entries over four run at load, and the ones a phase
                              calls take four or fewer already
    `f(void)`                 rule F5 wins over C23's `f()`, and `tools/c23.py` loses that rule
    the checker               ours, in Python. Epitech's own, banana, is a plug-in for its
                              compiler, built with CMake, and carries no licence: nothing of
                              it can be taken but the list of which rules a program can hold

## The tree against the style, before any of it

    F3  80 columns             written to about 100
    F4  20 lines a function    19 functions over 80 lines; the longest, a test's `main`, 742
    O3  5 functions a file     `storage.c` has 97; `abi.c` 35; `lifecycle.c` 29
    F5  4 arguments            `declare` 9, `system` 8, `observe` 7
    F6  no comment in a body   about 380 inside or beside code
    F2  a verb in the name     about 60 static functions named by a participle or a noun
    V1  `_t`                   no type has it
    L1  one statement a line   about 600 `if` with its statement on the same line
    L5  one declaration a      253 lines declare several
        line
    C3  no `goto`              20
    C1  3 branches             every test program is one chain on its argument, up to 28 arms
    G1  the header             no file has it
