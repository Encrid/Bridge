#!/usr/bin/env python3
"""Every reference resolves: `doc.md`, *Section* to a real heading, and "item N"
to an item still open in OPEN.md.

Catches the failure the restructure makes likely: a pointer that looks right and
resolves to nothing. The item half exists because a gap in the register means
closed, so a document citing one points at text nobody can read - and the number
wraps onto the next line often enough that a line-at-a-time grep misses it.
Run from docs/.
"""
import glob
import io
import os
import re
import sys

# keyed by basename: prose cites `data.md`, not `bridge/data.md`, because a
# reader wants the document's name and not where it sits in the tree.
heads, paths = {}, {}
# PHASE0.md is the method, not the design: it cites closed items on purpose,
# as the worked examples that make each step concrete, and it carries a literal
# `doc.md`, *Section* template.
for full in glob.glob('**/*.md', recursive=True):
    if os.path.basename(full) == 'PHASE0.md':
        continue
    f = os.path.basename(full)
    paths[f] = full
    heads[f] = set()
    for line in io.open(paths[f], encoding='utf-8'):
        m = re.match(r'#{2,4}\s+(.*?)\s*$', line)
        if m:
            heads[f].add(m.group(1).lower())

# the register is deleted when phase 0 ends, and then no item number is valid -
# so an absent OPEN.md means every "item N" reference is dangling, not that the
# check is off.
open_items = set()
if os.path.exists('OPEN.md'):
    open_items = set(re.findall(r'(?m)^\*\*(\d+)\.',
                                io.open('OPEN.md', encoding='utf-8').read()))

bad = 0
for f in sorted(paths):
    raw = io.open(paths[f], encoding='utf-8').read()
    # citations wrap, so a reference is matched against unwrapped prose. without
    # this, `doc.md`, *A section whose
    # title wrapped* silently matched nothing and a wrong file went unnoticed.
    text = re.sub(r'\n(?=\S)', ' ', raw)

    # item N, across a line break too: the register is the only place an item
    # number means anything, and a closed one is a gap.
    for m in re.finditer(r'[Ii]tem\s+(\d+)', text):
        if m.group(1) not in open_items:
            line = text.count('\n', 0, m.start()) + 1
            print('%-14s:%d -> item %s is closed' % (f, line, m.group(1)))
            bad += 1

    # cross-document: `other.md`, *Section*   (backticked or not)
    for m in re.finditer(r'`?([a-z-]+\.md)`?[^.\n]{0,30}?\*([^*\n]+)\*', text):
        doc, sec = m.group(1), m.group(2).strip().lower()
        if doc not in heads:
            print('%-14s -> missing file %s' % (f, doc))
            bad += 1
        elif sec not in heads[doc]:
            print('%-14s -> %s has no heading "%s"' % (f, doc, sec))
            bad += 1

    # same-document: *Section* with no file named. single asterisks only,
    # so **bold** lead-ins are not mistaken for references.
    for m in re.finditer(r'(?<![*\w])\*(?!\*)([A-Z][^*\n]{3,40})\*(?!\*)', text):
        sec = m.group(1).strip().lower().rstrip('.,;')
        if sec in heads[f] or any(sec in heads[d] for d in heads):
            continue
        print('%-14s -> dangling "%s"' % (f, m.group(1)))
        bad += 1

print('%d broken' % bad)
sys.exit(1 if bad else 0)
