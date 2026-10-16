# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import re

from shared import arguments, ours, report

FOLDERS = ('.github', '.toolchain', 'docs', 'include', 'src', 'tests', 'third_party', 'tools')
FENCE = re.compile(r'^```.*?^```', re.S | re.M)
CITED = re.compile(r'`([^`\n]+)`, \*([^*\n]+)\*')
SPAN = re.compile(r'`([^`\n]+)`')
LOCAL = re.compile(r'(?<![*\w])\*(?![*\s])([^*\n]+?)(?<!\s)\*(?![*\w])')
PATH = re.compile(r'(?:[\w.-]+/)+[\w-]*\.[\w.-]+')


def blank(match):
    return re.sub(r'[^\n]', ' ', match.group(0))


def headings(text):
    return set(re.findall(r'^#{1,6} (.+?)[ \t]*$', FENCE.sub(blank, text), re.M))


def faults(root, docs, doc):
    raw = (root / doc).read_text(encoding='utf-8')
    text = re.sub(r'\n(?=\S)', ' ', FENCE.sub(blank, raw))
    found = []
    for m in CITED.finditer(text):
        path, heading = m.groups()
        if not (root / path).is_file():
            found.append((m.start(), '%s is not a file of the project' % path))
        elif heading not in docs.get(path, ()):
            found.append((m.start(), '%s has no heading %s' % (path, heading)))
    text = CITED.sub(blank, text)
    for m in SPAN.finditer(text):
        path = m.group(1)
        if PATH.fullmatch(path) and path.split('/')[0] in FOLDERS and not (root / path).is_file():
            found.append((m.start(), '%s is not a file of the project' % path))
    for m in LOCAL.finditer(SPAN.sub(blank, text)):
        if m.group(1) not in docs[doc]:
            found.append((m.start(), 'this document has no heading %s' % m.group(1)))
    return ['%s:%d: %s' % (doc, raw.count('\n', 0, at) + 1, what) for at, what in sorted(found)]


def main():
    root = arguments().root
    docs = {path: headings((root / path).read_text(encoding='utf-8')) for path in ours(root) if path.endswith('.md')}
    report([line for doc in sorted(docs) for line in faults(root, docs, doc)], len(docs))


if __name__ == '__main__':
    main()
