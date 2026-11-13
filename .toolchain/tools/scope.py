# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import sys
import tomllib

from shared import arguments, config, die, part_of

TOPS = ('src/', 'include/', 'docs/extern/', 'docs/intern/', 'tests/cases/', 'tests/')
README = 'docs/usage/'
CONFIG = '.toolchain/config/'


def stated(root, path):
    try:
        found = tomllib.loads((root / path).read_text(encoding='utf-8')).get('parts', {})
    except (OSError, ValueError):
        return None
    return next(iter(found)) if len(found) == 1 else None


def owner(settings, root, path):
    if path.startswith(CONFIG):
        return stated(root, path)
    if path.startswith(README) and path.endswith('.md'):
        return part_of(settings, path[len(README):-len('.md')])
    top = next((top for top in TOPS if path.startswith(top)), None)
    return part_of(settings, path[len(top):]) if top else None


def users(settings, named):
    found, grown = set(named), True
    while grown:
        more = {part for part, table in settings['parts'].items() if found & set(table.get('uses', []))}
        grown = not more <= found
        found |= more
    return found


def scoped(settings, root, paths, into):
    if into == 'main' or not settings.get('parts') or not paths:
        return []
    owners = [owner(settings, root, path) for path in paths]
    if None in owners:
        return []
    return sorted(users(settings, owners))


def main():
    args = arguments('--changed', '--into')
    if not args.changed:
        die('name the list of the changed paths with --changed')
    try:
        text = open(args.changed, encoding='utf-8').read()
    except OSError:
        die('cannot read the list %s' % args.changed)
    paths = [line.strip().replace('\\', '/') for line in text.splitlines() if line.strip()]
    parts = scoped(config(args.root), args.root, paths, args.into)
    if parts:
        sys.stdout.buffer.write((' '.join('--part %s' % part for part in parts) + '\n').encode('utf-8'))


if __name__ == '__main__':
    main()
