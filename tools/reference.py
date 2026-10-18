# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import argparse
import subprocess
import sys
import urllib.request
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
    source = ROOT / 'tests' / 'reference'
    if not (out / 'flecs.o').exists():
        run([zig, 'cc', '-std=c23', *flags, '-c', out / 'flecs' / 'flecs.c', '-o', out / 'flecs.o'])
    run([zig, 'cc', '-std=c23', *flags, '-I', out / 'flecs', *sorted(source.glob('flecs_ref*.c')),
         out / 'flecs.o', '-o', out / ('flecs_ref' + exe), *windows])
    run([zig, 'c++', '-std=c++23', *flags, '-I', out, source / 'entt_ref.cpp', '-o', out / ('entt_ref' + exe)])
    run([zig, 'cc', '-std=c23', *flags, source / 'floor.c', '-o', out / ('floor' + exe)])


def main():
    parser = argparse.ArgumentParser(description='Measure each storage layout beside flecs and EnTT.')
    parser.add_argument('--root', type=Path, default=ROOT)
    parser.add_argument('--target')
    parser.add_argument('--times', type=int, default=2)
    args = parser.parse_args()
    tree = argparse.Namespace(root=args.root.resolve(), target=args.target, debug=False, out=None, set=[])
    _, values = build.prepared(tree, here=True)
    ours = Path(values['out']) / ('storage' + values['exe'])
    if not ours.exists():
        sys.exit('reference.py: %s is not built: run python project.py build' % ours.as_posix())
    out = ROOT / 'build' / 'reference'
    fetched(out)
    built(values, out)
    for time in range(args.times):
        print('==== run %d' % (time + 1), flush=True)
        for what in ('measure', 'weigh', 'baseline'):
            print(run([ours, what], cwd=ours.parent), end='', flush=True)
        for name in ('flecs_ref', 'entt_ref', 'floor'):
            print(run([out / (name + values['exe'])]), end='', flush=True)


if __name__ == '__main__':
    main()
