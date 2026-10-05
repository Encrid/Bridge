"""Bridge-ASM700's build: one compiler, one script. docs/design/build.md.

    python build.py                      build, for this machine
    python build.py test [-k NAME]       build, then run the tests (all, or those matching)
    python build.py --target T ...       x86_64-windows-gnu | x86_64-linux-gnu | aarch64-macos
    python build.py --debug ...          unoptimised, with debug information

Needs zig and wasm-tools on the path. Binaryen is taken from BINARYEN_ROOT or the path, and on
Windows fetched at its pin when it is in neither. Every path is taken from where this file is
and from --out, so a repository that has this one as a submodule builds it by calling this.
"""
import argparse
import concurrent.futures
import hashlib
import os
import platform
import re
import shutil
import subprocess
import sys
import tarfile
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent
# What differs by target, and nothing else does: where it runs, how a program and a library are
# named, which of the system's declarations are asked for by name (the language is ISO C, and a
# compiler's gnu mode would have shown them unasked), what a link names, how a library finds what
# sits beside it, and wasmtime's files under third_party/wasmtime: the one a backend is linked
# against, and the one it loads.
TARGETS = {
    'x86_64-windows-gnu': {
        'runs on': ('Windows', 'AMD64'), 'program': '.exe', 'library': '.dll',
        'compile': [], 'link': ['-lws2_32', '-ldbghelp'], 'beside': [],
        'wasmtime': ('windows/libwasmtime.dll.a', 'windows/wasmtime.dll')},
    'x86_64-linux-gnu': {
        'runs on': ('Linux', 'x86_64'), 'program': '', 'library': '.so',
        'compile': ['-fPIC', '-D_DEFAULT_SOURCE'], 'link': ['-lm', '-lpthread', '-ldl'],
        'beside': ['-Wl,-rpath,$ORIGIN'],
        'wasmtime': ('linux/libwasmtime.so', 'linux/libwasmtime.so')},
    'aarch64-macos': {
        'runs on': ('Darwin', 'arm64'), 'program': '', 'library': '.dylib',
        'compile': ['-fPIC', '-D_DARWIN_C_SOURCE'], 'link': [],
        'beside': ['-Wl,-rpath,@loader_path'],
        'wasmtime': ('macos/libwasmtime.dylib', 'macos/libwasmtime.dylib')},
}
BINARYEN = 'binaryen-version_133'
BINARYEN_URL = ('https://github.com/WebAssembly/binaryen/releases/download/version_133/'
                'binaryen-version_133-x86_64-windows.tar.gz')
BINARYEN_SHA256 = '17a2cbeac6b5693c5fbafab3838d3c65fd9c1eb38b05f5baec6c657e8c84995b'
FEATURES = ['--enable-multimemory', '--enable-bulk-memory', '--enable-sign-ext',
            '--enable-nontrapping-float-to-int', '--enable-mutable-globals']
ROW_TEST = '(if (i32.ge_u (local.get $row) (local.get $count)) (then (unreachable)))'
TIMEOUT = 120

# flecs, patched: the name, and the groups of the patch left out. Only the tests that must fail
# use the last two.
FLECS = [('flecs', []), ('flecs-unconfined', ['allocator']), ('flecs-nohook', ['hook'])]

BRIDGE = ['bridge', 'lifecycle', 'config', 'store', 'abi', 'schedule', 'pool', 'backend']
# who keeps the rows, behind src/rows.h. The one file of the library that sees flecs
ROWS = 'rows-flecs'

# a native module is a shared library: the name, its source, what it is compiled with. The same
# library twice is one module to the loader, so one source is built under several names.
NATIVE = [
    ('nmover', 'nmover', []), ('nflood', 'nflood', []), ('nsetter', 'nsetter', []),
    ('nsleep1', 'nsleep', []), ('nsleep2', 'nsleep', []), ('npinned', 'npinned', []),
    ('nticker1', 'nticker', ['STEP=1']), ('nticker2', 'nticker', ['STEP=10']),
    ('nticker3', 'nticker', ['STEP=10', 'FAIL_RELOAD']),
    ('nstore', 'nstore', []), ('nstorebig', 'nstore', ['BIG']), ('nstoretab', 'nstore', ['TABLES']),
    ('nstress1', 'nstress', ['SEED=1u', 'SPACE="s1"']), ('nstress2', 'nstress', ['SEED=2u', 'SPACE="s2"']),
    ('nstress3', 'nstress', ['SEED=3u', 'SPACE="s3"']), ('nstress4', 'nstress', ['SEED=4u', 'SPACE="s4"']),
]

# a module in C: the name, its source, which accessors it is merged with, what it is compiled
# with. `unchecked` is the accessors without the row < count test: only the measurement uses it.
PLAIN = ('mover reader refs fp wildcard spin3 flood chatty rows halver twostep watcher chain '
         'badobs events fixer nested nested2 namer spawner counter typer user settings hang')
WASM = [(name, name, 'checked', []) for name in PLAIN.split()] + [
    ('rows-unchecked', 'rows', 'unchecked', []),
    ('flaky-always', 'flaky', 'checked', ['OF=1', 'FAILS=1']),
    ('flaky-third', 'flaky', 'checked', ['OF=3', 'FAILS=1']),
    ('flaky-twothirds', 'flaky', 'checked', ['OF=3', 'FAILS=2']),
    ('keeper-strip', 'keeper', 'checked', ['PRIVATE_POLICY=BRIDGE_STRIP']),
    ('keeper-sweep', 'keeper', 'checked', ['PRIVATE_POLICY=BRIDGE_SWEEP']),
    ('keeper-delete', 'keeper', 'checked', ['PRIVATE_POLICY=BRIDGE_DELETE']),
    ('retype-private', 'retype', 'checked', ['WHICH=1']),
    ('retype-vec2', 'retype', 'checked', ['WHICH=2']),
    ('retype-extra', 'retype', 'checked', ['WHICH=3']),
    ('retype-settings', 'retype', 'checked', ['WHICH=4']),
    ('settings-drop', 'settings', 'checked', ['DROP']),
    ('ticker', 'ticker', 'checked', ['STEP=1']),
    ('ticker-ten', 'ticker', 'checked', ['STEP=10']),
    ('ticker-wide', 'ticker', 'checked', ['STEP=10', 'WIDE']),
    ('ticker-fail', 'ticker', 'checked', ['STEP=10', 'FAIL_RELOAD']),
    ('ticker-noextra', 'ticker', 'checked', ['STEP=10', 'NO_EXTRA']),
    ('ticker-trip', 'ticker', 'checked', ['STEP=1', 'TRIP']),
    ('ticker-alloc', 'ticker', 'checked', ['STEP=10', 'ALLOC']),
]


