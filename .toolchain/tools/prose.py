# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import re

from shared import arguments, config, ours, project_name, report

TAGS = ('pseudo', 'c', 'toml', 'sh', 'text', 'mermaid')
ORDER = {'Table of Contents': 0, 'Security': 1, 'Background': 2, 'Install': 3, 'Usage': 4, 'API': 6, 'Maintainer': 7,
         'Maintainers': 7, 'Thanks': 8, 'Credits': 8, 'Acknowledgements': 8, 'Contributing': 9, 'License': 10}
EXTRA = 5
NEEDED = ('Install', 'Usage', 'Contributing', 'License')
FENCE = re.compile(r'\s*```(.*)')
ITEM = re.compile(r'\s*(?:[-*+]|(\d+)\.)\s+(.*)')
SPAN = re.compile(r'`[^`\n]*`')
SENTENCE = re.compile(r'\S.*?(?:[.!?](?=\s|\Z)|\Z)', re.S)
WORD = re.compile(r'\S*[A-Za-z0-9]\S*')


def documents(root):
    return [(path, (root / path).read_text(encoding='utf-8', errors='replace').split('\n')) for path in ours(root)
            if path == 'readme.md' or re.fullmatch(r'(?:docs|\.toolchain/docs)/.+\.md', path)]


def blocks(lines):
    found, start = [], None
    for n, line in enumerate(lines):
        fence = FENCE.match(line)
        if fence and start is None:
            start, tag = n, fence.group(1).strip()
        elif fence:
            found.append((start, n, tag))
            start = None
    return found


def units(lines):
    found, unit = [], None
    for n, line in enumerate(lines, 1):
        bare = SPAN.sub('code', line)
        item = ITEM.match(bare)
        if not bare.strip() or bare.startswith('#'):
            unit = None
        elif bare.lstrip().startswith('|'):
            found += [[n, cell, 'cell'] for cell in bare.strip().strip('|').split('|')]
            unit = None
        elif item:
            unit = [n, item.group(2), 'step' if item.group(1) else 'item']
            found.append(unit)
        elif unit and (unit[2] == 'prose' or line.startswith(' ')):
            unit[1] += '\n' + bare
        else:
            unit = [n, bare, 'prose']
            found.append(unit)
    return found


def faults(lines, limits):
    found, text = [], list(lines)
    for start, end, tag in blocks(lines):
        text[start:end + 1] = [''] * (end + 1 - start)
        if tag not in TAGS:
            found.append((start + 1, "the tag '%s' is not one of %s" % (tag, ' '.join(TAGS))))
    for n, unit, kind in units(text):
        most = limits['step'] if kind == 'step' else limits['sentence']
        sentences = list(SENTENCE.finditer(unit))
        for m in sentences:
            words = len(WORD.findall(m.group()))
            if words > most:
                found.append((n + unit.count('\n', 0, m.start()), '%d words in a sentence, and %d is the most'
                              % (words, most)))
        if len(sentences) > limits['paragraph']:
            found.append((n, '%d sentences in a paragraph, and %d is the most' % (len(sentences), limits['paragraph'])))
    return found


def readme(lines, name, most):
    found = [] if lines[0] == '# ' + name else [(1, 'the title is not # ' + name)]
    rest = [(n, line) for n, line in enumerate(lines[1:], 2) if line.strip()]
    if not rest or rest[0][1].startswith('#'):
        found.append((1, 'no description after the title'))
    elif len(rest[0][1]) > most:
        found.append((rest[0][0], '%d characters in the description, and %d is the most' % (len(rest[0][1]), most)))
    sections = [(n, line[3:].strip()) for n, line in enumerate(lines, 1) if line.startswith('## ')]
    last = (-1, '')
    for n, title in sections:
        rank = ORDER.get(title, EXTRA)
        if rank < last[0]:
            found.append((n, 'the section %s must come before %s' % (title, last[1])))
        else:
            last = (rank, title)
    titles = {title for _, title in sections}
    return found + [(1, 'no section ' + title) for title in NEEDED if title not in titles]


def main():
    root = arguments().root
    settings = config(root)
    read = documents(root)
    found = []
    for path, lines in read:
        found += [(path, n, what) for n, what in faults(lines, settings['prose'])]
        if path == 'readme.md':
            found += [(path, n, what) for n, what in readme(lines, project_name(settings),
                                                           settings['prose']['description'])]
    report(['%s:%d: %s' % fault for fault in sorted(found)], len(read))


if __name__ == '__main__':
    main()
