#!/usr/bin/env python3
"""Re-wrap the paragraphs that hold a line over the limit, and nothing else.

A rename makes lines longer, and a line over 108 fails the scan. This re-fills only a paragraph
that has such a line, to 100, and never an indented block, a table, a list or a heading, which
are laid out by hand. Run from docs/: python ../tools/wrap.py [files]. Says what it changed.
"""
import glob
import sys
import textwrap

LIMIT, FILL = 108, 100

for path in sys.argv[1:] or glob.glob('**/*.md', recursive=True):
    raw = open(path, encoding='utf-8', newline='').read()
    nl = '\r\n' if '\r\n' in raw else '\n'
    lines = raw.replace('\r\n', '\n').split('\n')
    out, i, changed = [], 0, 0
    while i < len(lines):
        j = i
        while j < len(lines) and lines[j].strip():
            j += 1
        para = lines[i:j]
        plain = para and not any(l.startswith((' ', '\t', '#', '|', '-', '*   ', '>')) or l[:2].rstrip('.').isdigit()
                                 for l in para)
        if plain and any(len(l) > LIMIT for l in para):
            para = textwrap.wrap(' '.join(para), FILL, break_long_words=False, break_on_hyphens=False)
            changed += 1
        out += para
        if j < len(lines):
            out.append(lines[j])
        i = j + 1
    if changed:
        open(path, 'w', encoding='utf-8', newline='').write(nl.join(out))
        print('%s: %d paragraph%s re-wrapped' % (path, changed, '' if changed == 1 else 's'))
