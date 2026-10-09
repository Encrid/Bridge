#!/usr/bin/env python3
"""Every message the library and its backends can write was written by some case.

    python tools/said.py <out>/said

The messages are read from the source: each call of a function that writes one, by its format
string. What the cases said is read from the folder `build.py test` leaves, a file a case. A
message is held if some case's output has a line it matches, with anything where the format has
a conversion. One that no case made appear is named by its file and line, and fails.

A format that is nearly all conversion, "%s", passes on a message written elsewhere: it has no
words of its own to look for, and is counted apart.

And every function that writes a message says which document states the rule it enforces: the
comment above it cites a document and a heading, as `budgets.md`, *Budgets*, and one that cites
none fails. No comment stands inside a function, so the function is what cites. tools/xref.py
holds that the heading is there.

    python tools/said.py --cited     only that, with no case run
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SOURCES = ('src/*.c',)
WRITERS = ('abi_refuse', 'module_refuse', 'reload_refuse', 'say_error', 'say_about', 'module_fail',
           'config_refuse', 'schema_refuse', 'snprintf')
SPEC = r'%[-+ #0]*(?:\*|\d+)?(?:\.(?:\*|\d+))?(?:hh|h|ll|l|z|j|t)?[diouxXcsfgeEp]'
LITERAL = r'"((?:[^"\\\n]|\\.)*)"'
WORDS = 6


def arguments(text, at):
    """What a call is handed: the text between its brackets, from the one that opens at `at`."""
    depth, i, quoted = 0, at, False
    while True:
        c = text[i]
        if quoted:
            i += 1 if c == '\\' else 0
            quoted = c != '"'
        elif c == '"':
            quoted = True
        elif c == '(':
            depth += 1
        elif c == ')':
            depth -= 1
            if not depth:
                return text[at + 1:i]
        i += 1


def formats(path):
    """Each message a source can write: its line and its format string."""
    text = path.read_text(encoding='utf-8')
    found = []
    for m in re.finditer(r'\b(%s)\s*\(' % '|'.join(WRITERS), text):
        args = arguments(text, m.end() - 1)
        if m.group(1) == 'snprintf' and 'err->text' not in args.split(',')[0]:
            continue
        run = re.search(r'%s(?:\s*%s)*' % (LITERAL, LITERAL), args)
        if not run or text[:m.start()].rstrip().endswith(('int', 'void', ']]')):
            continue
        said = ''.join(re.findall(LITERAL, run.group(0)))
        said = said.replace('\\"', '"').replace('\\n', '\n').replace('\\\\', '\\')
        found.append((text.count('\n', 0, m.start()) + 1, said))
    return found


def functions(text):
    """Each function of a source: the line it begins at, the line after its last, its name,
    and the comment above it or ''."""
    starts = [m for m in re.finditer(r'^[A-Za-z_][^\n;]*\n?[^\n;{]*\)\n\{', text, re.M)]
    found = []
    for k, m in enumerate(starts):
        end = starts[k + 1].start() if k + 1 < len(starts) else len(text)
        before = text[:m.start()].rstrip()
        comment = before[before.rfind('/*'):] if before.endswith('*/') else ''
        found.append((text.count('\n', 0, m.start()) + 1, text.count('\n', 0, end) + 1,
                      re.findall(r'(\w+)\(', m.group(0))[0], comment))
    return found


def uncited():
    """Each function that writes a message under a comment that cites no document."""
    found = []
    for path in sorted(p for pat in SOURCES for p in ROOT.glob(pat)):
        text = path.read_text(encoding='utf-8')
        lines = [line for line, said in formats(path) if pattern(said)[1] >= WORDS]
        for first, last, name, comment in functions(text):
            cited = re.search(r'\w+\.md,\s+\*[^*]+\*', re.sub(r'\n\*\*\s*', ' ', comment))
            if not cited and [l for l in lines if first <= l < last]:
                found.append('%s:%d: %s writes a message, and its comment cites no document'
                             % (path.relative_to(ROOT).as_posix(), first, name))
    return found


def pattern(said):
    """A format as what it matches, and how many characters of it are its own words."""
    parts = re.split(SPEC, said)
    own = sum(len(p) for p in parts)
    return '[^\n]*?'.join(re.escape(p.replace('%%', '%')) for p in parts), own


def main(folder):
    files = sorted(Path(folder).glob('*.txt'))
    if not files:
        sys.exit('said: %s holds what no case said: run every case first' % folder)
    everything = '\n'.join(f.read_text(encoding='utf-8') for f in files)
    held = passed_on = 0
    missing = []
    for path in sorted(p for pat in SOURCES for p in ROOT.glob(pat)):
        for line, said in formats(path):
            wanted, own = pattern(said)
            if own < WORDS:
                passed_on += 1
            elif re.search(wanted, everything):
                held += 1
            else:
                missing.append('%s:%d: no case made it say: %s'
                               % (path.relative_to(ROOT).as_posix(), line, said))
    bare = uncited()
    for line in missing + bare:
        print(line)
    print('%d messages held, %d that pass one on, %d that no case made appear, %d functions that '
          'cite no document' % (held, passed_on, len(missing), len(bare)))
    return 1 if missing or bare else 0


if __name__ == '__main__':
    if sys.argv[1:] == ['--cited']:
        for line in uncited():
            print(line)
        sys.exit(0)
    sys.exit(main(sys.argv[1]))
