# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import re

from shared import arguments, ours, report, sha256

PUBLISHED = '0d96a4ff68ad6d4b6f1f30f713b18d5184912ba8dd389f86aa7710db079abcb0'
RESERVED = re.compile('All rights' + r'\s+reserved', re.I)


def faults(root, files):
    readme = (root / 'readme.md').read_text(encoding='utf-8') if 'readme.md' in files else ''
    sources = sorted({path.split('/')[1] for path in files if path.startswith('third_party/') and path.count('/') > 1})
    notices = ['third_party/%s/license' % source for source in sources]
    found = []
    if 'license' not in files:
        found.append('license is not there')
    elif sha256((root / 'license').read_bytes()) != PUBLISHED:
        found.append('license is not the published text of the licence')
    for path in sorted(files):
        raw = (root / path).read_bytes()
        if path == 'license' or path.startswith('third_party/') or b'\0' in raw[:4096]:
            continue
        text = raw.decode('utf-8', 'replace')
        reserved = RESERVED.search(text)
        if reserved:
            found.append('%s:%d: says that all rights are reserved' % (path, text.count('\n', 0, reserved.start()) + 1))
    found += ['%s is not there' % notice for notice in notices if notice not in files]
    if 'cla.md' not in files:
        found.append('cla.md is not there')
    found += ['readme.md does not name %s' % name for name in ['cla.md'] + notices if '`%s`' % name not in readme]
    return found


def main():
    root = arguments().root
    files = set(ours(root))
    report(faults(root, files), len(files))


if __name__ == '__main__':
    main()
