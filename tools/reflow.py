#!/usr/bin/env python3
"""Rewrap a document's prose paragraphs at 100 columns.

Fix the block, not the line. Rewrapping one overlong line pushes the overflow
onto the next one, which is a loop that costs more turns than it saves.

Indented blocks, headings and rules are left exactly as they are. Usage:

    python ../tools/reflow.py OPEN.md
"""
import io
import sys
import textwrap

WIDTH = 100

for path in sys.argv[1:]:
    blocks, cur = [], []
    for line in io.open(path, encoding='utf-8').read().split('\n'):
        if line.strip() == '':
            blocks.append(cur)
            cur = []
        else:
            cur.append(line)
    blocks.append(cur)

    out = []
    for b in blocks:
        if not b:
            out.append('')
            continue
        if b[0].startswith('    ') or b[0].startswith('#') or b[0].startswith('---'):
            out.extend(b)
            out.append('')
            continue
        joined = ' '.join(x.strip() for x in b)
        out.extend(textwrap.wrap(joined, width=WIDTH,
                                 break_long_words=False, break_on_hyphens=False))
        out.append('')
    while out and out[-1] == '':
        out.pop()

    io.open(path, 'w', encoding='utf-8', newline='\n').write('\n'.join(out) + '\n')
    print('reflowed', path)