def die(msg):
    sys.stderr.write('build: %s\n' % msg)
    sys.exit(1)


def stale(outs, deps):
    if not all(o.exists() for o in outs):
        return True
    oldest = min(o.stat().st_mtime for o in outs)
    return any(d.stat().st_mtime > oldest for d in deps)


def step(what, outs, deps, commands):
    """Run `commands` if an output is missing or older than a dependency."""
    if not stale(outs, deps):
        return
    for o in outs:
        o.parent.mkdir(parents=True, exist_ok=True)
    for command in commands:
        if callable(command):
            command()
            continue
        done = subprocess.run([str(c) for c in command], stdout=subprocess.PIPE,
                              stderr=subprocess.STDOUT)
        if done.returncode:
            for o in outs:
                if o.exists():
                    o.unlink()
            raise RuntimeError('%s\n  %s\n%s' % (what, ' '.join(str(c) for c in command),
                                                done.stdout.decode('utf-8', 'replace')))
        if done.stdout.strip():
            sys.stdout.write(done.stdout.decode('utf-8', 'replace'))


def together(steps):
    """Steps that do not depend on each other, at once."""
    with concurrent.futures.ThreadPoolExecutor(os.cpu_count()) as pool:
        failed = [f.exception() for f in [pool.submit(step, *s) for s in steps] if f.exception()]
    if failed:
        die('\n'.join(str(f) for f in failed))


def tool(name, hints=()):
    exe = name + ('.exe' if os.name == 'nt' else '')
    for hint in hints:
        if (Path(hint) / exe).exists():
            return Path(hint) / exe
    found = shutil.which(name)
    return Path(found) if found else None


def binaryen(build):
    """wasm-merge and wasm-opt: from BINARYEN_ROOT, the path, or fetched at the pin."""
    hints = [build / BINARYEN / 'bin']
    if os.environ.get('BINARYEN_ROOT'):
        hints.insert(0, Path(os.environ['BINARYEN_ROOT']) / 'bin')
    if not tool('wasm-merge', hints):
        if os.name != 'nt':
            die('wasm-merge is not on the path, and BINARYEN_ROOT does not name it')
        archive = build / 'binaryen.tar.gz'
        build.mkdir(parents=True, exist_ok=True)
        urllib.request.urlretrieve(BINARYEN_URL, archive)
        if hashlib.sha256(archive.read_bytes()).hexdigest() != BINARYEN_SHA256:
            die('%s is not the Binaryen this build pins: its hash differs' % archive)
        with tarfile.open(archive) as tar:
            tar.extractall(build)
    merge, opt = tool('wasm-merge', hints), tool('wasm-opt', hints)
    if not merge or not opt:
        die('Binaryen was found without wasm-merge or wasm-opt')
    return merge, opt


