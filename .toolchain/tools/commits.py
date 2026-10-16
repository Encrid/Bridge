# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import re
import subprocess

from shared import arguments, die, report, repository

TYPES = ('feat', 'fix', 'refactor', 'chore', 'docs', 'test', 'build', 'perf', 'style', 'ci', 'revert')
LINE = re.compile(r'(\w+)(?:\([^()\s]+\))?: (\S.*)')
MOST = 20


def faults(message):
    line = LINE.fullmatch(message.split('\n')[0])
    found = ['the message has more than 1 line'] if '\n' in message else []
    if not line:
        found.append('the line is not type(scope): description')
    elif line.group(1) not in TYPES:
        found.append('the type %s is not in the list' % line.group(1))
    if line and len(line.group(2).split()) > MOST:
        found.append('the description has %d words, and %d is the most' % (len(line.group(2).split()), MOST))
    return found


def main():
    args = arguments('--commits')
    head = subprocess.run(['git', '-C', str(args.root), 'rev-parse', '--verify', '-q', 'HEAD'], capture_output=True)
    if args.commits is None and not (repository(args.root) and head.returncode == 0):
        return report([], 0, 'commits')
    span = [args.commits] if args.commits is not None else ['-1', 'HEAD']
    done = subprocess.run(['git', '-C', str(args.root), 'log', '-z', '--format=%h%n%B'] + span,
                          capture_output=True, text=True, encoding='utf-8')
    if done.returncode:
        die('git log %s: %s' % (' '.join(span), done.stderr.strip()))
    commits = [entry.strip('\n').partition('\n') for entry in done.stdout.split('\0') if entry.strip()]
    report(['%s: %s' % (name, fault) for name, _, message in commits for fault in faults(message.strip())],
           len(commits), 'commits')


if __name__ == '__main__':
    main()
