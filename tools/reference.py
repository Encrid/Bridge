# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import argparse
import re
import subprocess
import sys
import urllib.request
from decimal import Decimal
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / '.toolchain' / 'tools'))

import build
import shared

PINNED = {
    'flecs': ('https://raw.githubusercontent.com/SanderMertens/flecs/v4.1.6/distr/', {
        'flecs.c': '6005392eb13c0f3c7abdecb2f85271e50934f63919afebf0bbe85f6dfc7320d6',
        'flecs.h': '526036a5a41678e2a43a3cb835e9eaa70fd1993868b1978950c0d275752f69b1',
    }),
    'entt': ('https://raw.githubusercontent.com/skypjack/entt/v4.0.0/single_include/entt/', {
        'entt.hpp': 'eedcb83fedffe640334ef69a0647625a186cf1daeb333b21b0a1445f01e89b6c',
    }),
}
STORAGE = ('measure', 'weigh', 'baseline')
CONTROLS = ('flecs_ref', 'entt_ref', 'floor')
UNITS = ('ns', 'us', 'ms')
NUMBER = re.compile(r'\d+(?:\.\d+)?')
ORDER = {}


def run(command, **more):
    done = subprocess.run([str(c) for c in command], capture_output=True, text=True, **more)
    if done.returncode:
        sys.exit('reference.py: failed: %s\n%s%s' % (' '.join(str(c) for c in command), done.stdout, done.stderr))
    return done.stdout


def downloaded(url, path, digest):
    part = path.with_name(path.name + '.part')
    path.parent.mkdir(parents=True, exist_ok=True)
    print('fetching', url, flush=True)
    try:
        with urllib.request.urlopen(url) as got:
            part.write_bytes(got.read())
    except OSError as failure:
        part.unlink(missing_ok=True)
        sys.exit('reference.py: %s did not download: %s' % (url, failure))
    found = shared.sha256(part.read_bytes())
    if found != digest:
        part.unlink()
        sys.exit('reference.py: %s is not the pinned file: its sha256 is %s' % (path.name, found))
    part.replace(path)


def fetched(out):
    for name, (url, files) in PINNED.items():
        for file, digest in files.items():
            path = out / name / file
            if path.is_file() and shared.sha256(path.read_bytes()) == digest:
                continue
            path.unlink(missing_ok=True)
            downloaded(url + file, path, digest)


def built(values, out):
    zig, exe = values['zig'], values['exe']
    flags = ['-target', values['target'], '-O2', '-DNDEBUG']
    windows = ['-lws2_32', '-ldbghelp'] if values['system'] == 'windows' else []
    source = ROOT / 'tests' / 'shared' / 'reference'
    if not (out / 'flecs.o').exists():
        run([zig, 'cc', '-std=c23', *flags, '-c', out / 'flecs' / 'flecs.c', '-o', out / 'flecs.o'])
    run([zig, 'cc', '-std=c23', *flags, '-I', out / 'flecs', *sorted(source.glob('flecs_ref*.c')),
         out / 'flecs.o', '-o', out / ('flecs_ref' + exe), *windows])
    run([zig, 'c++', '-std=c++23', *flags, '-I', out, source / 'entt_ref.cpp', '-o', out / ('entt_ref' + exe)])
    run([zig, 'cc', '-std=c23', *flags, source / 'floor.c', '-o', out / ('floor' + exe)])


def tree_of(root, target):
    return argparse.Namespace(root=Path(root).resolve(), target=target, debug=False, out=None, set=[], part=None)


def prepared(root, target):
    _, values = build.prepared(tree_of(root, target), here=True)
    return values, Path(values['out']) / ('storage' + values['exe'])


def measured(number, ours, out, exe):
    parts = ['==== run %d\n' % number]
    for what in STORAGE:
        parts += ['==== storage %s\n' % what, run([ours, what], cwd=ours.parent)]
    for name in CONTROLS:
        parts += ['==== %s\n' % name, run([out / (name + exe)])]
    return ''.join(parts)


def timed(row):
    words = row.split()
    text, times, unit, place, at = [], [], None, 0, 0
    for i, word in enumerate(words):
        start = row.index(word, at)
        at = start + len(word)
        core = word.lstrip('(').rstrip('),;:')
        if NUMBER.fullmatch(core):
            place += 1
            text.append(word.replace(core, '#', 1))
            begin = start + len(word) - len(word.lstrip('('))
            inside = row.rfind('(', 0, begin) > row.rfind(')', 0, begin)
            following = words[i + 1].rstrip(',;') if i + 1 < len(words) else ''
            if (following in UNITS or unit) and not inside:
                times.append((place, Decimal(core)))
        else:
            text.append(word)
        if word.rstrip(',;') in UNITS:
            unit = word.rstrip(',;')
    return ' '.join(text), times


