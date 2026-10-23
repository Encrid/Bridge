# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import argparse
import hashlib
import os
import platform
import re
import subprocess
import sys
import tomllib
from pathlib import Path

HERE = Path(__file__).resolve().parents[2]
NAME = re.compile(r'\{([a-z_][a-z0-9_]*)(?::([^{}]*))?\}')
KEYS = {'step': {'run', 'section', 'text', 'contains', 'copy', 'rename'},
        'case': {'name', 'run', 'env', 'stdin', 'script', 'helper', 'files', 'folders', 'claims', 'false', 'alone'},
        'false': {'run', 'env', 'stdin', 'script', 'helper', 'files', 'folders'},
        'helper': {'run', 'wait', 'stdin', 'claims'},
        'script': {'wait', 'put', 'remove', 'type'},
        'claims': {'status', 'status_not', 'line', 'has', 'lacks', 'matches', 'matches_not', 'order', 'output', 'files'},
        'source': {'name', 'url', 'sha256', 'into', 'top', 'patches'}}
SIDES = {'include': 'docs/extern', 'src': 'docs/intern', 'tools': 'docs/tools'}


def die(why):
    sys.exit('%s: %s' % (Path(sys.argv[0]).name, why))


def known(table, what, where):
    unknown = sorted(set(table) - KEYS[what])
    if unknown:
        die('%s: a %s has no key %s' % (where, what, ' '.join(unknown)))
    return table


def merged(into, more, above=''):
    for key, value in more.items():
        if isinstance(into.get(key), dict) and isinstance(value, dict):
            merged(into[key], value, above + key + '.')
        elif key in into:
            die('project.toml states %s%s, and toolchain.toml states it too' % (above, key))
        else:
            into[key] = value
    return into


def config(root):
    toolchain, project = [tomllib.loads(path.read_text(encoding='utf-8'))
                          for path in (HERE / '.toolchain' / 'toolchain.toml', root / '.toolchain' / 'project.toml')]
    whole = merged(toolchain, project)
    for path in sorted((root / '.toolchain' / 'config').glob('*.toml')):
        merged(whole, tomllib.loads(path.read_text(encoding='utf-8')))
    return whole


def located(text, values):
    return Path(values['root']) / expand(text, values)


def expand(text, values):
    def one(m):
        if m.group(1) not in values:
            die('no variable {%s} for %s' % (m.group(1), text))
        value = values[m.group(1)]
        if not isinstance(value, list):
            return expand(str(value), values)
        if m.group(2) is None:
            die('{%s} is a list inside a text: write {%s:<separator>}' % (m.group(1), m.group(1)))
        return m.group(2).join(expanded(value, values))
    return NAME.sub(one, text)


def expanded(items, values):
    out = []
    for item in items:
        whole = NAME.fullmatch(item)
        if whole and whole.group(2) is None and isinstance(values.get(whole.group(1)), list):
            out += expanded(values[whole.group(1)], values)
        else:
            out.append(expand(item, values))
    return out


def files(root, patterns):
    found = [p.relative_to(root).as_posix() for pattern in patterns
             for p in sorted(root.glob(pattern)) if p.is_file()]
    return list(dict.fromkeys(found))


def status(code):
    return '%d (0x%08X)' % (code, code & 0xFFFFFFFF)


def run(argv, root):
    done = subprocess.run(argv, cwd=root, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    said = done.stdout.decode('utf-8', 'replace').replace('\r\n', '\n')
    if done.returncode:
        die('%s\n  ended with %s\n%s' % (' '.join(argv), status(done.returncode), said))
    sys.stdout.write(said)


def this_target(targets):
    machine = [platform.system(), platform.machine()]
    return next((name for name, row in targets.items() if row['runs_on'] == machine), None)


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def variables(config, args, target, zig):
    build = (args.out or args.root / config['build']['folder']).resolve()
    sub = target + ('-debug' if args.debug else '')
    values = {'root': args.root.as_posix(), 'build': build.as_posix(), 'home': this_target(config['targets']) or '',
              'dist': (build / sub).as_posix(), 'out': (build / sub / 'scratch').as_posix(), 'target': target,
              'python': Path(sys.executable).as_posix(), 'zig': zig}
    values.update(config['targets'][target])
    own = config.get('variables', {})
    if set(own) & set(values):
        die('[variables] names a built-in variable or a key of a target: ' + ' '.join(sorted(set(own) & set(values))))
    values.update(own)
    if args.debug:
        values.update(config.get('debug', {}))
    for setting in args.set:
        name, _, value = setting.partition('=')
        if name not in own:
            die('--set %s: [variables] has no %s' % (setting, name))
        values[name] = value
    return values


def arguments(*valued, **flags):
    ap = argparse.ArgumentParser(prog=Path(sys.argv[0]).name)
    ap.add_argument('--root', type=Path, default=HERE)
    ap.add_argument('--target')
    ap.add_argument('--part', action='append')
    ap.add_argument('--debug', action='store_true')
    ap.add_argument('--out', type=Path)
    ap.add_argument('--set', action='append', default=[], metavar='NAME=VALUE')
    for option in valued:
        name, kind = option if isinstance(option, tuple) else (option, str)
        ap.add_argument(name, type=kind)
    for name, flag in flags.items():
        ap.add_argument(flag, dest=name, action='store_true')
    args = ap.parse_args()
    args.root = args.root.resolve()
    return args


def header(mark):
    pattern = ''
    for line in (HERE / '.toolchain' / 'header.txt').read_text(encoding='utf-8').splitlines():
        text = re.escape((mark + ' ' + line).rstrip())
        text = text.replace('<year>', r'\d{4}(?:-\d{4})?').replace('<holder>', r'[^\n]+')
        pattern += '(?:%s\n)+' % text if '<holder>' in line else text + '\n'
    return pattern


def project_name(config):
    return config.get('project', {}).get('name') or die('.toolchain/project.toml: write the name of the project in [project] name')


def document(path):
    top, _, rest = path.partition('/')
    if top not in SIDES or path.endswith('/.gitkeep'):
        return None
    return '%s/%s.md' % (SIDES[top], rest)


def repository(root):
    top = subprocess.run(['git', '-C', str(root), 'rev-parse', '--show-toplevel'], capture_output=True, text=True)
    return not top.returncode and Path(top.stdout.strip()).resolve() == Path(root).resolve()


def ours(root, tests=False):
    if repository(root):
        found = subprocess.run(['git', 'ls-files', '-co', '--exclude-standard'], cwd=root, capture_output=True,
                               text=True).stdout.split('\n')
    else:
        found = []
        for folder, subfolders, names in os.walk(root):
            subfolders[:] = [name for name in subfolders
                             if name != '__pycache__' and not (name == 'build' and Path(folder) == root)]
            found += [(Path(folder) / name).relative_to(root).as_posix() for name in names]
    left_out = ('.toolchain/tests/',) if tests else ('tests/', '.toolchain/tests/')
    return [path for path in found if path and not path.startswith(left_out) and (root / path).is_file()]


def report(faults, count, items='files'):
    for fault in faults:
        print(fault)
    if faults:
        print('faults: %d' % len(faults))
        sys.exit(1)
    print('no fault in %d %s' % (count, items))
