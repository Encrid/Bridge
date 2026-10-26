# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import subprocess
import sys

from shared import HERE, arguments, die

TOOLS = ('layout', 'workflows', 'text', 'prose', 'pseudo', 'xref', 'dup', 'cover', 'licence', 'commits', 'style', 'c23')


def main():
    args = arguments('--commits')
    failed = []
    for tool in TOOLS:
        print(tool + '.py', flush=True)
        more = ['--commits', args.commits] if tool == 'commits' and args.commits else []
        program = HERE / '.toolchain' / 'tools' / (tool + '.py')
        done = subprocess.run([sys.executable, str(program), '--root', str(args.root)] + more)
        if done.returncode:
            failed.append(tool)
    if failed:
        die('failed: ' + ' '.join(failed))
    print('every check passes')


if __name__ == '__main__':
    main()
