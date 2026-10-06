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
# sits beside it, and wasmtime's files under backends/wasm/wasmtime: the one that backend is linked
# against, and the one it loads.
TARGETS = {
    'x86_64-windows-gnu': {
        'runs on': ('Windows', 'AMD64'), 'program': '.exe', 'library': '.dll',
        'compile': [], 'link': ['-lws2_32', '-ldbghelp'], 'beside': [], 'named': [],
        'wasmtime': ('windows/libwasmtime.dll.a', 'windows/wasmtime.dll')},
    'x86_64-linux-gnu': {
        'runs on': ('Linux', 'x86_64'), 'program': '', 'library': '.so',
        'compile': ['-fPIC', '-D_DEFAULT_SOURCE'], 'link': ['-lm', '-lpthread', '-ldl'],
        'beside': ['-Wl,-rpath,$ORIGIN'], 'named': ['-Wl,-soname,bridge.so'],
        'wasmtime': ('linux/libwasmtime.so', 'linux/libwasmtime.so')},
    'aarch64-macos': {
        'runs on': ('Darwin', 'arm64'), 'program': '', 'library': '.dylib',
        'compile': ['-fPIC', '-D_DARWIN_C_SOURCE'], 'link': [],
        'beside': ['-Wl,-rpath,@loader_path'], 'named': ['-Wl,-install_name,@rpath/bridge.dylib'],
        'wasmtime': ('macos/libwasmtime.dylib', 'macos/libwasmtime.dylib')},
}
BINARYEN = 'binaryen-version_133'
BINARYEN_URL = ('https://github.com/WebAssembly/binaryen/releases/download/version_133/'
                'binaryen-version_133-x86_64-windows.tar.gz')
BINARYEN_SHA256 = '17a2cbeac6b5693c5fbafab3838d3c65fd9c1eb38b05f5baec6c657e8c84995b'
FEATURES = ['--enable-multimemory', '--enable-bulk-memory', '--enable-sign-ext',
            '--enable-nontrapping-float-to-int', '--enable-mutable-globals']
TIMEOUT = 120

BRIDGE = ['bridge', 'lifecycle', 'config', 'block', 'storage', 'schema', 'abi', 'schedule', 'pool',
          'backend']
# a native module is a shared library: the name, its source, what it is compiled with. The same
# library twice is one module to the loader, so one source is built under several names.
NATIVE = [
    ('nmover', 'nmover', []), ('nflood', 'nflood', []), ('nsetter', 'nsetter', []),
    ('nsleep1', 'nsleep', []), ('nsleep2', 'nsleep', []), ('npinned', 'npinned', []),
    ('nforeign', 'nforeign', []),
    ('nticker1', 'nticker', ['STEP=1']), ('nticker2', 'nticker', ['STEP=10']),
    ('nticker3', 'nticker', ['STEP=10', 'FAIL_RELOAD']),
    ('nstore', 'nstore', []), ('nstorebig', 'nstore', ['BIG']), ('nstoretab', 'nstore', ['TABLES']),
    ('nstorefat64', 'nstore', ['BIG', 'FAT64']), ('nstorefat256', 'nstore', ['BIG', 'FAT256']),
    ('nstorefat1k', 'nstore', ['BIG', 'FAT1K']), ('nstorefat4k', 'nstore', ['BIG', 'FAT4K']),
    ('nstamp5', 'nstamp', ['VALUE=5']), ('nstamp9', 'nstamp', ['VALUE=9']), ('nreaper', 'nreaper', []),
    ('nstampunstated', 'nstamp', ['VALUE=1', 'UNSTATED']),
    ('nstamptick', 'nstamp', ['VALUE=7', 'PHASE="tick"']),
    ('nfrag1', 'nfrag', ['SPLIT=1']), ('nfrag16', 'nfrag', ['SPLIT=16']),
    ('nfrag256', 'nfrag', ['SPLIT=256']), ('nfrag4096', 'nfrag', ['SPLIT=4096']),
    ('nsparse', 'nsparse', []), ('nsparsebig', 'nsparse', ['BIG']), ('nsparsefat', 'nsparse', ['FAT']),
    ('nsparsebad1', 'nsparse', ['BAD=1']), ('nsparsebad2', 'nsparse', ['BAD=2']),
    ('nmove1', 'nmove', ['KEPT=1']), ('nmove2', 'nmove', ['KEPT=2']), ('nmove3', 'nmove', ['KEPT=3']),
    ('nmove4', 'nmove', ['KEPT=4']), ('nmove5', 'nmove', ['KEPT=5']),
    ('ntag', 'ntag', []), ('ntagbad', 'ntag', ['BAD']), ('nfragtag4096', 'nfrag', ['SPLIT=4096', 'TAGS']),
    ('nsingle', 'nsingle', []), ('nsinglebad1', 'nsingle', ['BAD=1']),
    ('nsinglebad2', 'nsingle', ['BAD=2']), ('nsinglebad3', 'nsingle', ['BAD=3']),
    ('ndirect', 'ndirect', []), ('ndirectbig', 'ndirect', ['BIG']), ('ndirectbad', 'ndirect', ['BAD']),
    ('ntoggle', 'ntoggle', []), ('ntoggle1', 'ntoggle', ['BIG', 'ON=1']),
    ('ntoggle2', 'ntoggle', ['BIG', 'ON=2']), ('ntoggle16', 'ntoggle', ['BIG', 'ON=16']),
    ('nweigh1', 'nweigh', ['ON=1']), ('nweigh2', 'nweigh', ['ON=2']), ('nweigh16', 'nweigh', ['ON=16']),
    ('nstress1', 'nstress', ['SEED=1u', 'SPACE="s1"']), ('nstress2', 'nstress', ['SEED=2u', 'SPACE="s2"']),
    ('nstress3', 'nstress', ['SEED=3u', 'SPACE="s3"']), ('nstress4', 'nstress', ['SEED=4u', 'SPACE="s4"']),
]

# a module in C: the name, its source, whether its rows are tested, what it is compiled with.
# `checked` or `unchecked` is stated for each, as the SDK asks: BRIDGE_ROWS_CHECKED, or not.
PLAIN = ('mover reader refs fp badterm spin3 flood chatty rows halver twostep watcher chain '
         'badobs events fixer nested nested2 namer spawner counter typer user settings hang hit '
         'shield aim')
