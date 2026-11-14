# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import re

from shared import arguments, config, hosts, ours, report

WORKFLOW = re.compile(r'\.github/workflows/[^/]+\.yml')
TARGET = re.compile(r'^\s*- target: (\S+)\s*$', re.M)
TOOLCHAIN = '.github/workflows/toolchain.yml'


def faults(root, paths, mine_hosts, every):
    found = []
    for path in paths:
        named = TARGET.findall((root / path).read_text(encoding='utf-8'))
        mine, whose = (every, 'toolchain') if path == TOOLCHAIN else (mine_hosts, 'project')
        if named:
            found += ['%s: the matrix does not run on %s' % (path, host) for host in sorted(mine)
                      if host not in named]
            found += ['%s: %s is not a host of the %s' % (path, name, whose) for name in named if name not in mine]
    return found


def main():
    root = arguments().root
    paths = sorted(path for path in ours(root) if WORKFLOW.fullmatch(path))
    report(faults(root, paths, set(hosts(config(root))), set(hosts(config(root, True)))), len(paths),
           'workflows')


if __name__ == '__main__':
    main()