def build(target, debug, out):
    row = TARGETS[target]
    exe, lib = row['program'], row['library']
    zig = tool('zig') or die('zig is not on the path')
    wasm_tools = tool('wasm-tools') or die('wasm-tools is not on the path')
    merge, opt = binaryen(out.parent)
    obj, modules, backends = out / 'obj', out / 'modules', out / 'backends'
    wasmtime = ROOT / 'third_party/wasmtime'
    link_wasmtime, runtime = (wasmtime / f for f in row['wasmtime'])

    cc = [zig, 'cc', '-target', target] + (['-O0', '-g'] if debug else ['-O2', '-DNDEBUG'])
    cc += row['compile']
    # an error, since zig says a warning once and then answers from its cache without it. Members
    # an initializer leaves out are zero, which C states and flecs' descriptors are written for.
    ours = cc + ['-std=c23', '-pedantic-errors', '-Wall', '-Wextra', '-Werror',
                 '-Wno-missing-field-initializers']
    system, here = row['link'], row['beside']
    include, src = '-I%s' % (ROOT / 'include'), '-I%s' % (ROOT / 'src')
    headers = [Path(__file__)] + [h for d in ('include', 'src', 'tests', 'tests/modules')
                                  for h in (ROOT / d).glob('*.h')]

    # ---- flecs, patched at build time from the pristine copy: third_party/ is never edited ----
    patch, pristine = ROOT / 'tools/patch-flecs.py', ROOT / 'third_party/flecs'
    for name, omit in FLECS:
        step('patching flecs into %s' % name,
             [out / name / f for f in ('flecs.c', 'flecs.h', 'flecs_confine.h')],
             [patch, pristine / 'flecs.c', pristine / 'flecs.h'],
             [[sys.executable, patch, pristine, out / name] + [a for g in omit for a in ('--omit', g)]])

    # the script declines a flecs it was not written for: a copy that says it is another version
    other = out / 'other-version'

    def another_version():
        header = (pristine / 'flecs.h').read_text(encoding='utf-8')
        changed = header.replace('#define FLECS_VERSION_PATCH 6', '#define FLECS_VERSION_PATCH 7')
        if changed == header:
            die('flecs.h: its patch version is not where the build expects it')
        (other / 'flecs.h').write_text(changed, encoding='utf-8')
        shutil.copyfile(pristine / 'flecs.c', other / 'flecs.c')
    step('another flecs version', [other / 'flecs.h', other / 'flecs.c'],
         [pristine / 'flecs.h', pristine / 'flecs.c', Path(__file__)], [another_version])

    # ---- objects ----
    flecs = '-I%s' % (out / 'flecs')
    defines = ['-D%s="%s"' % (key, path.as_posix()) for key, path in
               [('BACKEND_PATH', backends / ('bridge-wasm' + lib)),
                ('NATIVE_PATH', backends / ('bridge-native' + lib)), ('MODULE_DIR', modules)] +
               [('%s_PATH' % name.upper(), out / (name + lib)) for name, _, _ in NATIVE]]
    compiles = []

    def compile_(name, source, flags, deps=()):
        compiles.append(('compiling %s' % name, [obj / (name + '.o')],
                         [source] + headers + list(deps),
                         [flags + ['-c', source, '-o', obj / (name + '.o')]]))
        return obj / (name + '.o')

    flecs_o = {}
    for name, _ in FLECS:
        # built on its own terms: its own warnings, none of ours
        flecs_o[name] = compile_(name, out / name / 'flecs.c',
                                 cc + ['-std=c99', '-w', '-I%s' % (out / name)])
        for test in ('confined', 'rowmove'):
            compile_('%s-%s' % (test, name), ROOT / 'tests' / (test + '.c'),
                     ours + ['-I%s' % (out / name)], [out / name / 'flecs.h'])
    os_o = compile_('os', ROOT / 'src/os.c', ours + [src])
    bridge_o = [compile_(name, ROOT / 'src' / (name + '.c'), ours + [include, src]) for name in BRIDGE]
    bridge_o.append(compile_(ROWS, ROOT / 'src' / (ROWS + '.c'), ours + [include, src, flecs],
                             [out / 'flecs/flecs.h']))
    wasm_o = compile_('wasm', ROOT / 'backends/wasm/wasm.c',
                      ours + [include, src, '-I%s' % (wasmtime / 'include')])
    native_o = compile_('native', ROOT / 'backends/native/native.c', ours + [include, src])
    adopt_o = compile_('adopt', ROOT / 'tests/adopt.c', ours + [include, src] + defines)
    phase_o = {name: compile_(name, ROOT / 'tests' / (name + '.c'), ours + [include] + defines)
               for name in ('phase2', 'phase3', 'phase4', 'phase5', 'phase6')}
    inside_o = {'tables': compile_('tables', ROOT / 'tests/tables.c',
                                   ours + [include, src, flecs] + defines, [out / 'flecs/flecs.h']),
                'storage': compile_('storage', ROOT / 'tests/storage.c', ours + [include, src] + defines)}
    module_o = {name: compile_(name, ROOT / 'tests/modules/native' / (source + '.c'),
                               ours + [include] + ['-D' + d for d in flags])
                for name, source, flags in NATIVE}
    together(compiles)

    # ---- libraries and programs ----
    links = []

    def link(name, objects, flags=(), shared=False):
        target_file = out / (name + (lib if shared else exe))
        links.append(('linking %s' % name, [target_file], list(objects),
                      [cc + (['-shared'] if shared else []) + list(objects) + list(flags) +
                       ['-o', target_file]]))

    # the backend loads wasmtime from beside itself
    # the backend loads wasmtime from beside itself, which is not beside the programs: a host is
    # not where its backends are, and the tests would not notice if they were
    step('copying wasmtime', [backends / runtime.name], [runtime],
         [lambda: shutil.copyfile(runtime, backends / runtime.name)])
    link('backends/bridge-wasm', [wasm_o, os_o], [link_wasmtime] + system + here, shared=True)
    link('backends/bridge-native', [native_o, os_o], system, shared=True)
    for name, _, _ in NATIVE:
        link(name, [module_o[name]], shared=True)
    for test, variant, name in (('confined', 'flecs', 'confined'), ('rowmove', 'flecs', 'rowmove'),
                                ('confined', 'flecs-unconfined', 'confined-unconfined'),
                                ('rowmove', 'flecs-nohook', 'rowmove-nohook')):
        link(name, [obj / ('%s-%s.o' % (test, variant)), flecs_o[variant]], system)
    link('adopt', [adopt_o, obj / 'backend.o', os_o], system)
    whole = bridge_o + [os_o, flecs_o['flecs']]
    for name, o in phase_o.items():
        link(name, [o] + whole, system)
    for name, o in inside_o.items():
        link(name, [o] + whole, system)
    together(links)

    # ---- wasm modules: text assembled, C compiled and merged with the accessors ----
    accessors = ROOT / 'include/bridge-wasm.wat'
    unchecked = out / 'bridge-wasm-unchecked.wat'

    def without_row_test():
        text = accessors.read_text(encoding='utf-8')
        if ROW_TEST not in text:
            die('bridge-wasm.wat: the row < count test is not where the build expects it')
        unchecked.write_text(text.replace(ROW_TEST, ''), encoding='utf-8')
    step('accessors without the row test', [unchecked], [accessors, Path(__file__)],
         [without_row_test])
    assembled = [('assembling %s' % wat.name, [modules / (wat.stem + '.wasm')], [wat],
                  [[wasm_tools, 'parse', wat, '-o', modules / (wat.stem + '.wasm')]])
                 for wat in sorted((ROOT / 'tests/modules').glob('*.wat'))]
    assembled += [('assembling the %s accessors' % kind, [modules / ('acc-%s.wasm' % kind)], [wat],
                   [[wasm_tools, 'parse', wat, '-o', modules / ('acc-%s.wasm' % kind)]])
                  for kind, wat in (('checked', accessors), ('unchecked', unchecked))]
    together(assembled)
    compiled = []
    for name, source, kind, flags in WASM:
        raw, merged, final = (modules / (name + s) for s in ('.raw.wasm', '.merged.wasm', '.wasm'))
        acc = modules / ('acc-%s.wasm' % kind)
        c = ROOT / 'tests/modules' / (source + '.c')
        compiled.append(('building module %s' % name, [final], [c, acc] + headers, [
            [zig, 'cc', '-target', 'wasm32-freestanding', '-std=c23', '-pedantic-errors', '-Wall',
             '-Wextra', '-Werror', '-Wno-missing-field-initializers', '-O2', '-nostdlib'] +
            ['-D' + d for d in flags] + [include, '-Wl,--no-entry', '-o', raw, c],
            [merge, raw, 'main', acc, 'acc'] + FEATURES + ['-o', merged],
            [opt, merged] + FEATURES + ['-O3', '--remove-exports=acc_*',
                                         '--remove-unused-module-elements', '-o', final]]))
    together(compiled)
    return {'python': sys.executable, 'patch': patch, 'other': other, 'nocall': ROOT / 'tools/nocall.py',
            'wasm-tools': wasm_tools, 'modules': modules, 'exe': exe}


