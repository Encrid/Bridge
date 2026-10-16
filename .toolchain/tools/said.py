# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import re
from pathlib import Path

from shared import arguments, config, die, ours, report, this_target, variables

SPEC = r'%[-+ #0]*(?:\*|\d+)?(?:\.(?:\*|\d+))?(?:hh|h|ll|l|z|j|t)?[diouxXcsfgeEp]'
LITERAL = r'"((?:[^"\\\n]|\\.)*)"'
OWN = 6


def called(text, at):
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


def messages(text, writers):
    for m in re.finditer(r'\b(?:%s)\s*\(' % '|'.join(map(re.escape, writers)), text):
        run = re.search(r'%s(?:\s*%s)*' % (LITERAL, LITERAL), called(text, m.end() - 1))
        if run:
            said = ''.join(re.findall(LITERAL, run.group(0)))
            said = said.replace('\\"', '"').replace('\\n', '\n').replace('\\\\', '\\')
            yield text.count('\n', 0, m.start()) + 1, said.strip()


def pattern(message):
    parts = re.split(SPEC, message)
    return '[^\n]*?'.join(re.escape(part.replace('%%', '%')) for part in parts), sum(len(part) for part in parts)


def checked(root, out):
    said = out / 'said.txt'
    if not said.is_file():
        die('%s is not there: run test first' % said.as_posix())
    written = said.read_text(encoding='utf-8')
    files = sorted(ours(root))
    quoted = '\n'.join((root / path).read_text(encoding='utf-8') for path in files
                       if path == 'readme.md' or path.startswith('docs/extern/'))
    writers = config(root).get('test', {}).get('writers', [])
    code = [path for path in files if writers and path.startswith(('src/', 'include/')) and path.endswith(('.c', '.h'))]
    found, count = [], 0
    for path in code:
        for line, message in messages((root / path).read_text(encoding='utf-8'), writers):
            wanted, own = pattern(message)
            if own < OWN:
                continue
            count += 1
            if not re.search(wanted, written):
                found.append('%s:%d: no case wrote: %s' % (path, line, message))
            if not re.search(wanted, quoted):
                found.append('%s:%d: no document quotes: %s' % (path, line, message))
    report(found, count, 'messages')


def main():
    args = arguments()
    settings = config(args.root)
    checked(args.root, Path(variables(settings, args, args.target or this_target(settings['targets']), '')['out']))


if __name__ == '__main__':
    main()
