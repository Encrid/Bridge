# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import fnmatch
import sys
import tomllib

from shared import arguments, config, die, part_of

TOPS = ('src/', 'include/', 'tests/')
CONFIG = '.toolchain/config/'
CODE = TOPS + ('tools/',)
BUILT = (CONFIG, '.toolchain/tools/', '.toolchain/runtime/')
FILES = ('.toolchain/toolchain.toml', '.toolchain/project.toml', '.toolchain/graph.zig', 'project.py')


def stated(root, path):
    try:
        found = tomllib.loads((root / path).read_text(encoding='utf-8')).get('parts', {})
    except (OSError, ValueError):
        return None
    return next(iter(found)) if len(found) == 1 else None


def owner(settings, root, path):
    if path.startswith(CONFIG):
        return stated(root, path)
    top = next((top for top in TOPS if path.startswith(top)), None)
    return part_of(settings, path[len(top):]) if top else None


def groups(settings, root):
    folder = root / settings.get('test', {}).get('cases', 'tests/cases')
    found = []
    for path in sorted(folder.rglob('*.toml')):
        table = tomllib.loads(path.read_text(encoding='utf-8'))
        file, code, whole = path.relative_to(root).as_posix(), table.get('code'), table.get('whole', False)
        if code is not None and not (isinstance(code, list) and all(isinstance(item, str) for item in code)):
            die('%s: code is a list of patterns' % file)
        if not isinstance(whole, bool):
            die('%s: whole is true or false' % file)
        if table.get('case'):
            name = path.relative_to(folder).with_suffix('').as_posix()
            found.append({'name': name, 'file': file, 'code': code, 'whole': whole, 'part': part_of(settings, name)})
    return found


def tests(settings, root, found, group, path):
    if path in [other['file'] for other in found]:
        return path == group['file']
    if group['code'] is not None:
        return any(fnmatch.fnmatch(path, pattern) for pattern in group['code'])
    if group['whole']:
        return False
    built = path.startswith(CODE + (CONFIG,)) or path == '.toolchain/project.toml'
    return built and owner(settings, root, path) == group['part']


def claimed(settings, root, names):
    found = groups(settings, root)

    def measured(path):
        testers = [group['name'] for group in found if tests(settings, root, found, group, path)]
        return bool(testers) and all(name in names for name in testers)
    return measured


def scoped(settings, root, paths, into):
    found = groups(settings, root)
    wholes = [group['name'] for group in found if group['whole']]
    if not found or into == 'main':
        return wholes
    picked = set()
    for path in paths:
        mine = [group['name'] for group in found if tests(settings, root, found, group, path)]
        if not mine and path.startswith(CODE):
            die('%s: no case group tests this file' % path)
        if not mine and (path.startswith(BUILT) or path in FILES):
            mine = wholes
        picked.update(mine)
    return sorted(picked)


def main():
    args = arguments('--changed', '--into')
    if not args.changed:
        die('name the list of the changed paths with --changed')
    try:
        text = open(args.changed, encoding='utf-8').read()
    except OSError:
        die('cannot read the list %s' % args.changed)
    paths = [line.strip().replace('\\', '/') for line in text.splitlines() if line.strip()]
    found = scoped(config(args.root), args.root, paths, args.into)
    if found:
        sys.stdout.buffer.write((' '.join('--group %s' % group for group in found) + '\n').encode('utf-8'))


if __name__ == '__main__':
    main()
