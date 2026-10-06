"""Measure each storage layout beside the best implementation of it there is.

    python tools/reference.py [--target x86_64-windows-gnu] [--times 2]

flecs and EnTT are fetched at the tags pinned below into build/reference/, built with zig, and
run in turn with the library's own `storage measure`, `storage weigh` and `storage baseline`,
which `python build.py` must have built. Nothing of either is kept in the tree: the library is
self contained, and these are what it is measured beside - docs/plan/STAGE11.md, *Beside the
best there is*.
"""
import argparse
import pathlib
import shutil
import subprocess
import sys
import urllib.request

ROOT = pathlib.Path(__file__).resolve().parent.parent
# what is measured beside: a project, its tag, and the files of it that are needed
PINNED = {
    'flecs': ('https://raw.githubusercontent.com/SanderMertens/flecs/v4.1.6/distr/', ['flecs.c', 'flecs.h']),
    'entt': ('https://raw.githubusercontent.com/skypjack/entt/v4.0.0/single_include/entt/', ['entt.hpp']),
}


def run(command, **more):
    done = subprocess.run([str(c) for c in command], capture_output=True, text=True, **more)
    if done.returncode:
        sys.exit('reference: failed: %s\n%s%s' % (' '.join(str(c) for c in command), done.stdout, done.stderr))
    return done.stdout


def main():
    parser = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    parser.add_argument('--target', default='x86_64-windows-gnu')
    parser.add_argument('--times', type=int, default=2)
    args = parser.parse_args()
    zig = shutil.which('zig') or sys.exit('reference: zig is not on the path')
    out = ROOT / 'build' / 'reference'
    exe = '.exe' if 'windows' in args.target else ''
    ours = ROOT / 'build' / args.target / ('storage' + exe)
    if not ours.exists():
        sys.exit('reference: %s is not built: python build.py' % ours)

    for name, (url, files) in PINNED.items():
        where = out / name
        where.mkdir(parents=True, exist_ok=True)
        for file in files:
            if not (where / file).exists():
                print('fetching', url + file, flush=True)
                urllib.request.urlretrieve(url + file, where / file)

    flags = ['-target', args.target, '-O2', '-DNDEBUG']
    windows = ['-lws2_32', '-ldbghelp'] if 'windows' in args.target else []
    source = ROOT / 'tests' / 'reference'
    if not (out / 'flecs.o').exists():
        run([zig, 'cc', '-std=c23', *flags, '-c', out / 'flecs' / 'flecs.c', '-o', out / 'flecs.o'])
    run([zig, 'cc', '-std=c23', *flags, '-I', out / 'flecs', source / 'flecs_ref.c', out / 'flecs.o',
         '-o', out / ('flecs_ref' + exe), *windows])
    run([zig, 'c++', '-std=c++23', *flags, '-I', out, source / 'entt_ref.cpp', '-o', out / ('entt_ref' + exe)])
    run([zig, 'cc', '-std=c23', *flags, source / 'floor.c', '-o', out / ('floor' + exe)])

    # in turn, and more than once: a machine is not as quick from one minute to the next
    for time in range(args.times):
        print('==== run %d' % (time + 1), flush=True)
        for what in ('measure', 'weigh', 'baseline'):
            print(run([ours, what], cwd=ours.parent), end='', flush=True)
        for name in ('flecs_ref', 'entt_ref', 'floor'):
            print(run([out / (name + exe)]), end='', flush=True)


if __name__ == '__main__':
    main()