def test(out, built, only):
    """Each case by what it said. With a pattern, the pattern decides and the exit code does
    not: several cases are a refusal that ends the program. Without one, the exit code does."""
    cases = [c for c in CASES if not only or only in c[0]]
    failed = []
    for name, command, match, never in cases:
        argv = [str(out / (command[0] + built['exe']))] + command[1:]
        if command[0] == 'patch-version':
            argv = [str(built[k]) for k in ('python', 'patch', 'other')] + [str(built['other'] / 'out')]
        elif command[0] == 'c23':
            argv = [str(built['python']), str(ROOT / 'tools/c23.py')]
        elif command[0] == 'nocall':
            argv = [str(built[k]) for k in ('python', 'nocall', 'wasm-tools')] + \
                   [str(built['modules'] / command[1]), command[2]]
        try:
            done = subprocess.run(argv, cwd=out, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                  timeout=TIMEOUT)
            said, code = done.stdout.decode('utf-8', 'replace'), done.returncode
        except subprocess.TimeoutExpired as late:
            said, code = (late.stdout or b'').decode('utf-8', 'replace'), 'timeout'
        if code == 'timeout':
            why = 'did not end in %d seconds' % TIMEOUT
        elif never and re.search(never, said, re.S):
            why = 'said what it must not: %s' % never
        elif match and match.startswith('@') and \
                said.replace('\r', '') != (ROOT / match[1:]).read_text(encoding='utf-8'):
            why = 'did not say what %s holds, word for word' % match[1:]
        elif match and not match.startswith('@') and not re.search(match, said, re.S):
            why = 'ended with %s and did not say: %s' % (code, match)
        elif not match and code:
            why = 'ended with %s' % code
        else:
            print('ok    %s' % name)
            continue
        print('FAIL  %s: %s\n%s' % (name, why, said))
        failed.append(name)
    print('%d of %d passed' % (len(cases) - len(failed), len(cases)))
    if failed:
        die('failed: %s' % ' '.join(failed))
    if not cases:
        die('no test is named like %r' % only)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('command', nargs='?', default='build', choices=['build', 'test'])
    ap.add_argument('--target', choices=sorted(TARGETS))
    ap.add_argument('--debug', action='store_true')
    ap.add_argument('--out', type=Path, help='where to build: build/<target> beside this file')
    ap.add_argument('-k', metavar='NAME', help='only the tests whose name has this in it')
    args = ap.parse_args()
    machine = (platform.system(), platform.machine())
    mine = next((t for t, row in TARGETS.items() if row['runs on'] == machine), None)
    target = args.target or mine or die('no target runs on %s %s: name one' % machine)
    if args.command == 'test' and target != mine:
        die('%s cannot be run here: build it, and test it where it runs' % target)
    out = (args.out or ROOT / 'build' / (target + ('-debug' if args.debug else ''))).resolve()
    built = build(target, args.debug, out)
    if args.command == 'test':
        test(out, built, args.k)


