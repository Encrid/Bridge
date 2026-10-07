#!/usr/bin/env python3
"""Every reference resolves: `doc.md`, *Section* to a real heading, and no
"item N" is left behind. A document of the design names no stage, and names
what is not built only where it is stated or in a citation of that. And no
tracked file of ours says a name that was retired.

Catches the failure the restructure makes likely: a pointer that looks right and
resolves to nothing. The item half exists because the register went when stage 0
ended, so a document citing an item points at text nobody can read - and the
number wraps onto the next line often enough that a line-at-a-time grep misses
it. Run from anywhere: it reads the documents where they are.
"""
import glob
import os
import re
import subprocess
import sys

# it reads the documents by their place under docs/, and is run from anywhere
os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'docs'))

# keyed by basename: prose cites `data.md`, not `bridge/data.md`, because a
# reader wants the document's name and not where it sits in the tree.
heads, paths = {}, {}
# contributing.md is the method, not the design: it cites closed items on
# purpose, as the worked examples that make each step concrete, and it carries
# a literal `doc.md`, *Section* template.
for full in glob.glob('**/*.md', recursive=True):
    if os.path.basename(full) == 'contributing.md':
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

# A document of the design names no stage, in any form: the plan points at the design, and the
# design never at the plan - stage14.md, *A stage in a document of the design*. What is held is
# every document that is not the plan's or a record, and the headers and the source. readme.md
# is the index of the tree, the plan's folder with the rest, and style.md gives a stage's file
# as an example of a name: neither says what the library is.
STAGE = re.compile(r'[Ss]tages? \d|stage\d+\.md')
APART = ('plan/', 'refused/')
INDEX = ('plan.md', 'readme.md', 'style.md')


def is_design(path):
    return not path.startswith(APART) and path not in INDEX


for f in sorted(paths):
    full = paths[f].replace(os.sep, '/')
    if not is_design(full):
        continue
    for n, line in enumerate(open(full, encoding='utf-8'), 1):
        if STAGE.search(line):
            print('%-14s:%d names a stage' % (f, n))
            bad += 1
for full in sorted(glob.glob('../include/*.h') + glob.glob('../src/*.[ch]') +
                   glob.glob('../backends/*/*.[ch]') + glob.glob('../backends/*/sdk/*') +
                   glob.glob('../utils/**/*.[ch]', recursive=True)):
    for n, line in enumerate(open(full, encoding='utf-8'), 1):
        if STAGE.search(line):
            print('%s:%d names a stage' % (full[3:].replace(os.sep, '/'), n))
            bad += 1

# What is designed and not built is named where it is stated, under a heading that opens by
# saying so, and anywhere else only in a paragraph that cites that heading - stage14.md, *What
# is said of what is not built*. The stage that builds one takes its row out of here.
UNBUILT = [
    (r'bridge_profile\w*|\[checks\] profile', [('profile.md', 'the profile')]),
    (r'bridge_manifest_parse', [('module.md', 'the parser of a manifest')]),
    (r'`validate`|\[checks\] validate', [('validation.md', 'the checksum pass, when asked')]),
    (r'\[checks\]', [('profile.md', 'the profile'),
                      ('validation.md', 'the checksum pass, when asked')]),
]


def cites(text, here, owners):
    """Whether a paragraph cites one of the headings that state a thing."""
    flat = re.sub(r'\s+', ' ', text).lower()
    for doc, head in owners:
        if '%s`, *%s*' % (doc, head) in flat or '%s, *%s*' % (doc, head) in flat:
            return True
        if here == doc and '*%s*' % head in flat:
            return True
    return False


def paragraphs(full, is_code):
    """Each paragraph of a document with the heading it is under, or each comment of a
    source."""
    text = open(full, encoding='utf-8').read().replace('\r\n', '\n')
    if is_code:
        for m in re.finditer(r'/\*.*?\*/', text, re.S):
            yield text.count('\n', 0, m.start()) + 1, '', re.sub(r'\n\*\*[ ]?', '\n', m.group(0))
        return
    head, line = '', 1
    for para in text.split('\n\n'):
        m = re.match(r'\n*#{1,4}\s+(.*?)\s*$', para)
        if m:
            head = m.group(1).lower()
        yield line, head, para
        line += para.count('\n') + 2


def check_unbuilt(full, name, is_code):
    found = 0
    for line, head, para in paragraphs(full, is_code):
        for pattern, owners in UNBUILT:
            if not re.search(pattern, para):
                continue
            if (name, head) in owners or cites(para, name, owners):
                continue
            print('%-14s:%d names what is not built, and cites no heading that states it'
                  % (name, line))
            found += 1
            break
    return found


for f in sorted(paths):
    full = paths[f].replace(os.sep, '/')
    if is_design(full):
        bad += check_unbuilt(full, f, False)
for full in sorted(glob.glob('../include/*.h') + glob.glob('../backends/*/sdk/*.h')):
    bad += check_unbuilt(full, full[3:].replace(os.sep, '/'), True)

# what is not built opens by saying so
for pattern, owners in UNBUILT:
    for doc, head in owners:
        text = open(paths[doc], encoding='utf-8').read().replace('\r\n', '\n').lower()
        at = re.search(r'^#{1,4} %s\n\n(.*?)\n' % re.escape(head), text, re.M)
        if not at or not at.group(1).startswith('it is not built.'):
            print('%-14s -> "%s" states what is not built, and does not open by saying so'
                  % (doc, head))
            bad += 1

# Three names were retired, and none comes back - stage15.md, *What holds it*: the name the
# library was built under, what the design before it was called, and the object a host
# creates called by the library's own word. Read in every tracked file of ours that is text.
# The record of the stage that retired them says what each was, and is not read.
RETIRED = [
    (re.compile('ASM' + '700'), 'the name the library was built under'),
    (re.compile(r'\b[Oo]riginal\s+Bridge\b'), 'what the design before this one was called'),
    (re.compile(r"\b(?:[Tt]he|[Aa]|[Aa]n|[Tt]wo|[Nn]o|[Ee]ach|[Ee]very|that|this|its|stopped|"
                r"host's|fixture's)\s+bridges?\b(?![-_/(]|[.:]\w)"),
     'what a host creates, which is a deck'),
]
RECORD = ('docs/plan/stage15.md', 'docs/refused/plan/stage15_refused.md')
NOT_OURS = 'backends/wasm/wasmtime/'

listed = subprocess.run(['git', 'ls-files'], cwd='..', stdout=subprocess.PIPE).stdout.decode()
for path in listed.split('\n'):
    if not path or path.startswith(NOT_OURS) or path in RECORD:
        continue
    raw = open(os.path.join('..', path), 'rb').read()
    if b'\0' in raw[:4096]:
        continue
    text = raw.decode('utf-8', 'replace')
    for word, what in RETIRED:
        for m in word.finditer(text):
            print('%s:%d says %s' % (path, text.count('\n', 0, m.start()) + 1, what))
            bad += 1

print('%d broken' % bad)
sys.exit(1 if bad else 0)
