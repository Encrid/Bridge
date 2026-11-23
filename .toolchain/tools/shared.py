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
import threading
import tomllib
from pathlib import Path

HERE = Path(__file__).resolve().parents[2]
WAIT = 0.2
NAME = re.compile(r'\{([a-z_][a-z0-9_]*)(?::([^{}]*))?\}')
KEYS = {'step': {'run', 'cwd', 'section', 'text', 'contains', 'copy', 'rename'},
        'case': {'name', 'run', 'env', 'stdin', 'script', 'helper', 'files', 'folders', 'fail', 'fail_parts', 'hold', 'claims', 'false', 'alone',
                 'ignore', 'rows', 'sweep', 'process', 'systems'},
        'row': {'name', 'in', 'claims', 'false'},
        'claim of a row': {'line', 'has', 'lacks', 'matches', 'matches_not', 'order', 'output'},
        'false': {'run', 'env', 'stdin', 'script', 'helper', 'files', 'folders', 'fail', 'fail_parts', 'hold', 'ignore'},
        'helper': {'run', 'wait', 'stdin', 'claims'},
        'script': {'wait', 'put', 'remove', 'type', 'close', 'signal', 'pause'},
        'claims': {'status', 'status_not', 'line', 'has', 'lacks', 'matches', 'matches_not', 'order', 'output', 'files'},
        'source': {'name', 'url', 'sha256', 'into', 'top', 'patches'},
        'part': {'name', 'folder', 'uses', 'third_party', 'package'},
        'kind': {'takes', 'gives'}}
SIDES = {'include': 'docs/extern', 'src': 'docs/intern', 'tools': 'docs/tools'}
TAG = re.compile(r'(?:([a-z0-9_]+)-)?v\d+\.\d+\.\d+')
UNSET = {}
sys.stdout.reconfigure(encoding='utf-8')
sys.stderr.reconfigure(encoding='utf-8')


def die(why):
    sys.exit('%s: %s' % (Path(sys.argv[0]).name, why))


def known(table, what, where):
    unknown = sorted(set(table) - KEYS[what])
    if unknown:
        die('%s: a %s has no key %s' % (where, what, ' '.join(unknown)))
    return table


def merged(into, more, name, whence, above=()):
    for key, value in more.items():
        at = above + (key,)
        if isinstance(into.get(key), dict) and isinstance(value, dict):
            merged(into[key], value, name, whence, at)
        elif isinstance(into.get(key), list) and isinstance(value, list) \
                and all(isinstance(item, dict) for item in into[key] + value):
            into[key] = into[key] + value
        elif key in into:
            first = next(whence[at[:n]] for n in range(len(at), 0, -1) if at[:n] in whence)
            die('%s states %s, and %s states it too' % (name, '.'.join(at), first))
        else:
            into[key], whence[at] = value, name
    return into


def config(root, every=False):
    whole, whence = {}, {}
    own = root / '.toolchain' / 'project.toml'
    files = [('toolchain.toml', HERE / '.toolchain' / 'toolchain.toml')] + ([('project.toml', own)] if own.is_file() else [])
    where = root / '.toolchain' / 'config'
    files += [('config/' + path.relative_to(where).as_posix(), path) for path in sorted(where.rglob('*.toml'))]
    for name, path in files:
        merged(whole, tomllib.loads(path.read_text(encoding='utf-8')), name, whence)
    if own.is_file() and not every:
        whole['targets'] = picked(whole)
    return whole


def picked(whole):
    names = whole.get('project', {}).get('targets')
    if names is None:
        die('.toolchain/project.toml: write the targets of the project in [project] targets')
    for name in names:
        if name not in whole['targets']:
            die('[project] targets: %s is not a target of the toolchain' % name)
        if names.count(name) > 1:
            die('[project] targets: %s comes 2 times' % name)
    return {name: row for name, row in whole['targets'].items() if name in names}


def held(path, folder):
    return path == folder or path.startswith(folder + '/')


def part_of(config, path):
    found = [(len(table['folder']), part) for part, table in config.get('parts', {}).items()
             if held(path, table['folder'])]
    return max(found)[1] if found else None


def tables(config, values, name):
    own = config.get(name, {})
    found = [own]
    for one in own.get('with', []):
        path = located(one, values)
        if not path.is_file():
            die('[%s] with: %s is not there' % (name, path.as_posix()))
        found.append(tomllib.loads(path.read_text(encoding='utf-8')).get(name, {}))
    return found


def located(text, values):
    return Path(values['root']) / expand(text, values)


def expand(text, values):
    def one(m):
        if m.group(1) not in values:
            die('no variable {%s} for %s' % (m.group(1), text))
        value = values[m.group(1)]
        if value is None:
            die(UNSET[m.group(1)])
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


def run(argv, root, env=None):
    done = subprocess.run(argv, cwd=root, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, env=env)
    said = done.stdout.decode('utf-8', 'replace').replace('\r\n', '\n')
    if done.returncode:
        die('%s\n  ended with %s\n%s' % (' '.join(argv), status(done.returncode), said))
    sys.stdout.write(said)


