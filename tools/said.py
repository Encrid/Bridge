#!/usr/bin/env python3
"""Every message the library and its backends can write was written by some case.

    python tools/said.py <out>/said

The messages are read from the source: each call of a function that writes one, by its format
string. What the cases said is read from the folder `build.py test` leaves, a file a case. A
message is held if some case's output has a line it matches, with anything where the format has
a conversion. One that no case made appear is named by its file and line, and fails.

A format that is nearly all conversion, "%s", passes on a message written elsewhere: it has no
words of its own to look for, and is counted apart.
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SOURCES = ('src/*.c', 'backends/wasm/*.c', 'backends/native/*.c')
WRITERS = ('abi_refuse', 'module_refuse', 'reload_refuse', 'say_error', 'say_about', 'module_fail',
           'config_refuse', 'schema_refuse', 'wasm_fail', 'snprintf')
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
    for line in missing:
        print(line)
    print('%d messages held, %d that pass one on, %d that no case made appear'
          % (held, passed_on, len(missing)))
    return 1 if missing else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1]))