# Every case: its name, what is run, what its output must match, what it must not. A phase's
# cases end in "alive": checked by what was said, and by the process still being there to say it.
# A match that begins with @ names a file the output must equal, word for word: a transcript.
CASES = [
    ('c23', ['c23'],
     None, None),
    ('storage-changes', ['storage', 'changes'],
     '@tests/storage-changes.txt', None),
    ('storage-tables', ['storage', 'tables'],
     '@tests/storage-tables.txt', None),
    ('storage-stress', ['storage', 'stress'],
     '@tests/storage-stress.txt', None),
    ('storage-measure', ['storage', 'measure'],
     r"an id handed out +[0-9.]+ ns each.*ten thousand tables made.*the stress run, on 1 thread.*alive", None),
    ('confined', ['confined'],
     None, None),
    ('rowmove', ['rowmove'],
     None, None),
    ('confined-unconfined', ['confined-unconfined'],
     r"FAIL: [0-9]+ pointers escaped the block", None),
    ('rowmove-nohook', ['rowmove-nohook'],
     r"FAIL: [0-9]+ row mismatches", None),
    ('patch-version', ['patch-version'],
     r"version 4.1.7, and this script was written for 4.1.6", None),
    ('adopt', ['adopt'],
     None, None),
    ('phase2-ok', ['phase2', 'ok'],
     r"test:ok@0.1.0: loaded.*alive", None),
    ('phase2-nolimit', ['phase2', 'nolimit'],
     r"test:ok@0.1.0: loaded.*alive", None),
    ('phase2-image', ['phase2', 'image'],
     r"test:image@0.1.0: loaded.*alive", None),
    ('phase2-oob', ['phase2', 'oob'],
     r"module 'test:oob@0.1.0' did not load: its init trapped: out of bounds memory access.*alive", None),
    ('phase2-spin', ['phase2', 'spin'],
     r"module 'test:spin@0.1.0' did not load: its init trapped: interrupt.*alive", None),
    ('phase2-grow', ['phase2', 'grow'],
     r"module 'test:grow@0.1.0' did not load: its init answered 7.*alive", None),
    ('phase2-big', ['phase2', 'big'],
     r"module 'test:big@0.1.0' did not load: cannot instantiate.*alive", None),
    ('phase2-shape', ['phase2', 'shape'],
     r"module 'test:shape@0.1.0' did not load: its export 'init' is not .. -> i32.*alive", None),
    ('phase2-noinit', ['phase2', 'noinit'],
     r"module 'test:noinit@0.1.0' did not load: no exported function 'init'.*alive", None),
    ('phase2-abi', ['phase2', 'abi'],
     r"module 'test:ok@0.1.0' is refused: it wants ABI 2 and this is ABI 1.*alive", None),
    ('phase2-nobackend', ['phase2', 'nobackend'],
     r"module 'test:ok@0.1.0' is refused: no backend named 'native'.*alive", None),
    ('phase2-unset', ['phase2', 'unset'],
     r"module 'test:ok@0.1.0' is refused: memory_limit is unset.*alive", None),
    ('phase2-stop', ['phase2', 'stop'],
     r"test:ok@0.1.0: loaded.*the bridge has stopped: module 'test:oob@0.1.0' failed and its on_error is fail.*test:ok@0.1.0: not loaded.*alive", None),
    ('phase2-nolib', ['phase2', 'nolib'],
     r"backend 'wasm': cannot open backend library 'no-such-backend-library'.*bridge_create refused.*alive", None),
    ('phase2-nosink', ['phase2', 'nosink'],
     r"refused: no sink.*alive", None),
    ('phase3-nocall', ['nocall', 'mover.wasm', 'move'],
     None, None),
    ('phase3-share', ['phase3', 'share'],
     r"sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive", None),
    ('phase3-spans', ['phase3', 'spans'],
     r"view:reader@0.1.0/sum: saw 1 500.*game:mover@0.1.0/count: moved 2 1000.*alive", None),
    ('phase3-log', ['phase3', 'log'],
     r"record.0. view:reader@0.1.0/sum: sum 250500 0.*alive", None),
    ('phase3-mirror', ['phase3', 'mirror'],
     r"mirror 100 0.*mirror 100 0.*mirror 100 0.*alive", None),
    ('phase3-fingerprint', ['phase3', 'fingerprint'],
     r"module 'game:fp@0.1.0' did not load: component 'game:Position' has another layout here: fingerprint [0-9a-f]+, and it was declared with [0-9a-f]+.*alive", None),
    ('phase3-grant', ['phase3', 'grant'],
     r"module 'view:reader@0.1.0' did not load: it may not declare 'game:Position': that is another namespace's, and no grant names it.*alive", None),
    ('phase3-wildcard', ['phase3', 'wildcard'],
     r"module 'game:wildcard@0.1.0' did not load: term 0 of system 'any' is a wildcard.*alive", None),
    ('phase3-fuel', ['phase3', 'fuel'],
     r"test:spin@0.1.0/forever: system 'forever' failed: trapped: all fuel consumed.*still here.*all fuel consumed.*still here.*alive", None),
    ('phase3-ceiling', ['phase3', 'ceiling'],
     r"system 'flood' failed: spawn: its spawns of 3000 between two barriers are used up.*system 'flood' failed: spawn: .budget. entities of 4096 is reached.*alive", None),
    ('phase3-ring', ['phase3', 'ring'],
     r"line 15 0.*test:chatty@0.1.0/-: 84 log lines were dropped: its ring holds 16.*alive", None),
    ('phase3-nospans', ['phase3', 'nospans'],
     r"module 'view:reader@0.1.0' is stopped: what its queries match no longer fits in its spans of 16 bytes.*moved 2 1000.*alive", None),
    ('phase3-nocolumns', ['phase3', 'nocolumns', '60000'],
     r"the bridge has stopped: .budget. columns of 60000 bytes ran out.*game:mover@0.1.0: not loaded.*update refused.*alive", None),
    ('phase3-nophase', ['phase3', 'nophase'],
     r"phase 'render' is not one the deployment declared.*alive", None),
    ('phase3-nophases', ['phase3', 'nophases'],
     r"the deployment declares no phase.*bridge_create refused.*alive", None),
    ('phase3-nobudget', ['phase3', 'nobudget'],
     r".budget. columns is unset.*bridge_create refused.*alive", None),
    ('phase3-beyond', ['phase3', 'beyond'],
     r".budget. entities of 1099511627776 is more than an offset of 32 bits can reach.*bridge_create refused.*alive", None),
    ('phase3-toobig', ['phase3', 'toobig'],
     r".budget. cannot be satisfied: the shared memory would be [0-9]+ bytes and an offset into it is 32 bits.*bridge_create refused.*alive", None),
    ('phase3-bigprivate', ['phase3', 'bigprivate'],
     r"module 'game:mover@0.1.0' is refused: its scratch of 4294967040, log_ring of [0-9]+ and spans of 4096 bytes come to more than an offset of 32 bits can reach.*alive", None),
    ('phase3-bench', ['phase3', 'bench'],
     r"rows-unchecked  no fuel   [0-9.]+ ns a row.*alive", None),
    ('phase4-raw', ['phase4', 'raw'],
     r"game:mover@0.1.0/tally: native tally 2 1000.*native tally 2 1000.*alive", None),
    ('phase4-mixed', ['phase4', 'mixed'],
     r"view:reader@0.1.0/sum: sum 250500 0.*native tally 2 1000.*sum 251000 0.*alive", None),
    ('phase4-untrusted', ['phase4', 'untrusted'],
     r"module 'game:mover@0.1.0' is refused: trusted = false needs 'sandbox', which backend 'native' lacks.*alive", r"cannot open"),
    ('phase4-limit', ['phase4', 'limit'],
     r"module 'game:mover@0.1.0' is refused: fuel needs 'meter', which backend 'native' lacks.*alive", r"cannot open"),
    ('phase4-libraries', ['phase4', 'libraries'],
     r"the wasm backend's library is still loaded, the native backend's is (gone|kept by the loader).*alive", None),
    ('phase4-nolib', ['phase4', 'nolib'],
     r"module 'game:mover@0.1.0' did not load: cannot open the library 'no-such-library'.*alive", None),
    ('phase4-refuse', ['phase4', 'refuse'],
     r"test:flood@0.1.0/flood: system 'flood' failed: spawn: its spawns of 1024 between two barriers are used up.*alive", None),
    ('phase5-order', ['phase5', 'order'],
     r"sum 125250 0.*alive", None),
    ('phase5-reorder', ['phase5', 'reorder'],
     r"sum 125500 0.*alive", None),
    ('phase5-ambiguous', ['phase5', 'ambiguous'],
     r"module 'view:halver@0.1.0' did not load: 'game:mover:move' and 'view:halver:halve' both touch 'game:Position' in phase 'update', one of them writing it, and nothing orders them.*alive", None),
    ('phase5-ownorder', ['phase5', 'ownorder'],
     r"game:twostep@0.1.0/show: x 3 0.*alive", None),
    ('phase5-cycle', ['phase5', 'cycle'],
     r".order. contradicts itself.*bridge_create refused.*alive", None),
    ('phase5-observe', ['phase5', 'observe'],
     r"b2 game:setter@0.1.0/mark: marked.*b2 view:watcher@0.1.0/seen: seen 1 77.*alive", None),
    ('phase5-chain', ['phase5', 'chain'],
     r"b1 game:chain@0.1.0/got_a: hop A.*b2 game:chain@0.1.0/got_b: hop B.*b3 game:chain@0.1.0/got_c: hop C.*alive", r"hop A.*hop A"),
    ('phase5-reserved', ['phase5', 'reserved'],
     r"'init' is a reserved name, and no observer may take it.*alive", None),
    ('phase5-parallel', ['phase5', 'parallel'],
     r"on 2 thread.s.[^a-z]*they ran at once.*alive", None),
    ('phase5-serial', ['phase5', 'serial'],
     r"on 1 thread.s.[^a-z]*they ran one after the other.*alive", None),
    ('phase5-pinned', ['phase5', 'pinned'],
     r"the others used [2-4] thread.s.[^a-z]*pinned: 9 of its 9 calls were on the thread that loaded it.*alive", None),
    ('phase5-wrongthread', ['phase5', 'wrongthread'],
     r"module 'test:pinned@0.1.0' is pinned to the thread that loaded it, and this call comes from another.*from another thread: refused.*bridge_destroy is refused: module 'test:pinned@0.1.0' is pinned.*destroy from another thread: refused.*destroy from the thread that loaded it: done.*alive", None),
    ('phase5-nothreads', ['phase5', 'nothreads'],
     r"threads is unset.*bridge_create refused.*alive", None),
    ('phase5-hop', ['phase5', 'hop'],
     r"sum 275000 0.*alive", None),
    ('phase5-events', ['phase5', 'events'],
     r"any: event 3 1.*any: event 2 2.*any: event 4 57005.*alive", None),
    ('phase5-obsconflict', ['phase5', 'obsconflict'],
     r"module 'view:fixer@0.1.0' did not load: 'view:watcher:seen' and 'view:fixer:fix' both touch 'game:Marked' at the barrier, one of them writing it, and nothing orders them.*seen 1 77.*alive", None),
    ('phase5-obsorder', ['phase5', 'obsorder'],
     r"view:fixer@0.1.0: loaded.*seen 1 77.*alive", None),
    ('phase5-obsreorder', ['phase5', 'obsreorder'],
     r"view:fixer@0.1.0: loaded.*seen 1 5.*alive", None),
    ('phase5-nested', ['phase5', 'nested'],
     r"read: body 2 2.*read: more 3 4.*alive", None),
    ('phase5-nested2', ['phase5', 'nested2'],
     r"module 'game:nested@0.1.0' did not load: component 'game:Body': its descriptor does not describe a struct, or names a type its namespace has not declared.*alive", None),
    ('phase4-setting', ['phase4', 'setting'],
     r"backend 'native' would not start: it knows no setting 'cache'.*bridge_create refused.*alive", None),
    ('phase5-two', ['phase5', 'two'],
     r"two bridges: sums 275000 and 265000.*alive", None),
    ('phase3-strings', ['phase3', 'strings'],
     r"game:namer@0.1.0/say: kept in the arena.*alive", None),
    ('phase3-noarena', ['phase3', 'noarena'],
     r"module 'game:namer@0.1.0' did not load: alloc: its persistent of 4 bytes is exhausted.*alive", None),
    ('phase3-bigarena', ['phase3', 'bigarena'],
     r"module 'game:namer@0.1.0' is refused: its persistent of 1048576 bytes does not fit in the 4096 left of .budget. persistent.*alive", None),
    ('phase3-nospawns', ['phase3', 'nospawns'],
     r"module 'game:mover@0.1.0' did not load: spawn: it stated that it spawns nothing.*alive", None),
    ('phase3-undo', ['phase3', 'undo'],
     r"game:wildcard@0.1.0: not loaded.*game:fp@0.1.0: loaded.*alive", None),
    ('phase5-minted', ['phase5', 'minted'],
     r"minted 160 ids a run. on four threads the same, and again the same.*alive", None),
    ('phase6-skipinit', ['phase6', 'skipinit'],
     r"game:badobs@0.1.0: not loaded.*show: x 3 0.*modules: game:twostep@0.1.0=RUNNING .1..*alive", None),
    ('phase6-refusedload', ['phase6', 'refusedload'],
     r"game:wildcard@0.1.0: not loaded.*game:fp@0.1.0: loaded.*alive", None),
    ('phase6-budget', ['phase6', 'budget'],
     r"module 'test:flaky@0.1.0' is stopped: it reached its max_system_failures of 3.*show: x 63 0.*game:twostep@0.1.0=RUNNING test:flaky@0.1.0=STOPPED.*alive", None),
    ('phase6-forgiven', ['phase6', 'forgiven'],
     r"modules: test:flaky@0.1.0=RUNNING.*alive", None),
    ('phase6-notforgiven', ['phase6', 'notforgiven'],
     r"it reached its max_system_failures of 3.*modules: test:flaky@0.1.0=STOPPED.*alive", None),
    ('phase6-neverstop', ['phase6', 'neverstop'],
     r"modules: test:flaky@0.1.0=RUNNING.*alive", None),
    ('phase6-failstop', ['phase6', 'failstop'],
     r"the bridge has stopped: module 'test:flaky@0.1.0' was stopped and its on_error is fail.*update refused.*test:flaky@0.1.0=STOPPED.*game:twostep@0.1.0: not unloaded.*alive", None),
    ('phase6-strip', ['phase6', 'strip'],
     r"left 10 5.*game:keeper@0.1.0: unloaded.*left 9 5.*game:retype@0.1.0: loaded.*alive", None),
    ('phase6-sweep', ['phase6', 'sweep'],
     r"left 10 5.*game:keeper@0.1.0: unloaded.*left 9 0.*game:retype@0.1.0: loaded.*alive", None),
    ('phase6-delete', ['phase6', 'delete'],
     r"left 10 5.*game:keeper@0.1.0: unloaded.*left 0 0.*game:retype@0.1.0: loaded.*alive", None),
    ('phase6-nested-unload', ['phase6', 'nested-unload'],
     r"game:typer@0.1.0: unloaded.*read: body 2 3.*game:retype@0.1.0: not loaded.*game:user@0.1.0: unloaded.*game:retype@0.1.0: loaded.*alive", None),
    ('phase6-nounload', ['phase6', 'nounload'],
     r"no module 'game:nobody@0.1.0' is loaded, so none is unloaded.*alive", None),
    ('phase6-timeouts', ['phase6', 'timeouts'],
     r"its shutdown trapped: interrupt. its teardown continues.*test:hang@0.1.0: unloaded.*modules: .0..*alive", None),
    ('phase6-slots', ['phase6', 'slots'],
     r"module 'game:ticker@0.1.0' is refused: .budget. max_modules of 1 is reached.*game:twostep@0.1.0: unloaded.*game:ticker@0.1.0: loaded.*modules: game:ticker@0.1.0=RUNNING .1..*alive", None),
    ('phase6-reload', ['phase6', 'reload'],
     r"tick 102 1.*game:ticker@0.1.0/-: shutdown 1 0.*game:ticker@0.2.0: reloaded.*tick 112 10.*tick 122 10.*alive", None),
    ('phase6-reloadlayout', ['phase6', 'reloadlayout'],
     r"was not reloaded: component 'game:Tick' has another layout here: fingerprint [0-9a-f]+, and it was declared with [0-9a-f]+.*tick 103 1.*tick 104 1.*alive", None),
    ('phase6-reloadfail', ['phase6', 'reloadfail'],
     r"was not reloaded: its reload answered 5.*tick 103 1.*tick 104 1.*game:ticker@0.1.0=RUNNING.*alive", None),
    ('phase6-sharegrow', ['phase6', 'sharegrow'],
     r"game:ticker@0.2.0: reloaded.*tick 112 10.*tick 122 10.*alive", None),
    ('phase6-sharechain', ['phase6', 'sharechain'],
     r"game:namer@0.1.0: loaded.*game:ticker@0.2.0: reloaded.*tick 112 10.*alive", None),
    ('phase6-sharesame', ['phase6', 'sharesame'],
     r"was not reloaded: alloc: its persistent of 16 bytes is exhausted.*tick 103 1.*tick 104 1.*alive", None),
    ('phase6-shareless', ['phase6', 'shareless'],
     r"is not reloaded: its persistent is 16 and a reload states less, which nothing can give back.*tick 104 1.*alive", None),
    ('phase6-sharebig', ['phase6', 'sharebig'],
     r"is not reloaded: its persistent of 1048576 bytes does not fit in the [0-9]+ left of .budget. persistent.*tick 104 1.*alive", None),
    ('phase6-repair', ['phase6', 'repair'],
     r"the bridge has stopped: module 'game:ticker@0.1.0' was stopped and its on_error is fail.*game:twostep@0.1.0: not loaded.*the bridge runs again: module 'game:ticker@0.2.0' is back.*game:ticker@0.2.0: reloaded.*tick 1[0-9]3 10.*game:ticker@0.2.0=RUNNING.*alive", None),
    ('phase6-repairload', ['phase6', 'repairload'],
     r"the bridge has stopped: module 'game:thing@0.1.0' failed and its on_error is fail.*game:other@0.1.0: not loaded.*the bridge runs again: module 'game:thing@0.2.0' is back.*game:thing@0.2.0: loaded.*show: x 3 0.*alive", None),
    ('phase6-reloadmove', ['phase6', 'reloadmove'],
     r"it runs on backend 'wasm' and a reload does not move it to 'native'.*tick 104 1.*alive", None),
    ('phase6-reloaddrop', ['phase6', 'reloaddrop'],
     r"game:ticker@0.2.0: reloaded.*tick 122 10.*game:retype@0.1.0: loaded.*alive", None),
    ('phase6-maynot', ['phase6', 'maynot'],
     r"is not reloaded: its hot_reload is false.*tick 104 1.*alive", None),
    ('phase6-revive', ['phase6', 'revive'],
     r"game:ticker@0.1.0=STOPPED.*game:ticker@0.2.0: reloaded.*tick 1[0-9]3 10.*game:ticker@0.2.0=RUNNING.*alive", None),
    ('phase6-noreload', ['phase6', 'noreload'],
     r"module 'test:noreload@0.1.0' did not load: no exported function 'reload'.*alive", None),
    ('phase6-notreloadable', ['phase6', 'notreloadable'],
     r"module 'game:twostep@0.1.0' is refused: hot_reload is stated, and its author states it is not reloadable.*alive", None),
    ('phase6-reloadnothing', ['phase6', 'reloadnothing'],
     r"is not reloaded: nothing of that name is loaded.*alive", None),
    ('phase6-nreload', ['phase6', 'nreload'],
     r"tick 102 1.*game:ticker@0.1.0/-: shutdown 1 0.*game:ticker@0.2.0: reloaded.*tick 112 10.*tick 122 10.*alive", None),
    ('phase6-nreloadfail', ['phase6', 'nreloadfail'],
     r"shutdown 1 0.*was not reloaded: its reload answered 5.*module 'game:ticker@0.1.0' is stopped: its reload failed after its old instance had been shut down. What it declared is kept.*game:ticker@0.1.0=STOPPED.*game:ticker@0.3.0: reloaded.*tick 112 10.*tick 122 10.*game:ticker@0.3.0=RUNNING.*alive", None),
    ('phase6-nreloadfailunload', ['phase6', 'nreloadfailunload'],
     r"game:ticker@0.1.0=STOPPED.*game:ticker@0.1.0: unloaded.*modules: game:twostep@0.1.0=RUNNING .1..*alive", None),
    ('phase6-config', ['phase6', 'config'],
     r"vsync width 1 1280.*scale mode 150 2.*show: hello 7 0.*show: some/long/path.cfg 0 0.*flags ratio 3 3.*show: big 2 0.*alive", None),
    ('phase6-configkey', ['phase6', 'configkey'],
     r"its settings have a key 'vsynk', and 'game:Settings' has no field of that name.*alive", None),
    ('phase6-configkind', ['phase6', 'configkind'],
     r"its setting 'scale' is text, and the field it names is a float.*alive", None),
    ('phase6-configlong', ['phase6', 'configlong'],
     r"its setting 'title' is 20 bytes of text, and the field it names holds 16 with the zero that ends it.*alive", None),
    ('phase6-configrange', ['phase6', 'configrange'],
     r"its setting 'small' is 300, which the field it names cannot hold.*alive", None),
    ('phase6-configenum', ['phase6', 'configenum'],
     r"its setting 'mode' is 'Sometimes', which the enum of the field it names has no constant for.*alive", None),
    ('phase6-confignone', ['phase6', 'confignone'],
     r"it is handed 10 settings, and states that it has no config component.*alive", None),
    ('phase6-configunset', ['phase6', 'configunset'],
     r"module 'game:settings@0.1.0' is refused: config is unset.*alive", None),
    ('phase6-configmissing', ['phase6', 'configmissing'],
     r"its config is 'game:Nope', which is not a struct its init declared.*alive", None),
    ('phase6-configarena', ['phase6', 'configarena'],
     r"its settings hold 24 bytes of text, and its persistent share has 8 left.*alive", None),
    ('phase6-configtwice', ['phase6', 'configtwice'],
     r"its config is 'game:Settings', which is already the settings of module 'game:settings@0.1.0'.*alive", None),
    ('phase6-configreload', ['phase6', 'configreload'],
     r"vsync width 1 1280.*game:settings@0.2.0: reloaded.*vsync width 1 1280.*alive", None),
    ('phase6-configdrop', ['phase6', 'configdrop'],
     r"game:settings@0.2.0: reloaded.*game:retype@0.1.0: loaded.*alive", None),
    ('phase6-reseed', ['phase6', 'reseed'],
     r"settings text written to the same place.*alive", None),
    ('phase6-nreloadfailstop', ['phase6', 'nreloadfailstop'],
     r"the bridge has stopped: module 'game:ticker@0.1.0' could not be reloaded and its on_error is fail.*update refused.*the bridge runs again: module 'game:ticker@0.3.0' is back.*tick 112 10.*game:ticker@0.3.0=RUNNING.*alive", None),
    ('phase6-ntimeout', ['phase6', 'ntimeout'],
     r"module 'game:ticker@0.1.0' is refused: shutdown_timeout needs 'deadline', which backend 'native' lacks.*alive", r"cannot open"),
    ('phase6-configceiling', ['phase6', 'configceiling', '440'],
     r"the row its settings go on does not fit under .budget. entities of 440.*alive", None),
    ('phase4-nopath', ['phase4', 'nopath'],
     r"module 'game:mover@0.1.0' did not load: this backend opens a module's library by its path, and was handed none.*alive", None),
    ('phase4-nobytes', ['phase4', 'nobytes'],
     r"module 'view:reader@0.1.0' did not load: this backend compiles a module from its bytes, and was handed none.*alive", None),
    ('phase6-churn', ['phase6', 'churn'],
     r"churned 200 times, 0 records of the index out of step.*alive", None),
    ('tables', ['tables'],
     r"a table flecs swept as empty: retired.*the table of a deleted parent's children: retired.*reused.*alive", None),
    ('phase6-destroy', ['phase6', 'destroy'],
     r"tick 101 1.*game:ticker@0.1.0/-: shutdown 1 0.*module 'test:hang@0.1.0': its shutdown trapped: interrupt. its teardown continues.*alive", None),
]

if __name__ == '__main__':
    main()
