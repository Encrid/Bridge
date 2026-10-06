#!/usr/bin/env python3
"""Every reference resolves: `doc.md`, *Section* to a real heading, and no
"item N" is left behind.

Catches the failure the restructure makes likely: a pointer that looks right and
resolves to nothing. The item half exists because the register went when stage 0
ended, so a document citing an item points at text nobody can read - and the
number wraps onto the next line often enough that a line-at-a-time grep misses
it. Run from docs/.
"""
import glob
import os
import re
import sys

# keyed by basename: prose cites `data.md`, not `bridge/data.md`, because a
# reader wants the document's name and not where it sits in the tree.
heads, paths = {}, {}
# stage0.md is the method, not the design: it cites closed items on purpose,
# as the worked examples that make each step concrete, and it carries a literal
# `doc.md`, *Section* template.
for full in glob.glob('**/*.md', recursive=True):
    if os.path.basename(full) == 'stage0.md':
        continue
    f = os.path.basename(full)
    paths[f] = full
    heads[f] = set()
    for line in open(paths[f], encoding='utf-8'):
        # a title counts: a document that is one section has only that heading.
        m = re.match(r'#{1,4}\s+(.*?)\s*$', line)
        if m:
            heads[f].add(m.group(1).lower())

bad = 0
for f in sorted(paths):
    raw = open(paths[f], encoding='utf-8').read()
    # citations wrap, so a reference is matched against unwrapped prose. without
    # this, `doc.md`, *A section whose
    # title wrapped* silently matched nothing and a wrong file went unnoticed.
    text = re.sub(r'\n(?=\S)', ' ', raw)

    # item N, across a line break too: the register was the only place an item
    # number meant anything, and it is gone.
    for m in re.finditer(r'[Ii]tem\s+(\d+)', text):
        line = text.count('\n', 0, m.start()) + 1
        print('%-14s:%d -> item %s is closed' % (f, line, m.group(1)))
        bad += 1

    # cross-document: `other.md`, *Section*   (backticked or not)
    for m in re.finditer(r'`?([A-Za-z0-9_]+\.md)`?[^.\n]{0,30}?\*([^*\n]+)\*', text):
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

# what the code cites, in a comment, in the same form: a heading that was renamed must not leave
# a function pointing at nothing. A comment's lines are joined first, as a paragraph's are.
for full in sorted(glob.glob('../**/*.[ch]', recursive=True)):
    full = full.replace(os.sep, '/')
    if '/build/' in full or '/wasmtime/' in full:
        continue
    text = re.sub(r'\n[ \t]*(?:\*\*?|//)[ \t]?', ' ', open(full, encoding='utf-8').read())
    for m in re.finditer(r'`?([A-Za-z0-9_]+\.md)`?[^.\n]{0,30}?\*([^*\n]+)\*', text):
        doc, sec = m.group(1), m.group(2).strip().lower()
        if doc not in heads:
            print('%-14s -> missing file %s' % (full[3:], doc))
            bad += 1
        elif sec not in heads[doc]:
            print('%-14s -> %s has no heading "%s"' % (full[3:], doc, sec))
            bad += 1

print('%d broken' % bad)
sys.exit(1 if bad else 0)
