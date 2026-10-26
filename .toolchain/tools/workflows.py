# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import re

from shared import arguments, config, ours, report

WORKFLOW = re.compile(r'\.github/workflows/[^/]+\.yml')
TARGET = re.compile(r'^\s*- target: (\S+)\s*$', re.M)


def faults(root, paths, targets):
    found = []
    for path in paths:
        named = TARGET.findall((root / path).read_text(encoding='utf-8'))
        if named:
            found += ['%s: the matrix does not build %s' % (path, target) for target in sorted(targets)
                      if target not in named]
            found += ['%s: %s is not a target of the toolchain' % (path, name) for name in named if name not in targets]
    return found


def main():
    root = arguments().root
    paths = sorted(path for path in ours(root) if WORKFLOW.fullmatch(path))
    report(faults(root, paths, set(config(root)['targets'])), len(paths), 'workflows')


if __name__ == '__main__':
    main()