def this_target(targets):
    machine = [platform.system(), platform.machine()]
    return next((name for name, row in targets.items() if row['runs_on'] == machine), None)


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def macos_sdk():
    if platform.system() != 'Darwin':
        return None, 'this host is not a Mac'
    try:
        done = subprocess.run(['xcrun', '--show-sdk-path'], capture_output=True, text=True)
    except OSError:
        return None, 'xcrun did not start'
    path = done.stdout.strip()
    return (path, '') if done.returncode == 0 and path else (None, 'xcrun gave no path')


def variables(config, args, target, zig):
    build = (args.out or args.root / config['build']['folder']).resolve()
    sub = target + ('-debug' if args.debug else '')
    values = {'root': args.root.as_posix(), 'build': build.as_posix(), 'home': this_target(config['targets']) or '',
              'dist': (build / sub).as_posix(), 'out': (build / sub / 'scratch').as_posix(), 'target': target,
              'python': Path(sys.executable).as_posix(), 'zig': zig, 'toolchain': (HERE / '.toolchain').as_posix()}
    sdk, why = macos_sdk()
    values['macos_sdk'] = sdk
    UNSET['macos_sdk'] = '{macos_sdk} needs a macOS host with xcrun: ' + why
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
    jobs = str(values['jobs'])
    if not jobs.isdigit() or int(jobs) < 1:
        die('[variables] jobs is %s, and it must be a whole number above 0' % jobs)
    values['jobs'] = jobs_of(config, int(jobs))
    return values


def jobs_of(config, most):
    each = config.get('build', {}).get('job_memory', 3 << 29)
    if not (isinstance(each, int) and not isinstance(each, bool) and each > 0):
        die('[build] job_memory is a count of bytes above 0')
    free = free_memory()
    return max(1, min(most, os.cpu_count() or 1, most if free is None else free // each))


def free_memory():
    said = os.environ.get('C23_FREE_MEMORY')
    if said is not None:
        return int(said) if said.isdigit() else die('C23_FREE_MEMORY is %s, and it must be a count of bytes' % said)
    if os.name == 'nt':
        import ctypes

        class Status(ctypes.Structure):
            _fields_ = [('length', ctypes.c_ulong), ('load', ctypes.c_ulong)] + [
                (name, ctypes.c_ulonglong) for name in ('phys', 'free_phys', 'commit', 'free_commit', 'virtual',
                                                        'free_virtual', 'free_extended')]
        status = Status(length=ctypes.sizeof(Status))
        if not ctypes.windll.kernel32.GlobalMemoryStatusEx(ctypes.byref(status)):
            return None
        return min(status.free_phys, status.free_commit)
    if platform.system() == 'Darwin':
        try:
            said = subprocess.run(['vm_stat'], capture_output=True, text=True, check=True).stdout
            size = int(re.search(r'page size of (\d+)', said).group(1))
        except (OSError, subprocess.CalledProcessError, AttributeError):
            return None
        return size * sum(int(n) for n in re.findall(r'Pages (?:free|inactive|speculative|purgeable):\s+(\d+)', said))
    try:
        with open('/proc/meminfo', encoding='ascii') as info:
            return next(int(line.split()[1]) * 1024 for line in info if line.startswith('MemAvailable:'))
    except (OSError, StopIteration, ValueError):
        return None


def floor_of(config):
    floor = config['test'].get('memory_floor', 2 << 30)
    if not (isinstance(floor, int) and not isinstance(floor, bool) and floor >= 0):
        die('[test] memory_floor is a count of bytes, 0 or more')
    return floor


class Room:
    def __init__(self, floor):
        self.floor, self.busy, self.gate = floor, 0, threading.Condition()

    def __enter__(self):
        with self.gate:
            while self.floor and self.busy and (free_memory() or self.floor) < self.floor:
                self.gate.wait(WAIT)
            self.busy += 1

    def __exit__(self, *_):
        with self.gate:
            self.busy -= 1
            self.gate.notify_all()


def arguments(*valued, **flags):
    ap = argparse.ArgumentParser(prog=Path(sys.argv[0]).name)
    ap.add_argument('--root', type=Path, default=HERE)
    ap.add_argument('--target')
    ap.add_argument('--part', action='append')
    ap.add_argument('--group', action='append')
    ap.add_argument('--tag')
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
    tag = args.tag and (TAG.fullmatch(args.tag) or die('the tag %s is not v<major>.<minor>.<patch> or '
                                                       '<part>-v<major>.<minor>.<patch>' % args.tag))
    if tag and tag.group(1):
        args.part = (args.part or []) + [tag.group(1)]
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


def base(root, branch):
    for name in ('origin/' + branch, branch):
        done = subprocess.run(['git', '-C', str(root), 'merge-base', 'HEAD', name], capture_output=True, text=True)
        if not done.returncode:
            return done.stdout.strip()
    return None


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
