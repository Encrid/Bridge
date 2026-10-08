#!/usr/bin/env python3
"""The packages of a target are what the document says a host is handed. The case package.

    python tools/package.py <the folder a target was built into>

host_api.md, *What a host is handed*, names five packages and what each holds. This reads
packages/ under the folder and fails on a package that is not there or that is not named
there; on a file a package should hold and does not, and one it holds and should not; on a
statement, package.txt, that does not give the package's name, a commit, the target and
whether its cases were run; on a hash in it that is not the file's; and on a header or a
licence in a package that is not, byte for byte, the one in the tree.

What each package holds is built.py's to say, in `packages`, and is read from there: so what
is checked here is that what was written is that, that the document names it, and that nothing
was changed after.
"""
import hashlib
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))
import build  # noqa: E402

if len(sys.argv) != 2:
    sys.exit('usage: package.py <the folder a target was built into>')
out = pathlib.Path(sys.argv[1])
target = out.name.removesuffix('-debug')
row = build.TARGETS[target]
runtime = ROOT / 'backends/wasm/wasmtime' / row['wasmtime'][1]
expected = build.packages(out, row['library'], runtime)
document = (ROOT / 'docs/seam/host_api/host_api.md').read_text(encoding='utf-8')
bad = 0


def fault(said):
    global bad
    print(said)
    bad += 1


there = {p.name for p in (out / 'packages').iterdir() if p.is_dir()} if (out / 'packages').is_dir() else set()
for name in sorted(there - set(expected)):
    fault('packages/%s is a package the build does not make' % name)
for name, files in sorted(expected.items()):
    folder = out / 'packages' / name
    if '`%s`' % name not in document:
        fault('host_api.md does not name the package %s' % name)
    if name not in there:
        fault('packages/%s is not there' % name)
        continue
    wanted = {called for _, called in files} | {'package.txt'}
    held = {p.name for p in folder.iterdir()}
    for called in sorted(wanted - held):
        fault('packages/%s does not hold %s' % (name, called))
    for called in sorted(held - wanted):
        fault('packages/%s holds %s, which is not of it' % (name, called))
    if 'package.txt' not in held:
        continue
    lines = (folder / 'package.txt').read_text(encoding='utf-8').split('\n')
    head = '\n'.join(lines[:4])
    if not re.fullmatch(r'package %s\ncommit [0-9a-f]{40}( with changes that are not committed)?\n'
                        r'target %s\nrun (yes|no)' % (re.escape(name), re.escape(target)), head):
        fault('packages/%s/package.txt does not state its name, a commit, the target and whether it ran' % name)
    stated = dict(reversed(line.split('  ', 1)) for line in lines[4:] if line)
    for source, called in files:
        if called not in held:
            continue
        now = hashlib.sha256((folder / called).read_bytes()).hexdigest()
        if stated.get(called) != now:
            fault('packages/%s/%s is not the file its statement gives the hash of' % (name, called))
        if ROOT in source.parents and (folder / called).read_bytes() != source.read_bytes():
            fault('packages/%s/%s is not the %s of the tree' % (name, called, source.relative_to(ROOT).as_posix()))
    for called in sorted(set(stated) - {c for _, c in files}):
        fault('packages/%s/package.txt gives a hash for %s, which is not of it' % (name, called))

print('%d packages: %d faults' % (len(expected), bad))
sys.exit(1 if bad else 0)