def runs(path):
    if not path.is_file():
        sys.exit('reference.py: %s is not there' % path.as_posix())
    found, command, counts = [], None, {}
    for row in path.read_text(encoding='utf-8').splitlines():
        if row.startswith('==== run '):
            found.append({})
            command = None
        elif row.startswith('===='):
            command, counts = row[4:].strip(), {}
        elif found and command and row.strip():
            text, times = timed(row)
            count = counts.get(text, 0)
            counts[text] = count + 1
            for place, value in times:
                found[-1][(command, text, place, count)] = value
                ORDER.setdefault((command, text, place, count), len(ORDER))
    if not found:
        sys.exit('reference.py: %s holds no run' % path.as_posix())
    return found


def compared(old, new):
    every = [set(found) for found in old + new]
    both = set.intersection(*every)
    slower = {}
    for key in both:
        before = [found[key] for found in old]
        after = [found[key] for found in new]
        spread = max(max(before) - min(before), max(after) - min(after))
        if min(after) - min(before) > spread:
            slower[key] = (min(before), max(before), min(after), max(after))
    return both, set.union(*every) - both, slower


def reported(pairs):
    results = [compared(runs(Path(old)), runs(Path(new))) for old, new in pairs]
    both = set.intersection(*(result[0] for result in results))
    lone = set.union(*(result[1] for result in results))
    slower = set.intersection(*(set(result[2]) for result in results))
    lines = []
    for key in sorted(slower, key=ORDER.get):
        command, text, place, count = key
        kind = 'control' if command in CONTROLS else 'slower'
        for _, _, ranges in results:
            lines.append('%s: %s: %s, row %d, number %d: %s to %s, then %s to %s'
                         % ((kind, command, text, count + 1, place) + ranges[key]))
    rows = {}
    for command, text, place, count in lone:
        rows.setdefault((command, text, count), ORDER[(command, text, place, count)])
    for command, text, count in sorted(rows, key=rows.get):
        lines.append('only in one tree: %s: %s, row %d' % (command, text, count + 1))
    for name, controls in (('Bridge', False), ('the control rows', True)):
        keys = [key for key in both if (key[0] in CONTROLS) == controls]
        lines.append('%d of %d times of %s are slower' % (len([key for key in keys if key in slower]), len(keys), name))
    return lines


def paired(old, new, target):
    timing = ROOT / 'build' / 'timing'
    timing.mkdir(parents=True, exist_ok=True)
    run(['git', '-C', ROOT, 'worktree', 'prune'])
    ours = {}
    for side, commit in (('old', old), ('new', new)):
        tree = timing / side
        if (tree / '.git').is_file():
            run(['git', '-C', tree, 'checkout', '--quiet', '--detach', commit])
        else:
            run(['git', '-C', ROOT, 'worktree', 'add', '--quiet', '--detach', tree, commit])
        run([sys.executable, ROOT / '.toolchain' / 'tools' / 'fetch.py', '--root', tree])
        build.build(tree_of(tree, target), here=True)
        values, ours[side] = prepared(tree, target)
    out = ROOT / 'build' / 'reference'
    fetched(out)
    built(values, out)
    for side in ('old', 'new'):
        measured(0, ours[side], out, values['exe'])
    texts = {'old1': [], 'new1': [], 'old2': [], 'new2': []}
    for number in (1, 2, 3):
        texts['old1'].append(measured(number, ours['old'], out, values['exe']))
        texts['new1'].append(measured(number, ours['new'], out, values['exe']))
    for number in (1, 2, 3):
        texts['new2'].append(measured(number, ours['new'], out, values['exe']))
        texts['old2'].append(measured(number, ours['old'], out, values['exe']))
    folder = timing / 'runs' / ('%s-%s' % (old, new))
    folder.mkdir(parents=True, exist_ok=True)
    for name, parts in texts.items():
        (folder / (name + '.txt')).write_text(''.join(parts), encoding='utf-8')
    lines = reported([(folder / 'old1.txt', folder / 'new1.txt'), (folder / 'old2.txt', folder / 'new2.txt')])
    (folder / 'report.txt').write_text('\n'.join(lines) + '\n', encoding='utf-8')
    print('\n'.join(lines))


def main():
    parser = argparse.ArgumentParser(description='Measure the storage of Bridge beside flecs and EnTT.')
    parser.add_argument('--root', type=Path, default=ROOT)
    parser.add_argument('--target')
    parser.add_argument('--times', type=int, default=2)
    parser.add_argument('--compare', nargs=2, action='append', metavar=('OLD', 'NEW'))
    parser.add_argument('--pair', nargs=2, metavar=('OLD', 'NEW'))
    args = parser.parse_args()
    if args.compare:
        print('\n'.join(reported(args.compare)))
        return
    if args.pair:
        paired(args.pair[0], args.pair[1], args.target)
        return
    values, ours = prepared(args.root, args.target)
    if not ours.exists():
        sys.exit('reference.py: %s is not built: run python project.py test' % ours.as_posix())
    out = ROOT / 'build' / 'reference'
    fetched(out)
    built(values, out)
    for time in range(args.times):
        print(measured(time + 1, ours, out, values['exe']), end='', flush=True)


if __name__ == '__main__':
    main()