WASM = [(name, name, 'checked', []) for name in PLAIN.split()] + [
    ('rows-unchecked', 'rows', 'unchecked', []),
    ('kept-unset', 'kept', 'checked', ['KEPT=0']), ('kept-none', 'kept', 'checked', ['KEPT=9']),
    ('kept-sparse', 'kept', 'checked', ['KEPT=BRIDGE_SPARSE_SET']),
    ('namer-reader', 'namer', 'checked', ['READER']),
    ('flaky-always', 'flaky', 'checked', ['OF=1', 'FAILS=1']),
    ('flaky-third', 'flaky', 'checked', ['OF=3', 'FAILS=1']),
    ('flaky-twothirds', 'flaky', 'checked', ['OF=3', 'FAILS=2']),
    ('flaky-calm', 'flaky', 'checked', ['OF=1', 'FAILS=1', 'CALM']),
    ('keeper-strip', 'keeper', 'checked', ['PRIVATE_POLICY=BRIDGE_STRIP']),
    ('keeper-sweep', 'keeper', 'checked', ['PRIVATE_POLICY=BRIDGE_SWEEP']),
    ('keeper-delete', 'keeper', 'checked', ['PRIVATE_POLICY=BRIDGE_DELETE']),
    ('retype-private', 'retype', 'checked', ['WHICH=1']),
    ('retype-vec2', 'retype', 'checked', ['WHICH=2']),
    ('retype-extra', 'retype', 'checked', ['WHICH=3']),
    ('retype-settings', 'retype', 'checked', ['WHICH=4']),
    ('settings-drop', 'settings', 'checked', ['DROP']),
    ('settings-table', 'settings', 'checked', ['KEPT=BRIDGE_TABLE']),
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
    wasmtime = ROOT / 'backends/wasm/wasmtime'
    link_wasmtime, runtime = (wasmtime / f for f in row['wasmtime'])

    cc = [zig, 'cc', '-target', target] + (['-O0', '-g'] if debug else ['-O2', '-DNDEBUG'])
    cc += row['compile']
    # an error, since zig says a warning once and then answers from its cache without it. Members
    # an initializer leaves out are zero, which C states.
    ours = cc + ['-std=c23', '-pedantic-errors', '-Wall', '-Wextra', '-Werror',
                 '-Wno-missing-field-initializers']
    system, here = row['link'], row['beside']
    include, src = '-I%s' % (ROOT / 'include'), '-I%s' % (ROOT / 'src')
    # how a module is written is its backend's: each backend's binding sits beside the backend
    sdk = {b: '-I%s' % (ROOT / 'backends' / b / 'sdk') for b in ('wasm', 'native')}
    headers = [Path(__file__)] + [h for d in ('include', 'src', 'tests', 'tests/modules',
                                              'backends/wasm', 'backends/wasm/sdk',
                                              'backends/native', 'backends/native/sdk')
                                  for h in (ROOT / d).glob('*.h')]

    # ---- objects ----
    defines = ['-D%s="%s"' % (key, path.as_posix()) for key, path in
               [('BACKEND_PATH', backends / ('bridge-wasm' + lib)),
                ('NATIVE_PATH', backends / ('bridge-native' + lib)),
                ('NATIVE_OTHER_PATH', backends / ('bridge-native-other' + lib)), ('MODULE_DIR', modules)] +
               [('%s_PATH' % name.upper(), out / (name + lib)) for name, _, _ in NATIVE]]
    compiles = []

    def compile_(name, source, flags, deps=()):
        compiles.append(('compiling %s' % name, [obj / (name + '.o')],
                         [source] + headers + list(deps),
                         [flags + ['-c', source, '-o', obj / (name + '.o')]]))
        return obj / (name + '.o')

    # the library's own objects: what bridge.h marks is exported, and nothing else is
    inside = ours + ['-DBRIDGE_BUILDING', '-fvisibility=hidden']
    os_o = compile_('os', ROOT / 'src/os.c', inside + [src])
    bridge_o = [compile_(name, ROOT / 'src' / (name + '.c'), inside + [include, src]) for name in BRIDGE]
    # a backend is compiled against the backend seam and what is its own: nothing of src/
    wasm_o = [compile_('wasm', ROOT / 'backends/wasm/wasm.c',
                       ours + [include, '-I%s' % (wasmtime / 'include')]),
              compile_('wasm-os', ROOT / 'backends/wasm/os.c', ours)]
    native_o = [compile_('native', ROOT / 'backends/native/native.c', ours + [include, sdk['native']]),
                compile_('native-os', ROOT / 'backends/native/os.c', ours)]
    native_other_o = compile_('native-other', ROOT / 'backends/native/native.c',
                              ours + [include, sdk['native'], '-DSAYS_ABI=(BACKEND_ABI+1)'])
    schema_test_o = compile_('schema-test', ROOT / 'tests/schema.c', ours + [include, src])
    adopt_o = compile_('adopt', ROOT / 'tests/adopt.c', ours + [include, src] + defines)
    phase_o = {name: compile_(name, ROOT / 'tests' / (name + '.c'), ours + [include] + defines)
               for name in ('stage2', 'stage3', 'stage4', 'stage5', 'stage6')}
    storage_o = compile_('storage-test', ROOT / 'tests/storage.c', ours + [include, src] + defines)
    # the library again, with its calls to the allocator sent to the test that counts them
    counting = ['-D%s=counted_%s' % (f, f) for f in ('malloc', 'calloc', 'realloc', 'free')]
    counted_o = [compile_('counted-' + name, ROOT / 'src' / (name + '.c'), ours + [include, src] + counting)
                 for name in BRIDGE + ['os']]
    counted_test_o = compile_('storage-counted-test', ROOT / 'tests/storage.c',
                              ours + [include, src] + defines + ['-DCOUNTED'])
    module_o = {name: compile_(name, ROOT / 'tests/modules/native' / (source + '.c'),
                               ours + [include, sdk['native']] + ['-D' + d for d in flags])
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
    link('backends/bridge-wasm', wasm_o, [link_wasmtime] + system + here, shared=True)
    link('backends/bridge-native', native_o, system, shared=True)
    # the same backend, saying it is of another backend seam: for the case that refuses it
    link('backends/bridge-native-other', [native_other_o, native_o[1]], system, shared=True)
    for name, _, _ in NATIVE:
        link(name, [module_o[name]], shared=True)
    link('adopt', [adopt_o, obj / 'backend.o', os_o], system)
    link('schema', [schema_test_o, obj / 'schema.o'], system)
    whole = bridge_o + [os_o]
    # the library, built: shared and static. What a host links - host-api.md
    shared_lib, static_lib = out / ('bridge' + lib), out / 'bridge.a'
    link('bridge', whole, system + row['named'], shared=True)
    links.append(('archiving bridge.a', [static_lib], whole,
                  [lambda: static_lib.unlink(missing_ok=True), [zig, 'ar', 'rcs', static_lib] + whole]))
    together(links)
    del links[:]
    # a host is compiled against include/ alone and linked with the library as a host would be:
    # the shared one, and one of them the static one as well
    for name, o in phase_o.items():
        link(name, [o, shared_lib], system + here)
    link('stage2-static', [phase_o['stage2'], static_lib], system)
    # these three reach inside, to test it, and so are linked with its objects
    link('storage', [storage_o] + whole, system)
    link('storage-counted', [counted_test_o] + counted_o, system)
    together(links)

    # ---- wasm modules: text assembled, C compiled and merged with the accessors ----
    accessors = ROOT / 'backends/wasm/sdk/bridge-wasm.wat'
    acc = modules / 'acc.wasm'
    assembled = [('assembling %s' % wat.name, [modules / (wat.stem + '.wasm')], [wat],
                  [[wasm_tools, 'parse', wat, '-o', modules / (wat.stem + '.wasm')]])
                 for wat in sorted((ROOT / 'tests/modules/wasm').glob('*.wat'))]
    assembled += [('assembling the accessors', [acc], [accessors],
                   [[wasm_tools, 'parse', accessors, '-o', acc]])]
    together(assembled)
    compiled = []
    for name, source, kind, flags in WASM:
        raw, merged, final = (modules / (name + s) for s in ('.raw.wasm', '.merged.wasm', '.wasm'))
        c = ROOT / 'tests/modules/wasm' / (source + '.c')
        compiled.append(('building module %s' % name, [final], [c, acc] + headers, [
            [zig, 'cc', '-target', 'wasm32-freestanding', '-std=c23', '-pedantic-errors', '-Wall',
             '-Wextra', '-Werror', '-Wno-missing-field-initializers', '-O2', '-nostdlib'] +
            ['-D' + d for d in flags + ['BRIDGE_ROWS_' + kind.upper()]] +
            [include, sdk['wasm'], '-Wl,--no-entry', '-o', raw, c],
            [merge, raw, 'main', acc, 'acc'] + FEATURES + ['-o', merged],
            [opt, merged] + FEATURES + ['-O3', '--remove-exports=acc_*',
                                         '--remove-unused-module-elements', '-o', final]]))
    together(compiled)
    return {'python': sys.executable, 'nocall': ROOT / 'tools/nocall.py',
            'wasm-tools': wasm_tools, 'modules': modules, 'exe': exe}


def test(out, built, only):
    """Each case by what it said. With a pattern, the pattern decides and the exit code does
    not: several cases are a refusal that ends the program. Without one, the exit code does."""
    cases = [c for c in CASES if not only or only in c[0]]
    failed = []
    for name, command, match, never in cases:
        argv = [str(out / (command[0] + built['exe']))] + command[1:]
        if command[0] == 'c23':
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
    ('schema', ['schema'],
     r"every descriptor read as it should, and every other refused by its word.*alive", None),
    ('storage-changes', ['storage', 'changes'],
     '@tests/storage-changes.txt', None),
    ('alloc-changes', ['storage-counted', 'changes'],
     '@tests/storage-changes.txt', None),
    ('alloc-tables', ['storage-counted', 'tables'],
     '@tests/storage-tables.txt', None),
    ('alloc-strip', ['storage-counted', 'strip'],
     '@tests/storage-strip.txt', None),
    ('storage-shape', ['storage', 'shape'],
     r"the shared block reserves more than it uses: yes.*a native module's private block reserves what it uses: yes.*the shared block is whole wasm pages: yes.*a native module's private block is not rounded to one: yes.*alive", None),
    ('alloc-nomemory', ['storage-counted', 'nomemory'],
     r"more than forty allocations were each answered null in turn.*alive", 'FAIL'),
    ('storage-tables', ['storage', 'tables'],
     '@tests/storage-tables.txt', None),
    ('storage-stress', ['storage', 'stress'],
     '@tests/storage-stress.txt', None),
    ('storage-stress1', ['storage', 'stress1'],
     '@tests/storage-stress.txt', None),
    ('storage-notables', ['storage', 'notables'],
     '@tests/storage-notables.txt', None),
    ('storage-nocolumns', ['storage', 'nocolumns'],
     '@tests/storage-nocolumns.txt', None),
    ('storage-scribble', ['storage', 'scribble'],
     '@tests/storage-scribble.txt', None),
    ('storage-strip', ['storage', 'strip'],
     '@tests/storage-strip.txt', None),
    ('storage-stripfull', ['storage', 'stripfull'],
     '@tests/storage-stripfull.txt', None),
    ('storage-weigh', ['storage', 'weigh'],
     r"an entity of 4096 bytes.*in    1 table.*in 4096 tables.*a sparse set.*of 4096 bytes.*a toggle, one row in 16 on.*direct-indexed.*in no order.*as tags.*alive", r"FAIL"),
    ('storage-toggle', ['storage', 'toggle'], '@tests/storage-toggle.txt', None),
    ('storage-move', ['storage', 'move'], '@tests/storage-move.txt', None),
    ('storage-tag', ['storage', 'tag'], '@tests/storage-tag.txt', None),
    ('storage-tagbad', ['storage', 'tagbad'],
     r"attach hands 'st:M' 4 bytes and it is 0.*alive", None),
    ('storage-single', ['storage', 'single'], '@tests/storage-single.txt', None),
    ('storage-singlebad1', ['storage', 'singlebad1'],
     r"did not load: term 0 of system 'by': 'st:G' is a singleton, and is on no entity a system "
     r"walks.*alive", None),
    ('storage-singlebad2', ['storage', 'singlebad2'],
     r"attach: 'st:G' is a singleton, and its id is 0.*alive", None),
    ('storage-singlebad3', ['storage', 'singlebad3'],
     r"attach: the id 0 is no entity, and 'st:A' is not a singleton.*alive", None),
    ('storage-direct', ['storage', 'direct'], '@tests/storage-direct.txt', None),
    ('storage-directstrip', ['storage', 'directstrip'], '@tests/storage-directstrip.txt', None),
    ('storage-directsweep', ['storage', 'directsweep'], '@tests/storage-directsweep.txt', None),
    ('storage-directdelete', ['storage', 'directdelete'], '@tests/storage-directdelete.txt', None),
    ('storage-directnone', ['storage', 'directnone'],
     r"did not load: type 'st:D' is one more than .budget. direct_indexed of 0.*alive", None),
    ('storage-directfull', ['storage', 'directfull'],
     r"did not load: type 'st:D' is direct-indexed, and .budget. columns of 1024 bytes has no room for "
     r"a cell of 4 bytes for each of 400000 entities.*alive", None),
    ('storage-directbad', ['storage', 'directbad'],
     r"did not load: term 1 of system 'both': 'st:D' is direct-indexed, and does not select what a "
     r"system walks.*alive", None),
    ('storage-sparse', ['storage', 'sparse'], '@tests/storage-sparse.txt', None),
    ('storage-sparsefull', ['storage', 'sparsefull'], '@tests/storage-sparsefull.txt', None),
    ('storage-sparsestrip', ['storage', 'sparsestrip'], '@tests/storage-sparsestrip.txt', None),
    ('storage-sparsesweep', ['storage', 'sparsesweep'], '@tests/storage-sparsesweep.txt', None),
    ('storage-sparsesweepa', ['storage', 'sparsesweepa'], '@tests/storage-sparsesweepa.txt', None),
    ('storage-sparsedelete', ['storage', 'sparsedelete'], '@tests/storage-sparsedelete.txt', None),
    ('storage-sparsenone', ['storage', 'sparsenone'],
     r"did not load: type 'st:S' is one more than .budget. sparse_sets of 0.*alive", None),
    ('storage-sparsebad1', ['storage', 'sparsebad1'],
     r"did not load: term 1 of system 'both': 'st:S' is kept in a sparse set, and does not select the "
     r"rows of a table.*alive", None),
    ('storage-sparsebad2', ['storage', 'sparsebad2'],
     r"did not load: term 0 of system 'without': 'st:S' is kept in a sparse set, and a system cannot "
     r"walk the entities that are not in it.*alive", None),
    ('storage-baseline', ['storage', 'baseline'],
     r"read in order.*in no order.*204800 of them on.*found through its query.*alive", r"FAIL"),
    ('storage-measure', ['storage', 'measure'],
     r"an id handed out +[0-9.]+ ns each.*ten thousand tables made.*the stress run, on 1 thread.*alive", None),
    ('adopt', ['adopt'],
     None, None),
    ('stage2-ok', ['stage2', 'ok'],
     r"test:ok@0.1.0: loaded.*alive", None),
    ('stage2-nolimit', ['stage2', 'nolimit'],
     r"test:ok@0.1.0: loaded.*alive", None),
    ('stage2-image', ['stage2', 'image'],
     r"test:image@0.1.0: loaded.*alive", None),
    ('stage2-oob', ['stage2', 'oob'],
     r"module 'test:oob@0.1.0' did not load: its init trapped: out of bounds memory access.*alive", None),
    ('stage2-spin', ['stage2', 'spin'],
     r"module 'test:spin@0.1.0' did not load: its init trapped: interrupt.*alive", None),
    ('stage2-grow', ['stage2', 'grow'],
     r"module 'test:grow@0.1.0' did not load: its init answered 7.*alive", None),
    ('stage2-big', ['stage2', 'big'],
     r"module 'test:big@0.1.0' did not load: cannot instantiate.*alive", None),
    ('stage2-shape', ['stage2', 'shape'],
     r"module 'test:shape@0.1.0' did not load: its export 'init' is not .. -> i32.*alive", None),
    ('stage2-shape64', ['stage2', 'shape64'],
     r"module 'test:shape64@0.1.0' did not load: its export 'init' is not .. -> i32.*alive", None),
    ('stage2-noinit', ['stage2', 'noinit'],
     r"module 'test:noinit@0.1.0' did not load: no exported function 'init'.*alive", None),
    ('stage2-abi', ['stage2', 'abi'],
     r"module 'test:ok@0.1.0' is refused: it wants ABI 2 and this is ABI 1.*alive", None),
    ('stage2-nobackend', ['stage2', 'nobackend'],
     r"module 'test:ok@0.1.0' is refused: no backend named 'native'.*alive", None),
    ('stage2-unsetlevel', ['stage2', 'unsetlevel'],
     r"module 'test:ok@0.1.0' is refused: log_level is unset.*alive", ': loaded'),
    ('stage2-unset', ['stage2', 'unset'],
     r"module 'test:ok@0.1.0' is refused: memory_limit is unset.*alive", None),
    ('stage2-nonelimit', ['stage2', 'nonelimit'],
     r"module 'test:ok@0.1.0' is refused: spawns is -1, which is neither an amount nor BRIDGE_NONE.*module 'test:ok@0.1.0' is refused: work_limit is -2, which is neither a limit nor BRIDGE_NO_LIMIT.*alive", ': loaded'),
    ('stage2-hostabi', ['stage2', 'hostabi'],
     r"the host is of host ABI 2, and this is 1.*bridge_create refused.*alive", None),
    ('stage2-hostabiunset', ['stage2', 'hostabiunset'],
     r"abi is unset.*bridge_create refused.*alive", None),
    ('stage2-unsetflag', ['stage2', 'unsetflag'],
     r"module 'test:ok@0.1.0' is refused: pinned is unset.*module 'test:ok@0.1.0' is refused: trusted is 3, which is neither BRIDGE_TRUE nor BRIDGE_FALSE.*alive", ': loaded'),
    ('stage2-negative', ['stage2', 'negative'],
     r"module 'test:ok@0.1.0' is refused: memory_limit is -7, which is neither a limit nor BRIDGE_NO_LIMIT.*alive", ': loaded'),
    ('stage2-stop', ['stage2', 'stop'],
     r"test:ok@0.1.0: loaded.*the bridge has stopped: module 'test:oob@0.1.0' failed and its on_error is fail.*test:ok@0.1.0: not loaded.*alive", None),
    ('stage2-nolib', ['stage2', 'nolib'],
     r"backend 'wasm': cannot open backend library 'no-such-backend-library'.*bridge_create refused.*alive", None),
    ('stage2-nosink', ['stage2', 'nosink'],
     r"refused: no sink.*alive", None),
    ('stage3-nocall', ['nocall', 'mover.wasm', 'move'],
     None, None),
    ('stage3-share', ['stage3', 'share'],
     r"sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive", None),
    ('stage3-spans', ['stage3', 'spans'],
     r"view:reader@0.1.0/sum: saw 1 500.*game:mover@0.1.0/count: moved 2 1000.*alive", None),
    ('stage3-log', ['stage3', 'log'],
     r"record.6. view:reader@0.1.0/sum: sum 250500 0.*alive", None),
    ('stage3-mirror', ['stage3', 'mirror'],
     r"mirror 100 0.*mirror 100 0.*mirror 100 0.*alive", None),
    ('stage3-keptunset', ['stage3', 'keptunset'],
     r"module 'game:kept@0.1.0' did not load: type 'game:Position': its storage layout is unset.*"
     r"module 'game:kept@0.1.0' did not load: type 'game:Position' is stated as a sparse set here, and it "
     r"was declared as a table. It is also declared by 'game:mover@0.1.0'.*alive", None),
    ('stage3-direct', ['stage3', 'direct'],
     r"game:aim@0.1.0/reached: aims 10 5.*game:aim@0.1.0/reached: aimed 20 0.*alive", r"did not load"),
    ('stage3-toggle', ['stage3', 'toggle'],
     r"game:shield@0.1.0/count: shields 10 10.*game:shield@0.1.0/count: strength 45 0.*"
     r"game:shield@0.1.0/count: shields 10 6.*game:shield@0.1.0/count: strength 27 0.*alive", r"did not load"),
    ('stage3-sparse', ['stage3', 'sparse'],
     r"game:hit@0.1.0/walked: hits 4 18.*game:hit@0.1.0/reached: bodies 10 4.*"
     r"game:hit@0.1.0/reached: reached 18 0.*alive", r"did not load"),
    ('stage3-fingerprint', ['stage3', 'fingerprint'],
     r"module 'game:fp@0.1.0' did not load: type 'game:Position' has another component layout here: fingerprint [0-9a-f]+, and it was declared with [0-9a-f]+. It is also declared by 'game:mover@0.1.0'.*alive", None),
    ('stage3-grant', ['stage3', 'grant'],
     r"module 'view:reader@0.1.0' did not load: it may not declare 'game:Position': that is another namespace's, and no grant names it.*alive", None),
    ('stage3-badterm', ['stage3', 'badterm'],
     r"module 'game:badterm@0.1.0' did not load: term 0 of system 'any': its operator is 4, and there are three.*alive", None),
    ('stage3-smallscratch', ['stage3', 'smallscratch'],
     r"game:mover@0.1.0: loaded.*moved.*alive", 'not loaded'),
    ('stage3-worklimit', ['stage3', 'worklimit'],
     r"test:spin@0.1.0/forever: system 'forever' failed: trapped: all fuel consumed.*still here.*all fuel consumed.*still here.*alive", None),
    ('stage3-ceiling', ['stage3', 'ceiling'],
     r"system 'flood' failed: spawn: its spawns of 3000 between two barriers are used up.*system 'flood' failed: spawn: .budget. entities of 4096 is reached.*alive", None),
    ('stage3-ring', ['stage3', 'ring'],
     r"line 15 0.*test:chatty@0.1.0/-: 84 log lines were dropped: its ring holds 16.*alive", None),
    ('stage3-nospans', ['stage3', 'nospans'],
     r"module 'view:reader@0.1.0' did not load: its spans of 16 bytes do not hold a column_base of 2 components by \d+ tables.*moved 2 1000.*alive", None),
    ('stage3-nocolumns', ['stage3', 'nocolumns', '4096'],
     r"module 'game:mover@0.1.0': .budget. columns of 4096 bytes is used up. Not applied, of the changes it asked: 1244 .*moved 1 256.*alive", r"has stopped"),
    ('stage3-nophase', ['stage3', 'nophase'],
     r"phase 'render' is not one the deployment declared.*alive", None),
    ('stage3-nophases', ['stage3', 'nophases'],
     r"the deployment declares no phase.*bridge_create refused.*alive", None),
    ('stage3-unsettables', ['stage3', 'unsettables'],
     r".budget. tables is unset.*bridge_create refused.*alive", None),
    ('stage3-unsettypes', ['stage3', 'unsettypes'],
     r".budget. types is unset.*bridge_create refused.*alive", None),
    ('stage3-unsetsystems', ['stage3', 'unsetsystems'],
     r".budget. systems is unset.*bridge_create refused.*alive", None),
    ('stage3-notypes', ['stage3', 'notypes'],
     r"module 'game:mover@0.1.0' did not load: type 'game:Marked' is one more than .budget. types of 1 .*not loaded.*alive", None),
    ('stage3-nosystems', ['stage3', 'nosystems'],
     r"module 'game:mover@0.1.0' did not load: it registers one more than .budget. systems of 1 .*not loaded.*alive", None),
    ('stage3-loglevel', ['stage3', 'loglevel'],
     r"module 'game:mover@0.1.0' is refused: its log_level of 8 is not one it can be, which is 1 to 7.*not loaded.*alive", None),
    ('stage3-quiet', ['stage3', 'quiet'],
     r"game:mover@0.1.0: loaded\s+alive", r"moved"),
    ('stage3-unsetterms', ['stage3', 'unsetterms'],
     r".budget. terms is unset.*bridge_create refused.*alive", None),
    ('stage3-noterms', ['stage3', 'noterms'],
     r"module 'view:reader@0.1.0' did not load: a system asks for 2 terms, which is more than .budget. terms of 1 .*not loaded.*alive", None),
    ('stage3-nobudget', ['stage3', 'nobudget'],
     r".budget. columns is unset.*bridge_create refused.*alive", None),
    ('stage3-beyond', ['stage3', 'beyond'],
     r".budget. entities of 1099511627776 is more than an offset of 32 bits can reach.*bridge_create refused.*alive", None),
    ('stage3-toobig', ['stage3', 'toobig'],
     r".budget. cannot be satisfied: the shared memory would be [0-9]+ bytes and an offset into it is 32 bits.*bridge_create refused.*alive", None),
    ('stage3-bigprivate', ['stage3', 'bigprivate'],
     r"module 'game:mover@0.1.0' is refused: its scratch of 4294967040, log_ring of [0-9]+ and spans of 4096 bytes come to more than an offset of 32 bits can reach.*alive", None),
    ('stage3-bench', ['stage3', 'bench'],
     r"rows-unchecked  no limit  [0-9.]+ ns a row.*alive", None),
    ('stage4-raw', ['stage4', 'raw'],
     r"game:mover@0.1.0/tally: native tally 2 1000.*native tally 2 1000.*alive", None),
    ('stage4-mixed', ['stage4', 'mixed'],
     r"view:reader@0.1.0/sum: sum 250500 0.*native tally 2 1000.*sum 251000 0.*alive", None),
    ('stage4-untrusted', ['stage4', 'untrusted'],
     r"module 'game:mover@0.1.0' is refused: it is not trusted, and backend 'native' runs only modules that are.*alive", r"cannot open"),
    ('stage4-limit', ['stage4', 'limit'],
     r"module 'game:mover@0.1.0' is refused: it states work_limit, which backend 'native' cannot keep.*alive", r"cannot open"),
    ('stage4-libraries', ['stage4', 'libraries'],
     r"the wasm backend's library is still loaded, the native backend's is gone.*alive", None),
    ('stage4-backendabi', ['stage4', 'backendabi'],
     r"backend 'native': backend library '[^']*bridge-native-other[^']*' is of backend ABI 2, and this is 1.*bridge_create refused.*alive", None),
    ('stage4-nolib', ['stage4', 'nolib'],
     r"module 'game:mover@0.1.0' did not load: cannot open the library 'no-such-library'.*alive", None),
    ('stage4-refuse', ['stage4', 'refuse'],
     r"test:flood@0.1.0/flood: system 'flood' failed: spawn: its spawns of 1024 between two barriers are used up.*alive", None),
    ('stage5-order', ['stage5', 'order'],
     r"sum 125250 0.*alive", None),
    ('stage5-reorder', ['stage5', 'reorder'],
     r"sum 125500 0.*alive", None),
    ('stage5-ambiguous', ['stage5', 'ambiguous'],
     r"module 'view:halver@0.1.0' did not load: 'game:mover:move' and 'view:halver:halve' both touch 'game:Position' in phase 'update', one of them writing it, and nothing orders them.*alive", None),
    ('stage5-ownorder', ['stage5', 'ownorder'],
     r"game:twostep@0.1.0/show: x 3 0.*alive", None),
    ('stage5-cycle', ['stage5', 'cycle'],
     r".order. contradicts itself.*bridge_create refused.*alive", None),
    ('stage5-observe', ['stage5', 'observe'],
     r"b2 game:setter@0.1.0/mark: marked.*b2 view:watcher@0.1.0/seen: seen 1 77.*alive", None),
    ('stage5-chain', ['stage5', 'chain'],
     r"b1 game:chain@0.1.0/got_a: hop A.*b2 game:chain@0.1.0/got_b: hop B.*b3 game:chain@0.1.0/got_c: hop C.*alive", r"hop A.*hop A"),
    ('stage5-reserved', ['stage5', 'reserved'],
     r"'init' is a reserved name, and no observer may take it.*alive", None),
    ('stage5-parallel', ['stage5', 'parallel'],
     r"on 2 thread.s.[^a-z]*they ran at once.*alive", None),
    ('stage5-serial', ['stage5', 'serial'],
     r"on 1 thread.s.[^a-z]*they ran one after the other.*alive", None),
    ('stage5-foreign', ['stage5', 'foreign'],
     r"test:foreign@0.1.0/reach: an id from another thread, and from this one 0 1.*alive", None),
    ('stage5-pinned', ['stage5', 'pinned'],
     r"the others used [2-4] thread.s.[^a-z]*pinned: 9 of its 9 calls were on the thread that loaded it.*alive", None),
    ('stage5-wrongthread', ['stage5', 'wrongthread'],
     r"module 'test:pinned@0.1.0' is pinned to the thread that loaded it, and this call comes from another.*from another thread: refused.*bridge_destroy is refused: module 'test:pinned@0.1.0' is pinned.*destroy from another thread: refused.*destroy from the thread that loaded it: done.*alive", None),
    ('stage5-nothreads', ['stage5', 'nothreads'],
     r"threads is unset.*bridge_create refused.*alive", None),
    ('stage5-hop', ['stage5', 'hop'],
     r"sum 275000 0.*alive", None),
    ('stage5-events', ['stage5', 'events'],
     r"any: event 3 1.*any: event 2 2.*any: event 4 57005.*alive", None),
    ('stage5-obsconflict', ['stage5', 'obsconflict'],
     r"module 'view:fixer@0.1.0' did not load: 'view:watcher:seen' and 'view:fixer:fix' both touch 'game:Marked' at the barrier, one of them writing it, and nothing orders them.*seen 1 77.*alive", None),
    ('stage5-fewchanges', ['stage5', 'fewchanges'],
     r"module 'game:stamp5@0.1.0' did not load: its changes of 16 bytes are used up.*not loaded.*alive", None),
    ('stage5-nochanges', ['stage5', 'nochanges'],
     r"module 'game:stamp5@0.1.0' did not load: it stated that it queues no change.*not loaded.*alive", None),
    ('stage5-phaseunmapped', ['stage5', 'phaseunmapped'],
     r"a system names phase 'tick', which the deployment did not declare.*not loaded.*alive", 'seen'),
    ('stage5-phasemapped', ['stage5', 'phasemapped'],
     r"game:stamp@0.1.0: loaded.*seen 1 7.*alive", None),
    ('stage5-phasebadmap', ['stage5', 'phasebadmap'],
     r"module 'game:stamp@0.1.0' is refused: its config maps phase 'tick' to 'nowhere', which the deployment did not declare.*not loaded.*alive", 'seen'),
    ('stage5-unstated', ['stage5', 'unstated'],
     r"system 'stamp' failed: system 'stamp' may not attach 'game:Marked': it is not among the changes it stated.*alive", 'seen'),
    ('stage5-changeconflict', ['stage5', 'changeconflict'],
     r"module 'game:stamp9@0.1.0' did not load: 'game:stamp5:stamp' and 'game:stamp9:stamp' both attach or detach 'game:Marked' in phase 'update', and nothing orders them.*seen 1 5.*alive", 'seen 1 9'),
    ('stage5-changeorder', ['stage5', 'changeorder'],
     r"seen 1 9.*alive", 'seen 1 5'),
    ('stage5-changeloaded', ['stage5', 'changeloaded'],
     r"seen 1 9.*alive", 'seen 1 5'),
    ('stage5-reapread', ['stage5', 'reapread'],
     r"reap: rows 1 0.*module 'view:reaper@0.1.0' asked to destroy entities that carry nothing it may write. Left as they were: 1.*reap: rows 1 0.*alive", 'rows 0 0'),
    ('stage5-reapwrite', ['stage5', 'reapwrite'],
     r"reap: rows 1 0.*reap: rows 0 0.*alive", 'Left as they were'),
    ('stage5-bothtold', ['stage5', 'bothtold'],
     r"module 'game:stamp5@0.1.0': .budget. tables of 1 is used up. Not applied, of the changes it asked: 2.*module 'game:stamp9@0.1.0': .budget. tables of 1 is used up. Not applied, of the changes it asked: 2.*alive", 'seen'),
    ('stage5-obsorder', ['stage5', 'obsorder'],
     r"view:fixer@0.1.0: loaded.*seen 1 77.*alive", None),
    ('stage5-obsreorder', ['stage5', 'obsreorder'],
     r"view:fixer@0.1.0: loaded.*seen 1 5.*alive", None),
    ('stage5-nested', ['stage5', 'nested'],
     r"read: body 2 2.*read: more 3 4.*alive", None),
    ('stage5-nested2', ['stage5', 'nested2'],
     r"module 'game:nested@0.1.0' did not load: type 'game:Body': its descriptor says 'Vec2', which is neither a type a field can be nor one its namespace has declared.*alive", None),
    ('stage4-setting', ['stage4', 'setting'],
     r"backend 'native' would not start: it knows no setting 'cache'.*bridge_create refused.*alive", None),
    ('stage5-two', ['stage5', 'two'],
     r"two bridges: sums 275000 and 265000.*alive", None),
    ('stage3-strings', ['stage3', 'strings'],
     r"game:namer@0.1.0/say: kept in the arena.*alive", None),
    ('stage3-noarena', ['stage3', 'noarena'],
     r"module 'game:namer@0.1.0' did not load: alloc: its persistent of 4 bytes is exhausted.*alive", None),
    ('stage3-bigarena', ['stage3', 'bigarena'],
     r"module 'game:namer@0.1.0' is refused: its persistent of 1048576 bytes does not fit in the 4096 left of .budget. persistent.*alive", None),
    ('stage3-nospawns', ['stage3', 'nospawns'],
     r"module 'game:mover@0.1.0' did not load: spawn: it stated that it spawns nothing.*alive", None),
    ('stage3-undo', ['stage3', 'undo'],
     r"game:badterm@0.1.0: not loaded.*game:fp@0.1.0: loaded.*alive", None),
    ('stage5-minted', ['stage5', 'minted'],
     r"minted 160 ids a run. on four threads the same, and again the same.*alive", None),
    ('stage6-skipinit', ['stage6', 'skipinit'],
     r"game:badobs@0.1.0: not loaded.*show: x 3 0.*modules: game:twostep@0.1.0=RUNNING .1..*alive", None),
    ('stage6-refusedload', ['stage6', 'refusedload'],
     r"game:badterm@0.1.0: not loaded.*game:fp@0.1.0: loaded.*alive", None),
    ('stage6-budget', ['stage6', 'budget'],
     r"module 'test:flaky@0.1.0' is stopped: its system 'trip' reached its max_system_failures of 3.*show: x 63 0.*game:twostep@0.1.0=RUNNING test:flaky@0.1.0=STOPPED.*alive", None),
    ('stage6-forgiven', ['stage6', 'forgiven'],
     r"modules: test:flaky@0.1.0=RUNNING.*alive", None),
    ('stage6-notforgiven', ['stage6', 'notforgiven'],
     r"its system 'trip' reached its max_system_failures of 3.*modules: test:flaky@0.1.0=STOPPED.*alive", None),
    ('stage6-notmasked', ['stage6', 'notmasked'],
     r"its system 'trip' reached its max_system_failures of 3.*modules: test:flaky@0.1.0=STOPPED.*alive", None),
    ('stage6-neverstop', ['stage6', 'neverstop'],
     r"modules: test:flaky@0.1.0=RUNNING.*alive", None),
    ('stage6-failstop', ['stage6', 'failstop'],
     r"the bridge has stopped: module 'test:flaky@0.1.0' was stopped and its on_error is fail.*update refused.*test:flaky@0.1.0=STOPPED.*game:twostep@0.1.0: not unloaded.*alive", None),
    ('stage6-failgiveup', ['stage6', 'failgiveup'],
     r"the bridge has stopped.*game:twostep@0.1.0: not unloaded.*the bridge runs again: module 'test:flaky@0.1.0' is unloaded, and it goes on without it.*test:flaky@0.1.0: unloaded.*modules: game:twostep@0.1.0=RUNNING.*alive", None),
    ('stage6-strip', ['stage6', 'strip'],
     r"left 10 5.*game:keeper@0.1.0: unloaded.*left 9 5.*game:retype@0.1.0: loaded.*alive", None),
    ('stage6-sweep', ['stage6', 'sweep'],
     r"left 10 5.*game:keeper@0.1.0: unloaded.*left 9 0.*game:retype@0.1.0: loaded.*alive", None),
    ('stage6-delete', ['stage6', 'delete'],
     r"left 10 5.*game:keeper@0.1.0: unloaded.*left 0 0.*game:retype@0.1.0: loaded.*alive", None),
    ('stage6-nested-unload', ['stage6', 'nested-unload'],
     r"game:typer@0.1.0: unloaded.*read: body 2 3.*game:retype@0.1.0: not loaded.*game:user@0.1.0: unloaded.*game:retype@0.1.0: loaded.*alive", None),
    ('stage6-nounload', ['stage6', 'nounload'],
     r"no module 'game:nobody@0.1.0' is loaded, so none is unloaded.*alive", None),
    ('stage6-timeouts', ['stage6', 'timeouts'],
     r"its shutdown trapped: interrupt. its teardown continues.*test:hang@0.1.0: unloaded.*modules: .0..*alive", None),
    ('stage6-slots', ['stage6', 'slots'],
     r"module 'game:ticker@0.1.0' is refused: .budget. modules of 1 is reached.*game:twostep@0.1.0: unloaded.*game:ticker@0.1.0: loaded.*modules: game:ticker@0.1.0=RUNNING .1..*alive", None),
    ('stage6-reload', ['stage6', 'reload'],
     r"tick 102 1.*game:ticker@0.1.0/-: shutdown 1 0.*game:ticker@0.2.0: reloaded.*tick 112 10.*tick 122 10.*alive", None),
    ('stage6-reloadexact', ['stage6', 'reloadexact'],
     r"tick 102 1.*game:ticker@0.2.0: reloaded.*tick 112 10.*tick 122 10.*alive", 'systems of 1'),
    ('stage6-reloadexactfail', ['stage6', 'reloadexactfail'],
     r"was not reloaded: its reload answered 5.*tick 103 1.*tick 104 1.*game:ticker@0.1.0=RUNNING.*alive", 'systems of 1'),
    ('stage6-reloadspawns', ['stage6', 'reloadspawns'],
     r"tick 102 1.*game:ticker@0.2.0: reloaded.*tick 112 10.*tick 122 10.*alive", None),
    ('stage6-reloadlayout', ['stage6', 'reloadlayout'],
     r"was not reloaded: type 'game:Tick' has another component layout here: fingerprint [0-9a-f]+, and it was declared with [0-9a-f]+.*tick 103 1.*tick 104 1.*alive", None),
    ('stage6-reloadfail', ['stage6', 'reloadfail'],
     r"was not reloaded: its reload answered 5.*tick 103 1.*tick 104 1.*game:ticker@0.1.0=RUNNING.*alive", None),
    ('stage6-sharegrow', ['stage6', 'sharegrow'],
     r"game:ticker@0.2.0: reloaded.*tick 112 10.*tick 122 10.*alive", None),
    ('stage6-sharechain', ['stage6', 'sharechain'],
     r"game:namer@0.1.0: loaded.*game:ticker@0.2.0: reloaded.*tick 112 10.*alive", None),
    ('stage6-sharesame', ['stage6', 'sharesame'],
     r"was not reloaded: alloc: its persistent of 16 bytes is exhausted.*tick 103 1.*tick 104 1.*alive", None),
    ('stage6-shareless', ['stage6', 'shareless'],
     r"is not reloaded: its persistent is 16 and a reload states less, which a loaded module's share cannot become.*tick 104 1.*alive", None),
    ('stage6-sharebig', ['stage6', 'sharebig'],
     r"is not reloaded: its persistent of 1048576 bytes does not fit in the [0-9]+ left of .budget. persistent.*tick 104 1.*alive", None),
    ('stage6-repair', ['stage6', 'repair'],
     r"the bridge has stopped: module 'game:ticker@0.1.0' was stopped and its on_error is fail.*game:twostep@0.1.0: not loaded.*the bridge runs again: module 'game:ticker@0.2.0' is back.*game:ticker@0.2.0: reloaded.*tick 1[0-9]3 10.*game:ticker@0.2.0=RUNNING.*alive", None),
    ('stage6-repairload', ['stage6', 'repairload'],
     r"the bridge has stopped: module 'game:thing@0.1.0' failed and its on_error is fail.*game:other@0.1.0: not loaded.*the bridge runs again: module 'game:thing@0.2.0' is back.*game:thing@0.2.0: loaded.*show: x 3 0.*alive", None),
    ('stage6-reloadmove', ['stage6', 'reloadmove'],
     r"it runs on backend 'wasm' and a reload does not move it to 'native'.*tick 104 1.*alive", None),
    ('stage6-reloaddrop', ['stage6', 'reloaddrop'],
     r"game:ticker@0.2.0: reloaded.*tick 122 10.*game:retype@0.1.0: loaded.*alive", None),
    ('stage6-maynot', ['stage6', 'maynot'],
     r"is not reloaded: its hot_reload is false.*tick 104 1.*alive", None),
    ('stage6-revive', ['stage6', 'revive'],
     r"game:ticker@0.1.0=STOPPED.*game:ticker@0.2.0: reloaded.*tick 1[0-9]3 10.*game:ticker@0.2.0=RUNNING.*alive", None),
    ('stage6-noreload', ['stage6', 'noreload'],
     r"module 'test:noreload@0.1.0' did not load: no exported function 'reload'.*alive", None),
    ('stage6-notreloadable', ['stage6', 'notreloadable'],
     r"module 'game:twostep@0.1.0' is refused: hot_reload is stated, and its author states it is not reloadable.*alive", None),
    ('stage6-reloadnothing', ['stage6', 'reloadnothing'],
     r"is not reloaded: nothing of that name is loaded.*alive", None),
    ('stage6-nreload', ['stage6', 'nreload'],
     r"tick 102 1.*game:ticker@0.1.0/-: shutdown 1 0.*game:ticker@0.2.0: reloaded.*tick 112 10.*tick 122 10.*alive", None),
    ('stage6-nreloadfail', ['stage6', 'nreloadfail'],
     r"shutdown 1 0.*was not reloaded: its reload answered 5.*module 'game:ticker@0.1.0' is stopped: its reload failed after its old instance had been shut down. What it declared is kept.*game:ticker@0.1.0=STOPPED.*game:ticker@0.3.0: reloaded.*tick 112 10.*tick 122 10.*game:ticker@0.3.0=RUNNING.*alive", None),
    ('stage6-nreloadfailunload', ['stage6', 'nreloadfailunload'],
     r"game:ticker@0.1.0=STOPPED.*game:ticker@0.1.0: unloaded.*modules: game:twostep@0.1.0=RUNNING .1..*alive", None),
    ('stage6-config', ['stage6', 'config'],
     r"vsync width 1 1280.*scale mode 150 2.*show: hello 7 0.*show: some/long/path.cfg 0 0.*flags ratio 3 3.*show: big 2 0.*alive", None),
    ('stage6-configkey', ['stage6', 'configkey'],
     r"its settings have a key 'vsynk', and 'game:Settings' has no field of that name.*alive", None),
    ('stage6-configkind', ['stage6', 'configkind'],
     r"its setting 'scale' is text, and the field it names is a float.*alive", None),
    ('stage6-configlong', ['stage6', 'configlong'],
     r"its setting 'title' is 20 bytes of text, and the field it names holds 16 with the zero that ends it.*alive", None),
    ('stage6-configrange', ['stage6', 'configrange'],
     r"its setting 'small' is 300, which the field it names cannot hold.*alive", None),
    ('stage6-configwide', ['stage6', 'configwide'],
     r"its setting 'mode' is 200, which the enum of the field it names cannot hold.*alive", None),
    ('stage6-configenum', ['stage6', 'configenum'],
     r"its setting 'mode' is 'Sometimes', which the enum of the field it names has no constant for.*alive", None),
    ('stage6-confignone', ['stage6', 'confignone'],
     r"it is handed 10 settings, and states that it has no config component.*alive", None),
    ('stage6-configunset', ['stage6', 'configunset'],
     r"module 'game:settings@0.1.0' is refused: config is unset.*alive", None),
    ('stage6-configmissing', ['stage6', 'configmissing'],
     r"its config is 'game:Nope', which is not a struct its init declared.*alive", None),
    ('stage6-configarena', ['stage6', 'configarena'],
     r"its settings hold 24 bytes of text, and its persistent share has 8 left.*alive", None),
    ('stage6-configtwice', ['stage6', 'configtwice'],
     r"its config is 'game:Settings', which is already the settings of module 'game:settings@0.1.0'.*alive", None),
    ('stage6-configreload', ['stage6', 'configreload'],
     r"vsync width 1 1280.*game:settings@0.2.0: reloaded.*vsync width 1 1280.*alive", None),
    ('stage6-configdrop', ['stage6', 'configdrop'],
     r"game:settings@0.2.0: reloaded.*game:retype@0.1.0: loaded.*alive", None),
    ('stage6-reseed', ['stage6', 'reseed'],
     r"settings text written to the same place.*alive", None),
    ('stage6-nreloadfailstop', ['stage6', 'nreloadfailstop'],
     r"the bridge has stopped: module 'game:ticker@0.1.0' could not be reloaded and its on_error is fail.*update refused.*the bridge runs again: module 'game:ticker@0.3.0' is back.*tick 112 10.*game:ticker@0.3.0=RUNNING.*alive", None),
    ('stage6-ntimeout', ['stage6', 'ntimeout'],
     r"module 'game:ticker@0.1.0' is refused: it states shutdown_timeout, which backend 'native' cannot keep.*alive", r"cannot open"),
    ('stage6-confignoroom', ['stage6', 'confignoroom'],
     r"did not load: type 'game:Settings' is a singleton, and .budget. columns of 16 bytes has no room "
     r"for its \d+ bytes.*alive", None),
    ('stage6-configtable', ['stage6', 'configtable'],
     r"did not load: its config is 'game:Settings', which is not declared a singleton.*alive", None),
    ('stage4-nopath', ['stage4', 'nopath'],
     r"module 'game:mover@0.1.0' did not load: this backend opens a module's library by its path, and was handed none.*alive", None),
    ('stage4-nobytes', ['stage4', 'nobytes'],
     r"module 'view:reader@0.1.0' did not load: this backend compiles a module from its bytes, and was handed none.*alive", None),
    ('stage6-churn', ['stage6', 'churn'],
     r"churned 200 times, 0 loads refused, 0 records of the index out of step.*alive", None),
    ('stage6-arenaback', ['stage6', 'arenaback'],
     r"game:namer@0.1.0/say: kept in the arena.*game:namer@0.1.0: unloaded.*game:ticker@0.1.0: loaded.*alive", None),
    ('stage6-arenakept', ['stage6', 'arenakept'],
     r"module 'game:namer@0.1.0' is unloaded and its share of the persistent arena, 64 bytes, stays: 'game:Name' holds a string in it, in this many rows: 1.*game:namer@0.1.0: unloaded.*game:reader@0.1.0/say: kept in the arena.*module 'game:ticker@0.1.0' is refused: its persistent of 64 bytes does not fit in the 0 left of .budget. persistent.*game:reader@0.1.0/say: kept in the arena.*alive", None),
    ('stage6-destroy', ['stage6', 'destroy'],
     r"tick 101 1.*game:ticker@0.1.0/-: shutdown 1 0.*module 'test:hang@0.1.0': its shutdown trapped: interrupt. its teardown continues.*alive", None),
]

# every case of the first host again, through the static library
CASES += [('static-' + name[len('stage2-'):], ['stage2-static'] + argv[1:], match, never)
          for name, argv, match, never in CASES if argv[0] == 'stage2']


if __name__ == '__main__':
    main()
