#!/usr/bin/env python3
"""The licence holds: docs/licence.md, *What Bridge is under*. It is the case licence.

    the text         `license` is the GNU Affero General Public License, version 3, as its
                     authors publish it, byte for byte: its hash is the one below
    every file       no tracked file of ours says that all rights are reserved. The header a
                     source file opens with is held by tools/style.py, rule G1
    somebody         each folder that holds a licence of its own, and each file whose header
    else's           names a holder beside ours, is named in docs/licence.md
    the agreement    cla.md is there, and docs/licence.md names it
    the seams        each file of include/ says, where it opens, that
                     what is compiled with it is a work based on Bridge, under the same licence

The records of the stage that brought the licence say what the files said before, and are not
read. Run from anywhere; exits non-zero on anything that does not hold.
"""
import hashlib
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
# https://www.gnu.org/licenses/agpl-3.0.txt, as fetched on 2026-10-07: 661 lines, 34523 bytes
PUBLISHED = '0d96a4ff68ad6d4b6f1f30f713b18d5184912ba8dd389f86aa7710db079abcb0'
RESERVED = re.compile('All rights' + r'\s+reserved', re.I)
RECORD = ('docs/plan/', 'docs/refused/plan/')
bad = 0


def fault(said):
    global bad
    print(said)
    bad += 1


text = ROOT / 'license'
if not text.exists():
    fault('license is not there')
elif hashlib.sha256(text.read_bytes()).hexdigest() != PUBLISHED:
    fault('license is not the published text of the licence, byte for byte')

listed = [p for p in subprocess.run(['git', 'ls-files'], cwd=ROOT, capture_output=True,
                                    text=True).stdout.split('\n') if p]
theirs = sorted({p.rsplit('/', 1)[0] + '/' for p in listed
                 if re.search(r'/(?:LICENSE|LICENCE|COPYING)[^/]*\Z', p)})
said = (ROOT / 'docs/licence.md').read_text(encoding='utf-8') if (ROOT / 'docs/licence.md').exists() else ''
if not said:
    fault('docs/licence.md is not there')
if not (ROOT / 'cla.md').exists():
    fault('cla.md is not there')
elif '`cla.md`' not in said:
    fault('docs/licence.md does not name cla.md')
for folder in theirs:
    if '`%s`' % folder not in said:
        fault('docs/licence.md does not name %s, which holds a licence of its own' % folder)
for path in listed:
    if path.startswith(tuple(theirs)) or path.startswith(RECORD) or path == 'license':
        continue
    raw = (ROOT / path).read_bytes()
    if b'\0' in raw[:4096]:
        continue
    body = raw.decode('utf-8', 'replace')
    m = RESERVED.search(body)
    if m:
        fault('%s:%d says that all rights are reserved' % (path, body.count('\n', 0, m.start()) + 1))
    head = body[:600]
    if path.endswith(('.c', '.h')) and re.search(r'^\*\* Copyright \(C\) (?!\d{4} Bridge Owner\s*$)', head, re.M):
        if '`%s`' % path not in said:
            fault('docs/licence.md does not name %s, whose header names another holder' % path)

for path in listed:
    if re.match(r'include/[^/]+\Z', path):
        opens = (ROOT / path).read_text(encoding='utf-8')[:1500]
        if not re.search(r'is a work based on Bridge and is under\s+(?:\*\* |;; )?the same licence', opens):
            fault('%s does not say, where it opens, what is compiled with it is under' % path)

print('the licence: %d faults' % bad)
sys.exit(1 if bad else 0)
