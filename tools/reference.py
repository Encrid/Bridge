# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import argparse
import shutil
import subprocess
import sys
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PINNED = {
    'flecs': ('https://raw.githubusercontent.com/SanderMertens/flecs/v4.1.6/distr/', ['flecs.c', 'flecs.h']),
    'entt': ('https://raw.githubusercontent.com/skypjack/entt/v4.0.0/single_include/entt/', ['entt.hpp']),
}


def run(command, **more):
    done = subprocess.run([str(c) for c in command], capture_output=True, text=True, **more)
    if done.returncode:
        sys.exit('reference.py: failed: %s\n%s%s' % (' '.join(str(c) for c in command), done.stdout, done.stderr))
    return done.stdout


def zig_path():
    fetched = sorted(ROOT.glob('build/zig/*/zig.exe')) + sorted(ROOT.glob('build/zig/*/zig'))
    return str(fetched[0]) if fetched else shutil.which('zig') or sys.exit('reference.py: zig is not there: run fetch')


def fetched(out):
    for name, (url, files) in PINNED.items():
        where = out / name
        where.mkdir(parents=True, exist_ok=True)
        for file in files:
            if not (where / file).exists():
                print('fetching', url + file, flush=True)
                urllib.request.urlretrieve(url + file, where / file)


def built(zig, target, out, exe):
    flags = ['-target', target, '-O2', '-DNDEBUG']
    windows = ['-lws2_32', '-ldbghelp'] if 'windows' in target else []
    source = ROOT / 'tests' / 'reference'
    if not (out / 'flecs.o').exists():
        run([zig, 'cc', '-std=c23', *flags, '-c', out / 'flecs' / 'flecs.c', '-o', out / 'flecs.o'])
    run([zig, 'cc', '-std=c23', *flags, '-I', out / 'flecs', *sorted(source.glob('flecs_ref*.c')),
         out / 'flecs.o', '-o', out / ('flecs_ref' + exe), *windows])
    run([zig, 'c++', '-std=c++23', *flags, '-I', out, source / 'entt_ref.cpp', '-o', out / ('entt_ref' + exe)])
    run([zig, 'cc', '-std=c23', *flags, source / 'floor.c', '-o', out / ('floor' + exe)])


def main():
    parser = argparse.ArgumentParser(description='Measure each storage layout beside flecs and EnTT.')
    parser.add_argument('--target', default='x86_64-windows-gnu')
    parser.add_argument('--times', type=int, default=2)
    args = parser.parse_args()
    out = ROOT / 'build' / 'reference'
    exe = '.exe' if 'windows' in args.target else ''
    ours = ROOT / 'build' / args.target / ('storage' + exe)
    if not ours.exists():
        sys.exit('reference.py: %s is not built: run python project.py build' % ours.as_posix())
    fetched(out)
    built(zig_path(), args.target, out, exe)
    for time in range(args.times):
        print('==== run %d' % (time + 1), flush=True)
        for what in ('measure', 'weigh', 'baseline'):
            print(run([ours, what], cwd=ours.parent), end='', flush=True)
        for name in ('flecs_ref', 'entt_ref', 'floor'):
            print(run([out / (name + exe)]), end='', flush=True)


if __name__ == '__main__':
    main()
