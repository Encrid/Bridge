"""Bridge's build: one compiler, one script. docs/design/build.md.

    python build.py                      build, for this machine
    python build.py test [-k NAME]       build, then run the tests (all, or those matching)
    python build.py test --copies 6      each case six times at once: under load
    python build.py test --rounds 10     the tests ten times over
    python build.py --target T ...       one of TARGETS, below: three systems, two processors
    python build.py --debug ...          unoptimised, with debug information
    python build.py package              build, run every case, and make what a host is handed

Needs zig on the path. Every path is taken from where this file is and from --out, so a repository that has this one as a submodule builds it by calling this.
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
from pathlib import Path

ROOT = Path(__file__).resolve().parent
# What differs by target, and nothing else does: where it runs, how a program and a library are
# named, which folder of utils/os is its operating system's, which of the system's declarations are asked for by name (the language is ISO C, and a
# compiler's gnu mode would have shown them unasked), what a link names, and how a library finds what
# sits beside it.
TARGETS = {
    'x86_64-windows-gnu': {
        'runs on': ('Windows', 'AMD64'), 'program': '.exe', 'library': '.dll', 'os': 'windows',
        'compile': [], 'link': [], 'beside': [], 'named': []},
    'x86_64-linux-gnu': {
        'runs on': ('Linux', 'x86_64'), 'program': '', 'library': '.so', 'os': 'posix',
        'compile': ['-fPIC', '-D_DEFAULT_SOURCE'], 'link': ['-lm', '-lpthread', '-ldl'],
        'beside': ['-Wl,-rpath,$ORIGIN'], 'named': ['-Wl,-soname,bridge.so']},
    'aarch64-macos': {
        'runs on': ('Darwin', 'arm64'), 'program': '', 'library': '.dylib', 'os': 'posix',
        'compile': ['-fPIC', '-D_DARWIN_C_SOURCE'], 'link': [],
        'beside': ['-Wl,-rpath,@loader_path'], 'named': ['-Wl,-install_name,@rpath/bridge.dylib']},
    'aarch64-linux-gnu': {
        'runs on': ('Linux', 'aarch64'), 'program': '', 'library': '.so', 'os': 'posix',
        'compile': ['-fPIC', '-D_DEFAULT_SOURCE'], 'link': ['-lm', '-lpthread', '-ldl'],
        'beside': ['-Wl,-rpath,$ORIGIN'], 'named': ['-Wl,-soname,bridge.so']},
    'x86_64-macos': {
        'runs on': ('Darwin', 'x86_64'), 'program': '', 'library': '.dylib', 'os': 'posix',
        'compile': ['-fPIC', '-D_DARWIN_C_SOURCE'], 'link': [],
        'beside': ['-Wl,-rpath,@loader_path'], 'named': ['-Wl,-install_name,@rpath/bridge.dylib']},
    'aarch64-windows-gnu': {
        'runs on': ('Windows', 'ARM64'), 'program': '.exe', 'library': '.dll', 'os': 'windows',
        'compile': [], 'link': [], 'beside': [], 'named': []},
}
TIMEOUT = 120

OS = ['memory', 'library', 'thread', 'mutex', 'event']
SCHEDULE = ['schedule_matrix', 'schedule_order', 'schedule_conflict', 'schedule_edges',
            'schedule_check', 'schedule_build', 'schedule_dispatch', 'schedule_jobs', 'schedule_run',
            'schedule_account']
BLOCK = ['region', 'region_take', 'block_column', 'block_module', 'block_log', 'block_barrier',
         'block_budget', 'block_layout', 'block_create', 'block_destroy']
STORAGE = ['storage_apply', 'storage_cells', 'storage_spans', 'storage_publish', 'storage_drop',
           'storage_drop_apart', 'storage_drop_table', 'storage_slots', 'storage_declare',
           'storage_terms', 'storage_ids', 'storage_carriers', 'storage_any', 'storage_refold',
           'storage_convert', 'storage_made', 'storage_create']
CONFIG = ['config_say', 'config_scalar', 'config_enum', 'config_text', 'config_field',
          'config_room', 'config_kept']
SCHEMA = ['schema_reader', 'schema_token', 'schema_value', 'schema_type', 'schema_struct',
          'schema_constants']
MODULE = ['module_say', 'module_check', 'module_check_stated', 'module_check_fit',
          'module_check_backend', 'module_record', 'module_discard', 'module_types', 'module_call',
          'module_instance', 'module_start', 'module_pointed', 'module_share', 'reload_check', 'reload_share',
          'reload_record', 'reload_rows', 'reload_fail']
BRIDGE = ['say', 'bridge_create', 'bridge_backend', 'bridge_backends', 'bridge_phases',
          'bridge_destroy', 'bridge_stop', 'bridge_run', 'bridge_load', 'bridge_unload',
          'bridge_reload', 'config',
          'schema', 'abi_calls', 'abi_alloc', 'abi_define', 'abi_names', 'abi_stated', 'abi_relay_run',
          'abi_relay_fields', 'abi_relay', 'abi_moved', 'abi_replace', 'abi_layout', 'abi_make',
          'abi_declare', 'abi_subscriber', 'abi_subscriber_room', 'abi_term', 'abi_subscribe',
          'abi_system'] + MODULE + ['pool', 'pool_job', 'pool_run', 'backend'] + SCHEMA + CONFIG + SCHEDULE + BLOCK + STORAGE
# the tests' backend, and the modules it runs: the name of a module's library, its source under
# tests/modules, what it is compiled with - stage22.md, *What a test module is written with*
TESTS_BACKEND = ['tests_abi', 'tests_backend', 'tests_memory', 'tests_define', 'tests_module', 'tests_instance', 'tests_func',
                 'tests_load', 'tests_phase', 'tests_moves']
BUILT = [
    ('ok', 'ok', []), ('image', 'image', []), ('oob', 'oob', []), ('spin', 'spin', []),
    ('grow', 'grow', []), ('noinit', 'noinit', []), ('noreload', 'noreload', []),
    ('stray', 'stray', []), ('tmover', 'tmover', []), ('tflood', 'tflood', []),
    ('tforeign', 'tforeign', []),
    ('sleep1', 'sleep', []), ('sleep2', 'sleep', []), ('pinned', 'pinned', []),
    ('setter', 'setter', []), ('reaper', 'reaper', []),
    ('stamp5', 'stamp', ['VALUE=5', 'PHASE="update"']),
    ('stamp9', 'stamp', ['VALUE=9', 'PHASE="update"']),
    ('stampunstated', 'stamp', ['VALUE=1', 'PHASE="update"', 'UNSTATED']),
    ('stamptick', 'stamp', ['VALUE=7', 'PHASE="tick"']),
    ('stepper1', 'stepper', ['STEP=1']), ('stepper2', 'stepper', ['STEP=10']),
    ('stepper3', 'stepper', ['STEP=10', 'FAIL_RELOAD']),
]
# what tests/storage loads
BUILT += [
    ('store', 'store', []),
    ('storebig', 'store', ['BIG']),
    ('storetab', 'store', ['TABLES']),
    ('storefat64', 'store', ['BIG', 'FAT64']),
    ('storefat256', 'store', ['BIG', 'FAT256']),
    ('storefat1k', 'store', ['BIG', 'FAT1K']),
    ('storefat4k', 'store', ['BIG', 'FAT4K']),
    ('frag1', 'frag', ['SPLIT=1']),
    ('frag16', 'frag', ['SPLIT=16']),
    ('frag256', 'frag', ['SPLIT=256']),
    ('frag4096', 'frag', ['SPLIT=4096']),
    ('sparse', 'sparse', []),
    ('sparsebig', 'sparse', ['BIG']),
    ('sparsefat', 'sparse', ['FAT']),
    ('sparsebad1', 'sparse', ['BAD=1']),
    ('sparsebad2', 'sparse', ['BAD=2']),
    ('move1', 'move', ['KEPT=1']),
    ('move2', 'move', ['KEPT=2']),
    ('move3', 'move', ['KEPT=3']),
    ('move4', 'move', ['KEPT=4']),
    ('move5', 'move', ['KEPT=5']),
    ('holder', 'holder', []),
    ('layout0', 'layout', ['V=0', 'KEPT=1']),
    ('layout1', 'layout', ['V=1', 'KEPT=1']),
    ('layout2', 'layout', ['V=2', 'KEPT=1']),
    ('layout3', 'layout', ['V=3', 'KEPT=1']),
    ('layout4', 'layout', ['V=4', 'KEPT=1']),
    ('layout5', 'layout', ['V=5', 'KEPT=1']),
    ('layout6', 'layout', ['V=6', 'KEPT=1']),
    ('layout7', 'layout', ['V=7', 'KEPT=1']),
    ('layout8', 'layout', ['V=8', 'KEPT=1']),
    ('layout9', 'layout', ['V=9', 'KEPT=1']),
    ('layout10', 'layout', ['V=10', 'KEPT=1']),
    ('layout11', 'layout', ['V=11', 'KEPT=1']),
    ('layout12', 'layout', ['V=12', 'KEPT=1']),
    ('layout13', 'layout', ['V=13', 'KEPT=1']),
    ('layout14', 'layout', ['V=14', 'KEPT=1']),
    ('layout0k2', 'layout', ['V=0', 'KEPT=2']),
    ('layout1k2', 'layout', ['V=1', 'KEPT=2']),
    ('layout2k2', 'layout', ['V=2', 'KEPT=2']),
    ('layout0k3', 'layout', ['V=0', 'KEPT=3']),
    ('layout1k3', 'layout', ['V=1', 'KEPT=3']),
    ('layout2k3', 'layout', ['V=2', 'KEPT=3']),
    ('layout0k4', 'layout', ['V=0', 'KEPT=4']),
    ('layout1k4', 'layout', ['V=1', 'KEPT=4']),
    ('layout2k4', 'layout', ['V=2', 'KEPT=4']),
    ('layout0k5', 'layout', ['V=0', 'KEPT=5']),
    ('layout1k5', 'layout', ['V=1', 'KEPT=5']),
    ('layout2k5', 'layout', ['V=2', 'KEPT=5']),
    ('tag', 'tag', []),
    ('fragtag4096', 'frag', ['SPLIT=4096', 'TAGS']),
    ('single', 'single', []),
    ('singlebad1', 'single', ['BAD=1']),
    ('singlebad2', 'single', ['BAD=2']),
    ('singlebad3', 'single', ['BAD=3']),
    ('direct', 'direct', []),
    ('directbig', 'direct', ['BIG']),
    ('directbad', 'direct', ['BAD']),
    ('toggle', 'toggle', []),
    ('toggle1', 'toggle', ['BIG', 'ON=1']),
    ('toggle2', 'toggle', ['BIG', 'ON=2']),
    ('toggle16', 'toggle', ['BIG', 'ON=16']),
    ('weigh1', 'weigh', ['ON=1']),
    ('weigh2', 'weigh', ['ON=2']),
    ('weigh16', 'weigh', ['ON=16']),
    ('stress1', 'stress', ['SEED=1u', 'SPACE="s1"']),
    ('stress2', 'stress', ['SEED=2u', 'SPACE="s2"']),
    ('stress3', 'stress', ['SEED=3u', 'SPACE="s3"']),
    ('stress4', 'stress', ['SEED=4u', 'SPACE="s4"']),
]

# a module written by offsets: the name, its source, whether its rows are tested, what it is
# compiled with.
# `checked` or `unchecked` is stated for each, as the SDK asks: BRIDGE_ROWS_CHECKED, or not.
PLAIN = ('mover reader refs fp badterm spin3 flood chatty rows halver twostep watcher chain '
         'badobs events fixer nested namer spawner counter typer user hang hit '
         'shield aim levels tagged')
WASM = [(name, name, 'checked', []) for name in PLAIN.split()] + [
    ('rows-unchecked', 'rows', 'unchecked', []),
    ('nested2', 'nested', 'checked', ['LEAVE_OUT_VEC2']),
    ('settings', 'settings', 'checked', ['KEPT=BRIDGE_SINGLETON']),
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
    ('settings-drop', 'settings', 'checked', ['DROP', 'KEPT=BRIDGE_SINGLETON']),
    ('settings-table', 'settings', 'checked', ['KEPT=BRIDGE_TABLE']),
    ('ticker', 'ticker', 'checked', ['STEP=1']),
    ('ticker-ten', 'ticker', 'checked', ['STEP=10']),
    ('ticker-wide', 'ticker', 'checked', ['STEP=10', 'WIDE']),
    ('ticker-fail', 'ticker', 'checked', ['STEP=10', 'FAIL_RELOAD']),
    ('ticker-noextra', 'ticker', 'checked', ['STEP=10', 'NO_EXTRA']),
    ('ticker-trip', 'ticker', 'checked', ['STEP=1', 'TRIP']),
    ('ticker-alloc', 'ticker', 'checked', ['STEP=10', 'ALLOC']),
]
# the wrong module: each build does one thing an entry refuses, at init or in its system, and
# states that it does nothing in the other
WRONG_AT_INIT = ('declare_outside declare_name_outside declare_no_namespace declare_kind_unset '
                 'declare_tag_with_bytes declare_policy_unset declare_size_other system_outside '
                 'system_phase_outside system_name_outside system_many_changes system_twice '
                 'observer_no_term observe_outside observe_events_unset term_access_unset '
                 'term_access_other term_oper_unset term_slot_undeclared term_write_ungranted '
                 'change_slot_undeclared change_ungranted alloc_stated_none')
WRONG_IN_SYSTEM = ('declare_late system_late observe_late attach_undeclared attach_ungranted '
                   'attach_outside detach_undeclared attach_tag_value')
WASM += [('wrong-' + act.replace('_', '-'), 'wrong', 'checked',
          ['AT_INIT=wrong_' + act, 'IN_SYSTEM=wrong_do_nothing']) for act in WRONG_AT_INIT.split()]
WASM += [('wrong-' + act.replace('_', '-'), 'wrong', 'checked',
          ['AT_INIT=wrong_do_nothing', 'IN_SYSTEM=wrong_' + act]) for act in WRONG_IN_SYSTEM.split()]


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
            # how it ended is said with what it said: a process the system ended says nothing
            raise RuntimeError('%s\n  %s\n  ended with %d (0x%08X)\n%s'
                               % (what, ' '.join(str(c) for c in command), done.returncode,
                                  done.returncode & 0xFFFFFFFF,
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


# What is handed - host_api.md, *What a host is handed*. A package is a folder, and this is what
# each holds: a file of the tree or of the build, and its name in the package where that
# differs. The backends the tests build for themselves are in none.
def packages(out, lib):
    licence = (ROOT / 'license', 'license')
    return {
        'bridge': [(out / ('bridge' + lib), 'bridge' + lib), (out / 'bridge.a', 'bridge.a'),
                   (ROOT / 'include/bridge.h', 'bridge.h'), licence],
        'seams': [(ROOT / 'include' / h, h)
                  for h in ('bridge_backend.h', 'bridge_abi.h', 'bridge_schema.h')] + [licence],
    }


def commit():
    """The commit the tree is at, and whether the tree is that commit and nothing more."""
    def git(*words):
        return subprocess.run(['git'] + list(words), cwd=ROOT, capture_output=True, text=True).stdout.strip()
    changed = git('status', '--porcelain', '--untracked-files=no')
    return git('rev-parse', 'HEAD') + (' with changes that are not committed' if changed else '')


def write_packages(target, out, lib, ran):
    """Each package of a target, as a folder under packages/, with its statement: its name, the
    commit, the target, whether the cases were run and passed on it, and the SHA-256 of each
    file. A file is written only where it differs, so that what is built from a package is
    built again only when the package changed."""
    at = commit()
    stated = packages(out, lib)
    for stray in (out / 'packages').glob('*/') if (out / 'packages').is_dir() else ():
        if stray.name not in stated:
            shutil.rmtree(stray)
    for name, files in stated.items():
        folder = out / 'packages' / name
        folder.mkdir(parents=True, exist_ok=True)
        said = ['package %s' % name, 'commit %s' % at, 'target %s' % target,
                'run %s' % ('yes' if ran else 'no')]
        for source, called in files:
            data = source.read_bytes()
            if not (folder / called).exists() or (folder / called).read_bytes() != data:
                (folder / called).write_bytes(data)
            said.append('%s  %s' % (hashlib.sha256(data).hexdigest(), called))
        for stray in set(p.name for p in folder.iterdir()) - {c for _, c in files} - {'package.txt'}:
            (folder / stray).unlink()
        text = '\n'.join(said) + '\n'
        if not (folder / 'package.txt').exists() or (folder / 'package.txt').read_text() != text:
            (folder / 'package.txt').write_text(text, newline='\n')


def archive_packages(target, out):
    """Each package as one archive, named for the package and the target: what is published."""
    made = []
    for folder in sorted(p for p in (out / 'packages').iterdir() if p.is_dir()):
        path = out / 'packages' / ('%s-%s.tar.gz' % (folder.name, target))
        with tarfile.open(path, 'w:gz') as tar:
            for f in sorted(folder.iterdir()):
                tar.add(f, arcname='%s/%s' % (folder.name, f.name))
        made.append(path)
    return made


def build(target, debug, out):
    row = TARGETS[target]
    exe, lib = row['program'], row['library']
    zig = tool('zig') or die('zig is not on the path')
    obj, backends = out / 'obj', out / 'backends'

    cc = [zig, 'cc', '-target', target] + (['-O0', '-g'] if debug else ['-O2', '-DNDEBUG'])
    cc += row['compile']
    # an error, since zig says a warning once and then answers from its cache without it. Members
    # an initializer leaves out are zero, which C states.
    ours = cc + ['-std=c23', '-pedantic-errors', '-Wall', '-Wextra', '-Werror',
                 '-Wno-missing-field-initializers']
    system, here = row['link'], row['beside']
    include, src = '-I%s' % (ROOT / 'include'), '-I%s' % (ROOT / 'src')
    # the operating system's layer: one source for the library and for each backend, which are
    # separate binaries and each compile it - build.md, *The C library*
    os_dir = ROOT / 'utils/os' / row['os']
    utils = '-I%s' % (ROOT / 'utils/os')
    headers = [Path(__file__)] + [h for d in ('include', 'src', 'tests', 'tests/*', 'tests/modules/*',
                                              'utils/os', 'utils/os/' + row['os'])
                                  for h in ROOT.glob(d + '/*.h')]

    # ---- objects ----
    defines = ['-D%s="%s"' % (key, path.as_posix()) for key, path in
               [('NATIVE_OTHER_PATH', backends / ('bridge_native_other' + lib)),
                ('NATIVE_REFUSING_PATH', backends / ('bridge_native_refusing' + lib)),
                ('HOLLOW_PATH', backends / ('bridge_hollow' + lib)),
                ('TESTS_PATH', backends / ('bridge_tests' + lib)),
                ('TESTS_ALL_PATH', backends / ('bridge_tests_all' + lib)),
                ('BUILT_DIR', out / 'built')] +
               [('%s_PATH' % name.upper(), out / 'built' / (name + lib)) for name, _, _ in BUILT]]
    defines.append('-DBUILT_SUFFIX="%s"' % lib)
    compiles = []

    def compile_(name, source, flags, deps=()):
        compiles.append(('compiling %s' % name, [obj / (name + '.o')],
                         [source] + headers + list(deps),
                         [flags + ['-c', source, '-o', obj / (name + '.o')]]))
        return obj / (name + '.o')

    # the library's own objects: what bridge.h marks is exported, and nothing else is
    inside = ours + ['-DBRIDGE_BUILDING', '-fvisibility=hidden']
    os_o = [compile_('os-' + name, os_dir / (name + '.c'), inside + [utils]) for name in OS]
    bridge_o = [compile_(name, ROOT / 'src' / (name + '.c'), inside + [include, src, utils])
                for name in BRIDGE]
    # a backend is compiled against the backend seam, the operating system's layer and what is
    # its own: nothing of src/
    backend_os_o = [compile_('backend-os-' + name, os_dir / (name + '.c'), ours + [utils]) for name in OS]
    hollow_o = [compile_('backend_hollow', ROOT / 'tests/support/backend_hollow.c', ours + [include])]
    # a host program is a directory of sources, with what the programs share compiled once -
    # stage12.md, *The test programs*. One that has not been cut yet is one source
    support = '-I%s' % (ROOT / 'tests/support')
    host_o = [compile_('support-' + c.stem, c, ours + [include, support] + defines)
              for c in sorted((ROOT / 'tests/support').glob('host*.c')) +
              sorted((ROOT / 'tests/support' / row['os']).glob('host*.c'))]

    def program(name, flags, shared):
        where = ROOT / 'tests' / name
        if not where.is_dir():
            return [compile_(name, ROOT / 'tests' / (name + '.c'), flags)]
        return [compile_(name + '-' + c.stem, c, flags + [support])
                for c in sorted(where.glob('*.c'))] + shared

    host_program_o = program('host', ours + [include] + defines, host_o)
    # these reach inside, to test it: they are compiled against src/ and share nothing
    schema_test_o = program('schema', ours + [include, src], [])
    storage_o = program('storage', ours + [include, src, utils] + defines, [])
    # the library again, with its calls to the allocator sent to the test that counts them
    counting = ['-D%s=counted_%s' % (f, f) for f in ('malloc', 'calloc', 'realloc', 'free')]
    # and what it asks the operating system to reserve, which the same test refuses in turn
    counted_o = [compile_('counted-' + name, ROOT / 'src' / (name + '.c'),
                          ours + [include, src, utils] + counting +
                          ['-Dos_reserve=counted_os_reserve']) for name in BRIDGE] + \
                [compile_('counted-os-' + name, os_dir / (name + '.c'), ours + [utils] + counting)
                 for name in OS]
    counted_test_o = [compile_('storage-counted-' + c.stem, c,
                               ours + [include, src, utils] + defines + ['-DCOUNTED'])
                      for c in sorted((ROOT / 'tests/storage').glob('*.c'))]
    shared = '-I%s' % (ROOT / 'tests/modules')
    # the tests' backend, and the modules it runs: each a library of its own, as a module is
    binding = '-I%s' % (ROOT / 'tests/binding')
    tests_o = [compile_(name, ROOT / 'tests/backend' / (name + '.c'), ours + [include, utils, binding])
               for name in TESTS_BACKEND] + backend_os_o
    # the same backend, with another answer to backend_get_abi in the place of its own
    native_other_o = [o for o in tests_o if o.name != 'tests_abi.o'] + [
        compile_('native_other_abi', ROOT / 'tests/support/native_other_abi.c', ours + [include])]
    # the same backend again, with first entries that adopt no block; and a library that has
    # the seam's number and no other entry
    native_refusing_o = [o for o in tests_o if o.name not in ('tests_backend.o', 'tests_memory.o')] + [
        compile_(name, ROOT / 'tests/support' / (name + '.c'), ours + [include])
        for name in ('native_refusing_backend', 'native_refusing_memory')]
    # a module written by offsets is built with the accessors that face of the binding is, which
    # are compiled once for each way a row is tested
    offsets_o = {kind: [compile_('offsets-%s-%s' % (kind, c.stem), c,
                                 ours + [include, binding, '-fvisibility=hidden',
                                         '-DBRIDGE_ROWS_' + kind.upper()])
                        for c in sorted((ROOT / 'tests/binding').glob('offsets_*.c'))]
                 for kind in ('checked', 'unchecked')}
    by_offsets_o = {name: [compile_('built-%s-%s' % (name, c.stem), c,
                                    ours + [include, binding, shared, '-fvisibility=hidden',
                                            '-DBRIDGE_ROWS_' + kind.upper()] + ['-D' + d for d in flags])
                           for c in sources(ROOT / 'tests/modules', source, 'offsets_idle',
                                            '"offsets_idle.h"')] + offsets_o[kind]
                    for name, source, kind, flags in WASM}
    built_o = {name: [compile_('built-%s-%s' % (name, c.stem), c,
                               ours + [include, binding, shared, '-fvisibility=hidden'] +
                               ['-D' + d for d in flags])
                      for c in sources(ROOT / 'tests/modules', source, 'idle', '"idle.h"')]
               for name, source, flags in BUILT}
    together(compiles)

    # ---- libraries and programs ----
    links = []

    def link(name, objects, flags=(), shared=False):
        target_file = out / (name + (lib if shared else exe))
        links.append(('linking %s' % name, [target_file], list(objects),
                      [cc + (['-shared'] if shared else []) + list(objects) + list(flags) +
                       ['-o', target_file]]))

    # the same backend, saying it is of another backend seam: for the case that refuses it
    link('backends/bridge_native_other', native_other_o, system, shared=True)
    link('backends/bridge_native_refusing', native_refusing_o, system, shared=True)
    link('backends/bridge_hollow', hollow_o, system, shared=True)
    link('backends/bridge_tests', tests_o, system, shared=True)
    # and again under another name: a library is in a process once, and a deck of the tests has
    # the tests' backend twice, told it keeps different things
    link('backends/bridge_tests_all', tests_o, system, shared=True)
    for name, _, _ in BUILT:
        link('built/' + name, built_o[name], shared=True)
    for name, _, _, _ in WASM:
        link('built/' + name, by_offsets_o[name], shared=True)
    link('schema', schema_test_o + [obj / (name + '.o') for name in ['schema'] + SCHEMA], system)
    whole = bridge_o + os_o
    # the library, built: shared and static. What a host links - host_api.md
    shared_lib, static_lib = out / ('bridge' + lib), out / 'bridge.a'
    link('bridge', whole, system + row['named'], shared=True)
    links.append(('archiving bridge.a', [static_lib], whole,
                  [lambda: static_lib.unlink(missing_ok=True), [zig, 'ar', 'rcs', static_lib] + whole]))
    together(links)
    del links[:]
    # a host is compiled against include/ alone and linked with the library as a host would be:
    # the shared one, and one of them the static one as well
    link('host', host_program_o + [shared_lib], system + here)
    link('host-static', host_program_o + [static_lib], system)
    # these three reach inside, to test it, and so are linked with its objects
    link('storage', storage_o + whole, system)
    link('storage-counted', counted_test_o + counted_o, system)
    together(links)

    # ---- what is handed, and a host, a backend and a module built from it alone: nothing below
    # names a path of the library's headers but the packages' - host_api.md, *What a host is
    # handed* ----
    write_packages(target, out, lib, False)
    pk, made = out / 'packages', out / 'packaged'
    stated = [pk / name / 'package.txt' for name in ('bridge', 'seams')]
    seams = '-I%s' % (pk / 'seams')
    where = ['-D%s="%s"' % (key, path.as_posix()) for key, path in
             [('BACKEND', made / ('bridge_tests' + lib)), ('MODULE', made / ('packed' + lib))]]
    # the number of the module seam is a module's to state, and bridge.h does not have it: a
    # host is told it by who built the module, which here is this build, from the seams' package
    where.append('-DMODULE_ABI=%s' % re.search(r'BRIDGE_ABI = (\d+);', (
        pk / 'seams' / 'bridge_abi.h').read_text(encoding='utf-8')).group(1))
    del compiles[:]
    packaged_host_o = compile_('packaged-host', ROOT / 'tests/packaged/host.c',
                               ours + ['-I%s' % (pk / 'bridge')] + where, stated)
    packaged_backend_o = [compile_('packaged-' + name, ROOT / 'tests/backend' / (name + '.c'),
                                   ours + [seams, utils, binding], stated)
                          for name in TESTS_BACKEND] + backend_os_o
    packaged_module_o = compile_('packaged-packed', ROOT / 'tests/modules/packed.c',
                                 ours + [seams, binding, '-fvisibility=hidden'], stated)
    together(compiles)
    beside = made / ('bridge' + lib)
    step('putting the library beside the packaged host', [beside], [pk / 'bridge' / beside.name],
         [lambda: shutil.copyfile(pk / 'bridge' / beside.name, beside)])
    del links[:]
    link('packaged/host', [packaged_host_o, pk / 'bridge' / beside.name], system + here)
    link('packaged/host-static', [packaged_host_o, pk / 'bridge' / 'bridge.a'], system)
    link('packaged/bridge_tests', packaged_backend_o, system, shared=True)
    link('packaged/packed', [packaged_module_o], shared=True)
    together(links)
    return {'python': sys.executable, 'exe': exe}


def sources(where, name, idle, said='_idle.h"'):
    """A module's sources: the one file of its name, or every C file of the directory of its
    name - docs/style.md, O3, allows five functions a file, and a module may export more. A
    module that includes the idle header of its backend states that its shutdown and its reload
    do nothing, and is built with the source that has them."""
    one = where / (name + '.c')
    found = [one] if one.is_file() else sorted((where / name).glob('*.c'))
    stated = any(said in c.read_text(encoding='utf-8') for c in found)
    return found + ([where / (idle + '.c')] if stated else [])


def command_line(out, built, command, kept):
    """What a case runs: a program of the build, or a tool of ours over the tree."""
    if command[0] == 'package':
        return [str(built['python']), str(ROOT / 'tools/package.py'), str(out)]
    if command[0] == 'cla':
        return [str(built['python']), str(ROOT / 'tools/cla.py'), '--decide'] + command[1:]
    if command[0] in ('c23', 'style', 'cover', 'xref', 'licence'):
        return [str(built['python']), str(ROOT / ('tools/%s.py' % command[0]))]
    if command[0] == 'said':
        return [str(built['python']), str(ROOT / 'tools/said.py'), str(kept)]
    if command[0] == 'deps':
        return [str(built['python']), str(ROOT / 'tools/deps.py'), str(out), '--check']
    return [str(out / (command[0] + built['exe']))] + command[1:]


def run(argv, out):
    """One copy of a case: what it said, and how it ended."""
    try:
        done = subprocess.run(argv, cwd=out, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                              timeout=TIMEOUT)
        return done.stdout.decode('utf-8', 'replace'), done.returncode
    except subprocess.TimeoutExpired as late:
        return (late.stdout or b'').decode('utf-8', 'replace'), 'timeout'


def judge(said, code, match, never):
    """Why a copy of a case failed, or None. With a pattern, the pattern decides and the exit
    code does not: several cases are a refusal that ends the program. Without one, the exit
    code does."""
    if code == 'timeout':
        return 'did not end in %d seconds' % TIMEOUT
    if never and re.search(never, said, re.S):
        return 'said what it must not: %s' % never
    if match and match.startswith('@') and \
            said.replace('\r', '') != (ROOT / match[1:]).read_text(encoding='utf-8'):
        return 'did not say what %s holds, word for word' % match[1:]
    if match and not match.startswith('@') and not re.search(match, said, re.S):
        return 'ended with %s and did not say: %s' % (code, match)
    if not match and code:
        return 'ended with %s' % code
    return None


def test(out, built, only, copies):
    """Each case by what it said. With copies, each case is started that many times at once,
    and every copy is judged as a case is: a case is a process, and the copies compete for the
    processors, for memory, and for what a process shares with its neighbours."""
    cases = [c for c in CASES if not only or only in c[0]]
    failed = []
    # what every case said is kept, a file a case: tools/said.py holds each message of the
    # source against it. A run of every case begins with none, so that no case that is gone
    # holds a message
    kept = out / 'said'
    if not only:
        shutil.rmtree(kept, ignore_errors=True)
    kept.mkdir(exist_ok=True)
    for name, command, match, never in cases:
        argv = command_line(out, built, command, kept)
        count = 1 if name in ALONE else copies
        with concurrent.futures.ThreadPoolExecutor(count) as pool:
            ran = list(pool.map(lambda _: run(argv, out), range(count)))
        (kept / (name + '.txt')).write_text(ran[0][0], encoding='utf-8')
        wrong = [(said, why) for said, why in ((s, judge(s, c, match, never)) for s, c in ran) if why]
        if not wrong:
            print('ok    %s' % name)
            continue
        print('FAIL  %s: %s, in %d of %d\n%s' % (name, wrong[0][1], len(wrong), count, wrong[0][0]))
        failed.append(name)
    print('%d of %d passed' % (len(cases) - len(failed), len(cases)))
    if failed:
        die('failed: %s' % ' '.join(failed))
    if not cases:
        die('no test is named like %r' % only)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('command', nargs='?', default='build', choices=['build', 'test', 'package'])
    ap.add_argument('--target', choices=sorted(TARGETS))
    ap.add_argument('--debug', action='store_true')
    ap.add_argument('--out', type=Path, help='where to build: build/<target> beside this file')
    ap.add_argument('-k', metavar='NAME', help='only the tests whose name has this in it')
    ap.add_argument('--copies', type=int, default=1, metavar='N',
                    help='start each case N times at once, and judge every copy')
    ap.add_argument('--rounds', type=int, default=1, metavar='N', help='run the tests N times')
    args = ap.parse_args()
    machine = (platform.system(), platform.machine())
    mine = next((t for t, row in TARGETS.items() if row['runs on'] == machine), None)
    target = args.target or mine or die('no target runs on %s %s: name one' % machine)
    if args.command == 'test' and target != mine:
        die('%s cannot be run here: build it, and test it where it runs' % target)
    out = (args.out or ROOT / 'build' / (target + ('-debug' if args.debug else ''))).resolve()
    built = build(target, args.debug, out)
    if args.command == 'package':
        # the cases are run where the target is this machine's, and the statement of each
        # package says whether they were: host_api.md, *What a host is handed*
        if target == mine:
            test(out, built, None, 1)
        row = TARGETS[target]
        write_packages(target, out, row['library'], target == mine)
        for path in archive_packages(target, out):
            print(path)
    if args.command == 'test':
        for _ in range(args.rounds):
            test(out, built, args.k, args.copies)


# Every case: its name, what is run, what its output must match, what it must not. A phase's
# cases end in "alive": checked by what was said, and by the process still being there to say it.
# A match that begins with @ names a file the output must equal, word for word: a transcript.
CASES = [
    ('c23', ['c23'],
     None, None),
    ('style', ['style'],
     None, None),
    ('deps', ['deps'],
     None, None),
    ('schema', ['schema'],
     r"every descriptor read as it should, and every other refused by its word.*alive", None),
    ('storage-changes', ['storage', 'changes'],
     '@tests/storage_changes.txt', None),
    ('allocation-changes', ['storage-counted', 'changes'],
     '@tests/storage_changes.txt', None),
    ('allocation-tables', ['storage-counted', 'tables'],
     '@tests/storage_tables.txt', None),
    ('allocation-strip', ['storage-counted', 'strip'],
     '@tests/storage_strip.txt', None),
    ('storage-shape', ['storage', 'shape'],
     r"the shared block reserves more than it uses: yes.*a native module's private block reserves what it uses: yes.*the shared block is whole wasm pages: yes.*a native module's private block is not rounded to one: yes.*alive", None),
    ('allocation-each-refused-in-turn', ['storage-counted', 'nomemory'],
     r"more than forty allocations were each answered null in turn.*alive", 'FAIL'),
    ('allocation-each-refused-in-turn-laying-out-again', ['storage-counted', 'nomemoryrelay'],
     r"more than forty allocations were each answered null in turn.*alive", 'FAIL'),
    ('storage-tables', ['storage', 'tables'],
     '@tests/storage_tables.txt', None),
    ('storage-stress', ['storage', 'stress'],
     '@tests/storage_stress.txt', None),
    ('storage-stress1', ['storage', 'stress1'],
     '@tests/storage_stress.txt', None),
    ('storage-notables', ['storage', 'notables'],
     '@tests/storage_notables.txt', None),
    ('storage-nocolumns', ['storage', 'nocolumns'],
     '@tests/storage_nocolumns.txt', None),
    ('storage-scribble', ['storage', 'scribble'],
     '@tests/storage_scribble.txt', None),
    ('storage-strip', ['storage', 'strip'],
     '@tests/storage_strip.txt', None),
    ('storage-stripfull', ['storage', 'stripfull'],
     '@tests/storage_stripfull.txt', None),
    ('storage-weigh', ['storage', 'weigh'],
     r"an entity of 4096 bytes.*in    1 table.*in 4096 tables.*a sparse set.*of 4096 bytes.*a toggle, one row in 16 on.*direct-indexed.*in no order.*as tags.*a singleton read.*a toggled row switched.*alive", r"FAIL"),
    ('storage-toggle', ['storage', 'toggle'], '@tests/storage_toggle.txt', None),
    ('storage-togglefull', ['storage', 'togglefull'], '@tests/storage_togglefull.txt', None),
    ('storage-move', ['storage', 'move'], '@tests/storage_move.txt', None),
    ('storage-layout', ['storage', 'layout'], '@tests/storage_layout.txt', None),
    ('BRIDGE_TAG-carried-with-no-bytes', ['storage', 'tag'], '@tests/storage_tag.txt', None),
    ('BRIDGE_SINGLETON-given-and-read', ['storage', 'single'], '@tests/storage_single.txt', None),
    ('BRIDGE_SINGLETON-walked-refused', ['storage', 'singlebad1'],
     r"did not load: term 0 of system 'by': 'st:G' is a singleton, and is on no entity a system "
     r"walks.*alive", None),
    ('attach-singleton-to-an-id-refused', ['storage', 'singlebad2'],
     r"attach: 'st:G' is a singleton, and its id is 0.*alive", None),
    ('attach-many-values-through-little-scratch', ['host', 'attach-many-values-through-little-scratch'],
     r"game:mover@0.1.0: loaded.*moved.*alive", 'not loaded'),
    ('attach-to-id-zero-refused', ['storage', 'singlebad3'],
     r"attach: the id 0 is no entity, and 'st:A' is not a singleton.*alive", None),
    ('storage-direct', ['storage', 'direct'], '@tests/storage_direct.txt', None),
    ('storage-directstrip', ['storage', 'directstrip'], '@tests/storage_directstrip.txt', None),
    ('storage-directsweep', ['storage', 'directsweep'], '@tests/storage_directsweep.txt', None),
    ('storage-directdelete', ['storage', 'directdelete'], '@tests/storage_directdelete.txt', None),
    ('budget.direct_indexed-none-refused', ['storage', 'directnone'],
     r"did not load: type 'st:D' is one more than .budget. direct_indexed of 0.*alive", None),
    ('budget.columns-no-room-for-direct-indexed-refused', ['storage', 'directfull'],
     r"did not load: type 'st:D' is direct-indexed, and .budget. columns of 1024 bytes has no room for "
     r"a cell of 4 bytes for each of 400000 entities.*alive", None),
    ('BRIDGE_DIRECT_INDEXED-as-second-term-refused', ['storage', 'directbad'],
     r"did not load: term 1 of system 'both': 'st:D' is direct-indexed, and does not select what a "
     r"system walks.*alive", None),
    ('storage-sparse', ['storage', 'sparse'], '@tests/storage_sparse.txt', None),
    ('storage-sparsefull', ['storage', 'sparsefull'], '@tests/storage_sparsefull.txt', None),
    ('storage-sparsestrip', ['storage', 'sparsestrip'], '@tests/storage_sparsestrip.txt', None),
    ('storage-sparsesweep', ['storage', 'sparsesweep'], '@tests/storage_sparsesweep.txt', None),
    ('storage-sparsesweepa', ['storage', 'sparsesweepa'], '@tests/storage_sparsesweepa.txt', None),
    ('storage-sparsedelete', ['storage', 'sparsedelete'], '@tests/storage_sparsedelete.txt', None),
    ('budget.sparse_sets-none-refused', ['storage', 'sparsenone'],
     r"did not load: type 'st:S' is one more than .budget. sparse_sets of 0.*alive", None),
    ('BRIDGE_SPARSE_SET-as-second-term-refused', ['storage', 'sparsebad1'],
     r"did not load: term 1 of system 'both': 'st:S' is kept in a sparse set, and does not select the "
     r"rows of a table.*alive", None),
    ('BRIDGE_NOT-of-a-sparse-set-refused', ['storage', 'sparsebad2'],
     r"did not load: term 0 of system 'without': 'st:S' is kept in a sparse set, and a system cannot "
     r"walk the entities that are not in it.*alive", None),
    ('storage-baseline', ['storage', 'baseline'],
     r"read in order.*in no order.*204800 of them on.*found through its query.*alive", r"FAIL"),
    ('storage-measure', ['storage', 'measure'],
     r"an id handed out +[0-9.]+ ns each.*ten thousand tables made.*the stress run, on 1 thread.*alive", None),
    ('bridge_load-loaded', ['host', 'bridge_load-loaded'],
     r"test:ok@0.1.0: loaded.*alive", None),
    ('BRIDGE_NO_LIMIT-stated-for-every-limit', ['host', 'BRIDGE_NO_LIMIT-stated-for-every-limit'],
     r"test:ok@0.1.0: loaded.*alive", None),
    ('bridge_load-init-trapped', ['host', 'bridge_load-init-trapped'],
     r"module 'test:oob@0.1.0' did not load: its init trapped: the module asked for it.*alive", None),
    ('module_config.init_timeout-init-interrupted', ['host', 'module_config.init_timeout-init-interrupted'],
     r"module 'test:spin@0.1.0' did not load: its init trapped: the timeout passed.*alive", None),
    ('module_config.memory_limit-growth-refused', ['host', 'module_config.memory_limit-growth-refused'],
     r"module 'test:grow@0.1.0' did not load: its init answered 7.*alive", None),
    ('instance_get_func-no-such-export-refused', ['host', 'instance_get_func-no-such-export-refused'],
     r"module 'test:noinit@0.1.0' did not load: no exported function 'module_init'.*alive", None),
    ('bridge_load-no-init-export', ['host', 'bridge_load-no-init-export'],
     r"module 'test:noinit@0.1.0' did not load: no exported function 'module_init'.*alive", None),
    ('module_config.abi-another-refused', ['host', 'module_config.abi-another-refused'],
     r"module 'test:ok@0.1.0' is refused: it wants ABI 2 and this is ABI 1.*alive", None),
    ('artifact.backend-not-configured-refused', ['host', 'artifact.backend-not-configured-refused'],
     r"module 'test:ok@0.1.0' is refused: no backend named 'native'.*alive", None),
    ('module_config.log_level-unset-refused', ['host', 'module_config.log_level-unset-refused'],
     r"module 'test:ok@0.1.0' is refused: log_level is unset.*alive", ': loaded'),
    ('module_config.memory_limit-unset-refused', ['host', 'module_config.memory_limit-unset-refused'],
     r"module 'test:ok@0.1.0' is refused: memory_limit is unset.*alive", None),
    ('BRIDGE_NONE-for-a-limit-refused', ['host', 'BRIDGE_NONE-for-a-limit-refused'],
     r"module 'test:ok@0.1.0' is refused: spawns is -1, which is neither an amount nor BRIDGE_NONE.*module 'test:ok@0.1.0' is refused: work_limit is -2, which is neither a limit nor BRIDGE_NO_LIMIT.*alive", ': loaded'),
    ('config.abi-another-refused', ['host', 'config.abi-another-refused'],
     r"the host is of host ABI 2, and this is 1.*bridge_create refused.*alive", None),
    ('config.abi-unset-refused', ['host', 'config.abi-unset-refused'],
     r"abi is unset.*bridge_create refused.*alive", None),
    ('BRIDGE_UNSET-flag-refused', ['host', 'BRIDGE_UNSET-flag-refused'],
     r"module 'test:ok@0.1.0' is refused: pinned is unset.*module 'test:ok@0.1.0' is refused: trusted is 3, which is neither BRIDGE_TRUE nor BRIDGE_FALSE.*alive", ': loaded'),
    ('module_config.memory_limit-negative-refused', ['host', 'module_config.memory_limit-negative-refused'],
     r"module 'test:ok@0.1.0' is refused: memory_limit is -7, which is neither a limit nor BRIDGE_NO_LIMIT.*alive", ': loaded'),
    ('module_config.on_error-fail-stops-and-skip-does-not', ['host', 'module_config.on_error-fail-stops-and-skip-does-not'],
     r"test:ok@0.1.0: loaded.*the deck has stopped: module 'test:oob@0.1.0' failed and its on_error is fail.*test:ok@0.1.0: not loaded.*alive", None),
    ('backend.library-cannot-open-refused', ['host', 'backend.library-cannot-open-refused'],
     r"backend 'wasm': cannot open backend library 'no-such-backend-library'.*bridge_create refused.*alive", None),
    ('bridge_create-no-sink-refused', ['host', 'bridge_create-no-sink-refused'],
     r"refused: no sink.*alive", None),
    ('module_config.read-another-namespace-read', ['host', 'module_config.read-another-namespace-read'],
     r"sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive", None),
    ('record.level-info-with-its-two-numbers', ['host', 'record.level-info-with-its-two-numbers'],
     r"record.6. view:reader@0.1.0/sum: sum 250500 0.*alive", None),
    ('declare-storage-unset-refused', ['host', 'declare-storage-unset-refused'],
     r"module 'game:kept@0.1.0' did not load: type 'game:Position': its storage layout is unset.*"
     r"module 'game:kept@0.1.0' did not load: type 'game:Position' is stated as a sparse set here, and it "
     r"was declared as a table. It is also declared by 'game:mover@0.1.0'.*alive", None),
    ('BRIDGE_DIRECT_INDEXED-reached-from-rows', ['host', 'BRIDGE_DIRECT_INDEXED-reached-from-rows'],
     r"game:aim@0.1.0/reached: aims 10 5.*game:aim@0.1.0/reached: aimed 20 0.*alive", r"did not load"),
    ('BRIDGE_TOGGLE-counted-on-and-off', ['host', 'BRIDGE_TOGGLE-counted-on-and-off'],
     r"game:shield@0.1.0/count: shields 10 10.*game:shield@0.1.0/count: strength 45 0.*"
     r"game:shield@0.1.0/count: shields 10 6.*game:shield@0.1.0/count: strength 27 0.*alive", r"did not load"),
    ('BRIDGE_SPARSE_SET-walked-and-reached', ['host', 'BRIDGE_SPARSE_SET-walked-and-reached'],
     r"game:hit@0.1.0/walked: hits 4 18.*game:hit@0.1.0/reached: bodies 10 4.*"
     r"game:hit@0.1.0/reached: reached 18 0.*alive", r"did not load"),
    ('declare-another-layout-refused', ['host', 'declare-another-layout-refused'],
     r"module 'game:fp@0.1.0' did not load: type 'game:Position' has another component layout here: fingerprint [0-9a-f]+, and it was declared with [0-9a-f]+. It is also declared by 'game:mover@0.1.0'.*alive", None),
    ('declare-ungranted-namespace-refused', ['host', 'declare-ungranted-namespace-refused'],
     r"module 'view:reader@0.1.0' did not load: it may not declare 'game:Position': that is another namespace's, and no grant names it.*alive", None),
    ('add_system-unknown-operator-refused', ['host', 'add_system-unknown-operator-refused'],
     r"module 'game:badterm@0.1.0' did not load: term 0 of system 'any': its operator is 4, and there are three.*alive", None),
    ('module_config.work_limit-spinning-system-ended', ['host', 'module_config.work_limit-spinning-system-ended'],
     r"test:spin@0.1.0/forever: system 'forever' failed: trapped: the work limit is used up.*still here.*the work limit is used up.*still here.*alive", None),
    ('module_config.spawns-used-up', ['host', 'module_config.spawns-used-up'],
     r"system 'flood' failed: spawn: its spawns of 3000 between two barriers are used up.*system 'flood' failed: spawn: .budget. entities of 4096 is reached.*alive", None),
    ('module_config.log_ring-full-lines-dropped', ['host', 'module_config.log_ring-full-lines-dropped'],
     r"line 15 0.*test:chatty@0.1.0/-: 84 log lines were dropped: its ring holds 16.*alive", None),
    ('module_config.spans-too-small-refused', ['host', 'module_config.spans-too-small-refused'],
     r"module 'view:reader@0.1.0' did not load: its spans of 16 bytes do not hold a column_base of 2 components by \d+ tables.*moved 2 1000.*alive", None),
    ('budget.columns-used-up-changes-not-applied', ['host', 'budget.columns-used-up-changes-not-applied'],
     r"module 'game:mover@0.1.0': .budget. columns of 4096 bytes is used up. Not applied, of the changes it asked: 1244 .*moved 1 256.*alive", r"has stopped"),
    ('bridge_run_phase-undeclared-phase-refused', ['host', 'bridge_run_phase-undeclared-phase-refused'],
     r"phase 'render' is not one the deployment declared.*alive", None),
    ('config.phases-none-refused', ['host', 'config.phases-none-refused'],
     r"the deployment declares no phase.*bridge_create refused.*alive", None),
    ('budget.tables-unset-refused', ['host', 'budget.tables-unset-refused'],
     r".budget. tables is unset.*bridge_create refused.*alive", None),
    ('budget.types-unset-refused', ['host', 'budget.types-unset-refused'],
     r".budget. types is unset.*bridge_create refused.*alive", None),
    ('budget.systems-unset-refused', ['host', 'budget.systems-unset-refused'],
     r".budget. systems is unset.*bridge_create refused.*alive", None),
    ('budget.types-reached-refused', ['host', 'budget.types-reached-refused'],
     r"module 'game:mover@0.1.0' did not load: type 'game:Marked' is one more than .budget. types of 1 .*not loaded.*alive", None),
    ('budget.systems-reached-refused', ['host', 'budget.systems-reached-refused'],
     r"module 'game:mover@0.1.0' did not load: it registers one more than .budget. systems of 1 .*not loaded.*alive", None),
    ('module_config.log_level-no-severity-refused', ['host', 'module_config.log_level-no-severity-refused'],
     r"module 'game:mover@0.1.0' is refused: its log_level of 8 is not one it can be, which is 1 to 7.*not loaded.*alive", None),
    ('module_config.log_level-warnings-drop-info', ['host', 'module_config.log_level-warnings-drop-info'],
     r"game:mover@0.1.0: loaded\s+alive", r"moved"),
    ('budget.terms-unset-refused', ['host', 'budget.terms-unset-refused'],
     r".budget. terms is unset.*bridge_create refused.*alive", None),
    ('budget.terms-exceeded-refused', ['host', 'budget.terms-exceeded-refused'],
     r"module 'view:reader@0.1.0' did not load: a system asks for 2 terms, which is more than .budget. terms of 1 .*not loaded.*alive", None),
    ('budget.columns-unset-refused', ['host', 'budget.columns-unset-refused'],
     r".budget. columns is unset.*bridge_create refused.*alive", None),
    ('budget.entities-beyond-32-bits-refused', ['host', 'budget.entities-beyond-32-bits-refused'],
     r".budget. entities of 1099511627776 is more than an offset of 32 bits can reach.*bridge_create refused.*alive", None),
    ('bridge_create-budget-sum-beyond-32-bits-refused', ['host', 'bridge_create-budget-sum-beyond-32-bits-refused'],
     r".budget. cannot be satisfied: the shared memory would be [0-9]+ bytes and an offset into it is 32 bits.*bridge_create refused.*alive", None),
    ('module_config.scratch-private-beyond-32-bits-refused', ['host', 'module_config.scratch-private-beyond-32-bits-refused'],
     r"module 'game:mover@0.1.0' is refused: its scratch of 4294967040, log_ring of [0-9]+ and spans of 4096 bytes come to more than an offset of 32 bits can reach.*alive", None),
    ('bridge_run_phase-wasm-beside-native', ['host', 'bridge_run_phase-wasm-beside-native'],
     r"view:reader@0.1.0/sum: sum 250500 0.*native tally 2 1000.*sum 251000 0.*alive", None),
    ('BACKEND_UNTRUSTED-lacking-untrusted-refused', ['host', 'BACKEND_UNTRUSTED-lacking-untrusted-refused'],
     r"module 'game:mover@0.1.0' is refused: it is not trusted, and backend 'native' runs only modules that are.*alive", r"cannot open"),
    ('BACKEND_WORK_LIMIT-lacking-work-limit-refused', ['host', 'BACKEND_WORK_LIMIT-lacking-work-limit-refused'],
     r"module 'game:mover@0.1.0' is refused: it states work_limit, which backend 'native' cannot keep.*alive", r"cannot open"),
    ('BACKEND_UNLOAD-library-gone-or-kept', ['host', 'BACKEND_UNLOAD-library-gone-or-kept'],
     r"the wasm backend's library is still loaded, the native backend's is gone.*alive", None),
    ('backend_get_abi-another-refused', ['host', 'backend_get_abi-another-refused'],
     r"backend 'native': backend library '[^']*bridge_native_other[^']*' is of backend ABI 2, and this is 1.*bridge_create refused.*alive", None),
    ('artifact.path-cannot-open', ['host', 'artifact.path-cannot-open'],
     r"module 'game:mover@0.1.0' did not load: cannot open the library 'no-such-library'.*alive", None),
    ('spawn-refused-fails-a-native-invocation', ['host', 'spawn-refused-fails-a-native-invocation'],
     r"test:flood@0.1.0/flood: system 'flood' failed: spawn: its spawns of 1024 between two barriers are used up.*alive", None),
    ('config.order-mover-before-halver', ['host', 'config.order-mover-before-halver'],
     r"sum 125250 0.*alive", None),
    ('config.order-halver-before-mover', ['host', 'config.order-halver-before-mover'],
     r"sum 125500 0.*alive", None),
    ('order-unordered-conflict-refused', ['host', 'order-unordered-conflict-refused'],
     r"module 'view:halver@0.1.0' did not load: 'game:mover:move' and 'view:halver:halve' both touch 'game:Position' in phase 'update', one of them writing it, and nothing orders them.*alive", None),
    ('add_system-own-conflict-in-registration-order', ['host', 'add_system-own-conflict-in-registration-order'],
     r"game:twostep@0.1.0/show: x 3 0.*alive", None),
    ('config.order-cycle-refused', ['host', 'config.order-cycle-refused'],
     r".order. contradicts itself.*bridge_create refused.*alive", None),
    ('observe-native-attach-wasm-observer', ['host', 'observe-native-attach-wasm-observer'],
     r"b2 game:setter@0.1.0/mark: marked.*b2 view:watcher@0.1.0/seen: seen 1 77.*alive", None),
    ('observe-chain-one-hop-a-barrier', ['host', 'observe-chain-one-hop-a-barrier'],
     r"b1 game:chain@0.1.0/got_a: hop A.*b2 game:chain@0.1.0/got_b: hop B.*b3 game:chain@0.1.0/got_c: hop C.*alive", r"hop A.*hop A"),
    ('observe-reserved-name-refused', ['host', 'observe-reserved-name-refused'],
     r"'init' is a reserved name, and no observer may take it.*alive", None),
    ('config.threads-two-run-at-once', ['host', 'config.threads-two-run-at-once'],
     r"on 2 thread.s.[^a-z]*they ran at once.*alive", None),
    ('config.threads-one-runs-in-turn', ['host', 'config.threads-one-runs-in-turn'],
     r"on 1 thread.s.[^a-z]*they ran one after the other.*alive", None),
    ('module_config.pinned-runs-on-the-loading-thread', ['host', 'module_config.pinned-runs-on-the-loading-thread'],
     r"the others used [2-4] thread.s.[^a-z]*pinned: 9 of its 9 calls were on the thread that loaded it.*alive", None),
    ('bridge_run_phase-pinned-from-another-thread-refused', ['host', 'bridge_run_phase-pinned-from-another-thread-refused'],
     r"module 'test:pinned@0.1.0' is pinned to the thread that loaded it, and this call comes from another.*from another thread: refused.*bridge_destroy is refused: module 'test:pinned@0.1.0' is pinned.*destroy from another thread: refused.*destroy from the thread that loaded it: done.*alive", None),
    ('config.threads-unset-refused', ['host', 'config.threads-unset-refused'],
     r"threads is unset.*bridge_create refused.*alive", None),
    ('config.threads-four-wasm-instances-hop', ['host', 'config.threads-four-wasm-instances-hop'],
     r"sum 275000 0.*alive", None),
    ('order-unordered-observers-refused', ['host', 'order-unordered-observers-refused'],
     r"module 'view:fixer@0.1.0' did not load: 'view:watcher:seen' and 'view:fixer:fix' both touch 'game:Marked' at the barrier, one of them writing it, and nothing orders them.*seen 1 77.*alive", None),
    ('module_config.changes-used-up', ['host', 'module_config.changes-used-up'],
     r"module 'game:stamp5@0.1.0' did not load: its changes of 16 bytes are used up.*not loaded.*alive", None),
    ('module_config.changes-stated-as-none-refused', ['host', 'module_config.changes-stated-as-none-refused'],
     r"module 'game:stamp5@0.1.0' did not load: it stated that it queues no change.*not loaded.*alive", None),
    ('module_config.phases-unmapped-refused', ['host', 'module_config.phases-unmapped-refused'],
     r"a system names phase 'tick', which the deployment did not declare.*not loaded.*alive", 'seen'),
    ('phase_name.from-mapped-loads', ['host', 'phase_name.from-mapped-loads'],
     r"game:stamp@0.1.0: loaded.*seen 1 7.*alive", None),
    ('phase_name.to-undeclared-refused', ['host', 'phase_name.to-undeclared-refused'],
     r"module 'game:stamp@0.1.0' is refused: its config maps phase 'tick' to 'nowhere', which the deployment did not declare.*not loaded.*alive", 'seen'),
    ('attach-unstated-change-fails', ['host', 'attach-unstated-change-fails'],
     r"system 'stamp' failed: system 'stamp' may not attach 'game:Marked': it is not among the changes it stated.*alive", 'seen'),
    ('order-unordered-changes-refused', ['host', 'order-unordered-changes-refused'],
     r"module 'game:stamp9@0.1.0' did not load: 'game:stamp5:stamp' and 'game:stamp9:stamp' both attach or detach 'game:Marked' in phase 'update', and nothing orders them.*seen 1 5.*alive", 'seen 1 9'),
    ('order-changes-ordered-loaded-first', ['host', 'order-changes-ordered-loaded-first'],
     r"seen 1 9.*alive", 'seen 1 5'),
    ('order-changes-ordered-loaded-last', ['host', 'order-changes-ordered-loaded-last'],
     r"seen 1 9.*alive", 'seen 1 5'),
    ('destroy-with-a-read-grant-left', ['host', 'destroy-with-a-read-grant-left'],
     r"reap: rows 1 0.*module 'view:reaper@0.1.0' asked to destroy entities that carry nothing it may write. Left as they were: 1.*reap: rows 1 0.*alive", 'rows 0 0'),
    ('module_config.write-destroy-goes', ['host', 'module_config.write-destroy-goes'],
     r"reap: rows 1 0.*reap: rows 0 0.*alive", 'Left as they were'),
    ('budget.tables-used-up-both-told', ['host', 'budget.tables-used-up-both-told'],
     r"module 'game:stamp5@0.1.0': .budget. tables of 1 is used up. Not applied, of the changes it asked: 2.*module 'game:stamp9@0.1.0': .budget. tables of 1 is used up. Not applied, of the changes it asked: 2.*alive", 'seen'),
    ('config.order-observers-watcher-first', ['host', 'config.order-observers-watcher-first'],
     r"view:fixer@0.1.0: loaded.*seen 1 77.*alive", None),
    ('config.order-observers-fixer-first', ['host', 'config.order-observers-fixer-first'],
     r"view:fixer@0.1.0: loaded.*seen 1 5.*alive", None),
    ('declare-nested-types', ['host', 'declare-nested-types'],
     r"read: body 2 2.*read: more 3 4.*alive", None),
    ('declare-undeclared-field-type-refused', ['host', 'declare-undeclared-field-type-refused'],
     r"module 'game:nested@0.1.0' did not load: type 'game:Body': its descriptor says 'vec2_t', which is neither a type a field can be nor one its namespace has declared.*alive", None),
    ('backend_create-unknown-setting-refused', ['host', 'backend_create-unknown-setting-refused'],
     r"backend 'native' would not start: it knows no setting 'cache'.*bridge_create refused.*alive", None),
    ('bridge_create-two-decks-at-once', ['host', 'bridge_create-two-decks-at-once'],
     r"two decks: sums 275000 and 265000.*alive", None),
    ('alloc-string-kept', ['host', 'alloc-string-kept'],
     r"game:namer@0.1.0/say: kept in the arena.*alive", None),
    ('alloc-share-exhausted', ['host', 'alloc-share-exhausted'],
     r"module 'game:namer@0.1.0' did not load: alloc: its persistent of 4 bytes is exhausted.*alive", None),
    ('module_config.persistent-more-than-the-arena-refused', ['host', 'module_config.persistent-more-than-the-arena-refused'],
     r"module 'game:namer@0.1.0' is refused: its persistent of 1048576 bytes does not fit in the 4096 left of .budget. persistent.*alive", None),
    ('spawn-stated-as-none-refused', ['host', 'spawn-stated-as-none-refused'],
     r"module 'game:mover@0.1.0' did not load: spawn: it stated that it spawns nothing.*alive", None),
    ('bridge_load-failed-init-undone', ['host', 'bridge_load-failed-init-undone'],
     r"game:badterm@0.1.0: not loaded.*game:fp@0.1.0: loaded.*alive", None),
    ('spawn-same-ids-on-one-thread-and-four', ['host', 'spawn-same-ids-on-one-thread-and-four'],
     r"minted 160 ids a run. on four threads the same, and again the same.*alive", None),
    ('BRIDGE_SKIP-failed-init-skipped', ['host', 'BRIDGE_SKIP-failed-init-skipped'],
     r"game:badobs@0.1.0: not loaded.*show: x 3 0.*modules: game:twostep@0.1.0=RUNNING .1..*alive", None),
    ('bridge_load-refused-leaves-nothing', ['host', 'bridge_load-refused-leaves-nothing'],
     r"game:badterm@0.1.0: not loaded.*game:fp@0.1.0: loaded.*alive", None),
    ('module_config.max_system_failures-reached-stops-the-module', ['host', 'module_config.max_system_failures-reached-stops-the-module'],
     r"module 'test:flaky@0.1.0' is stopped: its system 'trip' reached its max_system_failures of 3.*show: x 63 0.*game:twostep@0.1.0=RUNNING test:flaky@0.1.0=STOPPED.*alive", None),
    ('module_config.max_system_failures-one-in-three-forgiven', ['host', 'module_config.max_system_failures-one-in-three-forgiven'],
     r"modules: test:flaky@0.1.0=RUNNING.*alive", None),
    ('module_config.max_system_failures-two-in-three-reached', ['host', 'module_config.max_system_failures-two-in-three-reached'],
     r"its system 'trip' reached its max_system_failures of 3.*modules: test:flaky@0.1.0=STOPPED.*alive", None),
    ('module_config.max_system_failures-clean-system-forgives-no-other', ['host', 'module_config.max_system_failures-clean-system-forgives-no-other'],
     r"its system 'trip' reached its max_system_failures of 3.*modules: test:flaky@0.1.0=STOPPED.*alive", None),
    ('module_config.max_system_failures-no-limit-never-stops', ['host', 'module_config.max_system_failures-no-limit-never-stops'],
     r"modules: test:flaky@0.1.0=RUNNING.*alive", None),
    ('BRIDGE_FAIL-stopped-module-stops-the-deck', ['host', 'BRIDGE_FAIL-stopped-module-stops-the-deck'],
     r"the deck has stopped: module 'test:flaky@0.1.0' was stopped and its on_error is fail.*update refused.*test:flaky@0.1.0=STOPPED.*game:twostep@0.1.0: not unloaded.*alive", None),
    ('bridge_unload-of-what-stopped-the-deck', ['host', 'bridge_unload-of-what-stopped-the-deck'],
     r"the deck has stopped.*game:twostep@0.1.0: not unloaded.*the deck runs again: module 'test:flaky@0.1.0' is unloaded, and it goes on without it.*test:flaky@0.1.0: unloaded.*modules: game:twostep@0.1.0=RUNNING.*alive", None),
    ('BRIDGE_STRIP-unload-strips', ['host', 'BRIDGE_STRIP-unload-strips'],
     r"left 10 5.*game:keeper@0.1.0: unloaded.*left 9 5.*game:retype@0.1.0: loaded.*alive", None),
    ('BRIDGE_SWEEP-unload-sweeps', ['host', 'BRIDGE_SWEEP-unload-sweeps'],
     r"left 10 5.*game:keeper@0.1.0: unloaded.*left 9 0.*game:retype@0.1.0: loaded.*alive", None),
    ('BRIDGE_DELETE-unload-deletes', ['host', 'BRIDGE_DELETE-unload-deletes'],
     r"left 10 5.*game:keeper@0.1.0: unloaded.*left 0 0.*game:retype@0.1.0: loaded.*alive", None),
    ('bridge_unload-declarer-goes-user-holds-the-type', ['host', 'bridge_unload-declarer-goes-user-holds-the-type'],
     r"game:typer@0.1.0: unloaded.*read: body 2 3.*game:retype@0.1.0: not loaded.*game:user@0.1.0: unloaded.*game:retype@0.1.0: loaded.*alive", None),
    ('bridge_unload-not-loaded-refused', ['host', 'bridge_unload-not-loaded-refused'],
     r"no module 'game:nobody@0.1.0' is loaded, so none is unloaded.*alive", None),
    ('module_config.shutdown_timeout-shutdown-interrupted', ['host', 'module_config.shutdown_timeout-shutdown-interrupted'],
     r"its shutdown trapped: the timeout passed. its teardown continues.*test:hang@0.1.0: unloaded.*modules: .0..*alive", None),
    ('budget.modules-reached-refused', ['host', 'budget.modules-reached-refused'],
     r"module 'game:ticker@0.1.0' is refused: .budget. modules of 1 is reached.*game:twostep@0.1.0: unloaded.*game:ticker@0.1.0: loaded.*modules: game:ticker@0.1.0=RUNNING .1..*alive", None),
    ('bridge_reload-reloaded', ['host', 'bridge_reload-reloaded'],
     r"tick 102 1.*game:ticker@0.1.0/-: shutdown 1 0.*game:ticker@0.2.0: reloaded.*tick 112 10.*tick 122 10.*alive", None),
    ('bridge_reload-room-for-one-system', ['host', 'bridge_reload-room-for-one-system'],
     r"tick 102 1.*game:ticker@0.2.0: reloaded.*tick 112 10.*tick 122 10.*alive", 'systems of 1'),
    ('bridge_reload-failed-with-room-for-one-system', ['host', 'bridge_reload-failed-with-room-for-one-system'],
     r"was not reloaded: its reload answered 5.*tick 103 1.*tick 104 1.*game:ticker@0.1.0=RUNNING.*alive", 'systems of 1'),
    ('bridge_reload-more-spawns-stated', ['host', 'bridge_reload-more-spawns-stated'],
     r"tick 102 1.*game:ticker@0.2.0: reloaded.*tick 112 10.*tick 122 10.*alive", None),
    ('bridge_reload-wider-layout', ['host', 'bridge_reload-wider-layout'],
     r"tick 102 1.*game:ticker@0.2.0: reloaded.*tick 112 10.*tick 122 10.*alive", r"was not reloaded"),
    ('bridge_reload-reload-answered-nonzero', ['host', 'bridge_reload-reload-answered-nonzero'],
     r"was not reloaded: its reload answered 5.*tick 103 1.*tick 104 1.*game:ticker@0.1.0=RUNNING.*alive", None),
    ('bridge_reload-larger-share', ['host', 'bridge_reload-larger-share'],
     r"game:ticker@0.2.0: reloaded.*tick 112 10.*tick 122 10.*alive", None),
    ('bridge_reload-larger-share-past-a-neighbour', ['host', 'bridge_reload-larger-share-past-a-neighbour'],
     r"game:namer@0.1.0: loaded.*game:ticker@0.2.0: reloaded.*tick 112 10.*alive", None),
    ('bridge_reload-same-share-exhausted', ['host', 'bridge_reload-same-share-exhausted'],
     r"was not reloaded: alloc: its persistent of 16 bytes is exhausted.*tick 103 1.*tick 104 1.*alive", None),
    ('bridge_reload-smaller-share-refused', ['host', 'bridge_reload-smaller-share-refused'],
     r"is not reloaded: its persistent is 16 and a reload states less, which a loaded module's share cannot become.*tick 104 1.*alive", None),
    ('bridge_reload-share-more-than-the-arena-refused', ['host', 'bridge_reload-share-more-than-the-arena-refused'],
     r"is not reloaded: its persistent of 1048576 bytes does not fit in the [0-9]+ left of .budget. persistent.*tick 104 1.*alive", None),
    ('bridge_reload-lifts-a-stopped-deck', ['host', 'bridge_reload-lifts-a-stopped-deck'],
     r"the deck has stopped: module 'game:ticker@0.1.0' was stopped and its on_error is fail.*game:twostep@0.1.0: not loaded.*the deck runs again: module 'game:ticker@0.2.0' is back.*game:ticker@0.2.0: reloaded.*tick 1[0-9]3 10.*game:ticker@0.2.0=RUNNING.*alive", None),
    ('bridge_load-lifts-a-stopped-deck', ['host', 'bridge_load-lifts-a-stopped-deck'],
     r"the deck has stopped: module 'game:thing@0.1.0' failed and its on_error is fail.*game:other@0.1.0: not loaded.*the deck runs again: module 'game:thing@0.2.0' is back.*game:thing@0.2.0: loaded.*show: x 3 0.*alive", None),
    ('bridge_reload-to-another-backend-refused', ['host', 'bridge_reload-to-another-backend-refused'],
     r"it runs on backend 'wasm' and a reload does not move it to 'native'.*tick 104 1.*alive", None),
    ('bridge_reload-dropped-declaration-frees-its-name', ['host', 'bridge_reload-dropped-declaration-frees-its-name'],
     r"game:ticker@0.2.0: reloaded.*tick 122 10.*game:retype@0.1.0: loaded.*alive", None),
    ('module_config.hot_reload-false-refused', ['host', 'module_config.hot_reload-false-refused'],
     r"is not reloaded: its hot_reload is false.*tick 104 1.*alive", None),
    ('bridge_reload-stopped-module-revived', ['host', 'bridge_reload-stopped-module-revived'],
     r"game:ticker@0.1.0=STOPPED.*game:ticker@0.2.0: reloaded.*tick 1[0-9]3 10.*game:ticker@0.2.0=RUNNING.*alive", None),
    ('bridge_load-no-reload-export', ['host', 'bridge_load-no-reload-export'],
     r"module 'test:noreload@0.1.0' did not load: no exported function 'module_reload'.*alive", None),
    ('module_config.reloadable-false-with-hot-reload-refused', ['host', 'module_config.reloadable-false-with-hot-reload-refused'],
     r"module 'game:twostep@0.1.0' is refused: hot_reload is stated, and its author states it is not reloadable.*alive", None),
    ('bridge_reload-not-loaded-refused', ['host', 'bridge_reload-not-loaded-refused'],
     r"is not reloaded: nothing of that name is loaded.*alive", None),
    ('bridge_reload-native', ['host', 'bridge_reload-native'],
     r"tick 102 1.*game:ticker@0.1.0/-: shutdown 1 0.*game:ticker@0.2.0: reloaded.*tick 112 10.*tick 122 10.*alive", None),
    ('BACKEND_SIDE_BY_SIDE-lacking-failed-reload-stops-the-module', ['host', 'BACKEND_SIDE_BY_SIDE-lacking-failed-reload-stops-the-module'],
     r"shutdown 1 0.*was not reloaded: its reload answered 5.*module 'game:ticker@0.1.0' is stopped: its reload failed after its old instance had been shut down. What it declared is kept.*game:ticker@0.1.0=STOPPED.*game:ticker@0.3.0: reloaded.*tick 112 10.*tick 122 10.*game:ticker@0.3.0=RUNNING.*alive", None),
    ('bridge_unload-after-a-failed-native-reload', ['host', 'bridge_unload-after-a-failed-native-reload'],
     r"game:ticker@0.1.0=STOPPED.*game:ticker@0.1.0: unloaded.*modules: game:twostep@0.1.0=RUNNING .1..*alive", None),
    ('module_config.settings-of-every-kind', ['host', 'module_config.settings-of-every-kind'],
     r"vsync width 1 1280.*scale mode 150 2.*show: hello 7 0.*show: some/long/path.cfg 0 0.*flags ratio 3 3.*show: big 2 0.*alive", None),
    ('setting.key-no-such-field-refused', ['host', 'setting.key-no-such-field-refused'],
     r"its settings have a key 'vsynk', and 'game:Settings' has no field of that name.*alive", None),
    ('setting.kind-text-for-a-float-refused', ['host', 'setting.kind-text-for-a-float-refused'],
     r"its setting 'scale' is text, and the field it names is a float.*alive", None),
    ('BRIDGE_STRING-longer-than-the-field-refused', ['host', 'BRIDGE_STRING-longer-than-the-field-refused'],
     r"its setting 'title' is 20 bytes of text, and the field it names holds 16 with the zero that ends it.*alive", None),
    ('BRIDGE_INT-beyond-the-field-refused', ['host', 'BRIDGE_INT-beyond-the-field-refused'],
     r"its setting 'small' is 300, which the field it names cannot hold.*alive", None),
    ('BRIDGE_INT-beyond-the-enum-refused', ['host', 'BRIDGE_INT-beyond-the-enum-refused'],
     r"its setting 'mode' is 200, which the enum of the field it names cannot hold.*alive", None),
    ('BRIDGE_STRING-no-such-constant-refused', ['host', 'BRIDGE_STRING-no-such-constant-refused'],
     r"its setting 'mode' is 'Sometimes', which the enum of the field it names has no constant for.*alive", None),
    ('BRIDGE_NO_CONFIG-with-settings-refused', ['host', 'BRIDGE_NO_CONFIG-with-settings-refused'],
     r"it is handed 10 settings, and states that it has no config component.*alive", None),
    ('module_config.config-unset-refused', ['host', 'module_config.config-unset-refused'],
     r"module 'game:settings@0.1.0' is refused: config is unset.*alive", None),
    ('module_config.config-undeclared-refused', ['host', 'module_config.config-undeclared-refused'],
     r"its config is 'game:Nope', which is not a struct its init declared.*alive", None),
    ('module_config.settings-text-beyond-the-share-refused', ['host', 'module_config.settings-text-beyond-the-share-refused'],
     r"its settings hold 24 bytes of text, and its persistent share has 8 left.*alive", None),
    ('module_config.config-taken-refused', ['host', 'module_config.config-taken-refused'],
     r"its config is 'game:Settings', which is already the settings of module 'game:settings@0.1.0'.*alive", None),
    ('bridge_reload-settings-row-kept', ['host', 'bridge_reload-settings-row-kept'],
     r"vsync width 1 1280.*game:settings@0.2.0: reloaded.*vsync width 1 1280.*alive", None),
    ('bridge_reload-config-dropped', ['host', 'bridge_reload-config-dropped'],
     r"game:settings@0.2.0: reloaded.*game:retype@0.1.0: loaded.*alive", None),
    ('module_config.settings-reseeded-in-place', ['host', 'module_config.settings-reseeded-in-place'],
     r"settings text written to the same place.*alive", None),
    ('BRIDGE_FAIL-failed-reload-stops-the-deck', ['host', 'BRIDGE_FAIL-failed-reload-stops-the-deck'],
     r"the deck has stopped: module 'game:ticker@0.1.0' could not be reloaded and its on_error is fail.*update refused.*the deck runs again: module 'game:ticker@0.3.0' is back.*tick 112 10.*game:ticker@0.3.0=RUNNING.*alive", None),
    ('BACKEND_TIMEOUT-lacking-timeout-refused', ['host', 'BACKEND_TIMEOUT-lacking-timeout-refused'],
     r"module 'game:ticker@0.1.0' is refused: it states shutdown_timeout, which backend 'native' cannot keep.*alive", r"cannot open"),
    ('budget.columns-no-room-for-a-singleton-refused', ['host', 'budget.columns-no-room-for-a-singleton-refused'],
     r"did not load: type 'game:Settings' is a singleton, and .budget. columns of 16 bytes has no room "
     r"for its \d+ bytes.*alive", None),
    ('module_config.config-not-a-singleton-refused', ['host', 'module_config.config-not-a-singleton-refused'],
     r"did not load: its config is 'game:Settings', which is not declared a singleton.*alive", None),
    ('artifact.path-none-for-native', ['host', 'artifact.path-none-for-native'],
     r"module 'game:mover@0.1.0' did not load: the tests' backend opens a module's library by its path, and was handed neither.*alive", None),
    ('artifact.bytes-none-for-wasm', ['host', 'artifact.bytes-none-for-wasm'],
     r"module 'view:reader@0.1.0' did not load: the tests' backend opens a module's library by its path, and was handed neither.*alive", None),
    ('bridge_unload-two-hundred-rounds', ['host', 'bridge_unload-two-hundred-rounds'],
     r"churned 200 times, 0 loads refused, 0 records of the index out of step.*alive", None),
    ('bridge_unload-share-given-back', ['host', 'bridge_unload-share-given-back'],
     r"game:namer@0.1.0/say: kept in the arena.*game:namer@0.1.0: unloaded.*game:ticker@0.1.0: loaded.*alive", None),
    ('bridge_unload-share-kept-while-pointed-into', ['host', 'bridge_unload-share-kept-while-pointed-into'],
     r"module 'game:namer@0.1.0' is unloaded and its share of the persistent arena, 64 bytes, stays: 'game:Name' holds a string in it, in this many rows: 1.*game:namer@0.1.0: unloaded.*game:reader@0.1.0/say: kept in the arena.*module 'game:ticker@0.1.0' is refused: its persistent of 64 bytes does not fit in the 0 left of .budget. persistent.*game:reader@0.1.0/say: kept in the arena.*alive", None),
    ('bridge_destroy-shuts-down-what-is-loaded', ['host', 'bridge_destroy-shuts-down-what-is-loaded'],
     r"tick 101 1.*game:ticker@0.1.0/-: shutdown 1 0.*module 'test:hang@0.1.0': its shutdown trapped: the timeout passed. its teardown continues.*alive", None),
    ('budget.entities-unset-refused', ['host', 'budget.entities-unset-refused'],
     r".budget. entities is unset.*alive", None),
    ('budget.modules-unset-refused', ['host', 'budget.modules-unset-refused'],
     r".budget. modules is unset.*alive", None),
    ('budget.persistent-unset-refused', ['host', 'budget.persistent-unset-refused'],
     r".budget. persistent is unset.*alive", None),
    ('budget.sparse_sets-unset-refused', ['host', 'budget.sparse_sets-unset-refused'],
     r".budget. sparse_sets is unset.*alive", None),
    ('budget.sparse_sets-negative-refused', ['host', 'budget.sparse_sets-negative-refused'],
     r".budget. sparse_sets is -5, which is neither a count nor none.*alive", None),
    ('budget.sparse_sets-beyond-32-bits-refused', ['host', 'budget.sparse_sets-beyond-32-bits-refused'],
     r".budget. cannot be satisfied: sparse_sets of 16777216, with four bytes for each of 4096 entities, is more than an offset of 32 bits can reach.*alive", None),
    ('budget.direct_indexed-unset-refused', ['host', 'budget.direct_indexed-unset-refused'],
     r".budget. direct_indexed is unset.*alive", None),
    ('budget.direct_indexed-no-limit-refused', ['host', 'budget.direct_indexed-no-limit-refused'],
     r".budget. direct_indexed is -1, which is neither a count nor none.*alive", None),
    ('budget.direct_indexed-beyond-32-bits-refused', ['host', 'budget.direct_indexed-beyond-32-bits-refused'],
     r".budget. cannot be satisfied: direct_indexed of 16777216, with a bit for each of 4096 entities, is more than an offset of 32 bits can reach.*alive", None),
    ('backend.name-none-refused', ['host', 'backend.name-none-refused'],
     r"backend 1 has no name.*alive", None),
    ('backend.library-none-refused', ['host', 'backend.library-none-refused'],
     r"backend 1 has no library.*alive", None),
    ('backend.name-twice-refused', ['host', 'backend.name-twice-refused'],
     r"backend 'wasm' is named twice.*alive", None),
    ('config.phases-nameless-refused', ['host', 'config.phases-nameless-refused'],
     r"phase 1 has no name.*alive", None),
    ('config.phases-twice-refused', ['host', 'config.phases-twice-refused'],
     r"phase 'update' is named twice.*alive", None),
    ('order.before-none-refused', ['host', 'order.before-none-refused'],
     r".order. pair 0 does not name two systems.*alive", None),
    ('order.after-none-refused', ['host', 'order.after-none-refused'],
     r".order. pair 0 does not name two systems.*alive", None),
    ('module_config.identity-none-refused', ['host', 'module_config.identity-none-refused'],
     r"a module with no identity is refused.*alive", None),
    ('module_config.trusted-unset-refused', ['host', 'module_config.trusted-unset-refused'],
     r"module 'test:ok@0\.1\.0' is refused: trusted is unset.*alive", None),
    ('module_config.reloadable-unset-refused', ['host', 'module_config.reloadable-unset-refused'],
     r"module 'test:ok@0\.1\.0' is refused: reloadable is unset.*alive", None),
    ('module_config.hot_reload-unset-refused', ['host', 'module_config.hot_reload-unset-refused'],
     r"module 'test:ok@0\.1\.0' is refused: hot_reload is unset.*alive", None),
    ('module_config.on_error-unset-refused', ['host', 'module_config.on_error-unset-refused'],
     r"module 'test:ok@0\.1\.0' is refused: on_error is unset.*alive", None),
    ('module_config.max_system_failures-unset-refused', ['host', 'module_config.max_system_failures-unset-refused'],
     r"module 'test:ok@0\.1\.0' is refused: max_system_failures is unset.*alive", None),
    ('module_config.work_limit-unset-refused', ['host', 'module_config.work_limit-unset-refused'],
     r"module 'test:ok@0\.1\.0' is refused: work_limit is unset.*alive", None),
    ('module_config.init_timeout-unset-refused', ['host', 'module_config.init_timeout-unset-refused'],
     r"module 'test:ok@0\.1\.0' is refused: init_timeout is unset.*alive", None),
    ('module_config.reload_timeout-unset-refused', ['host', 'module_config.reload_timeout-unset-refused'],
     r"module 'test:ok@0\.1\.0' is refused: reload_timeout is unset.*alive", None),
    ('module_config.shutdown_timeout-unset-refused', ['host', 'module_config.shutdown_timeout-unset-refused'],
     r"module 'test:ok@0\.1\.0' is refused: shutdown_timeout is unset.*alive", None),
    ('module_config.scratch-unset-refused', ['host', 'module_config.scratch-unset-refused'],
     r"module 'test:ok@0\.1\.0' is refused: scratch is unset.*alive", None),
    ('module_config.log_ring-unset-refused', ['host', 'module_config.log_ring-unset-refused'],
     r"module 'test:ok@0\.1\.0' is refused: log_ring is unset.*alive", None),
    ('module_config.log_ring-smaller-than-a-record-refused', ['host', 'module_config.log_ring-smaller-than-a-record-refused'],
     r"module 'test:ok@0\.1\.0' is refused: a log_ring of 8 bytes holds no record, and one is 24.*alive", None),
    ('module_config.spans-unset-refused', ['host', 'module_config.spans-unset-refused'],
     r"module 'test:ok@0\.1\.0' is refused: spans is unset.*alive", None),
    ('module_config.spawns-unset-refused', ['host', 'module_config.spawns-unset-refused'],
     r"module 'test:ok@0\.1\.0' is refused: spawns is unset.*alive", None),
    ('module_config.spawns-more-than-entities-refused', ['host', 'module_config.spawns-more-than-entities-refused'],
     r"module 'test:ok@0\.1\.0' is refused: its spawns of 5000 is more than .budget. entities of 4096.*alive", None),
    ('module_config.persistent-unset-refused', ['host', 'module_config.persistent-unset-refused'],
     r"module 'test:ok@0\.1\.0' is refused: persistent is unset.*alive", None),
    ('module_config.changes-unset-refused', ['host', 'module_config.changes-unset-refused'],
     r"module 'test:ok@0\.1\.0' is refused: changes is unset.*alive", None),
    ('module_config.changes-beyond-32-bits-refused', ['host', 'module_config.changes-beyond-32-bits-refused'],
     r"module 'test:ok@0\.1\.0' is refused: its changes of 8589934592 bytes is more than an offset of 32 bits can reach.*alive", None),
    ('module_config.losses-count-unset-refused', ['host', 'module_config.losses-count-unset-refused'],
     r"module 'test:ok@0\.1\.0' is refused: loss_count is unset.*alive", None),
    ('module_config.losses-counted-and-none-refused', ['host', 'module_config.losses-counted-and-none-refused'],
     r"module 'test:ok@0\.1\.0' is refused: it states 2 losses, and losses is unset.*alive", None),
    ('bridge_load-loaded-already-refused', ['host', 'bridge_load-loaded-already-refused'],
     r"module 'test:ok@0\.1\.0' is refused: 'test:ok@0\.1\.0' is loaded already.*alive", None),
    ('module_config.spawns-sum-beyond-entities-refused', ['host', 'module_config.spawns-sum-beyond-entities-refused'],
     r"module 'test:image@0\.1\.0' did not load: its spawns of 3000 do not fit under .budget. entities of 4096.*alive", None),
    ('phase_name.from-unset-refused', ['host', 'phase_name.from-unset-refused'],
     r"module 'test:ok@0\.1\.0' is refused: phase name 1 of its config is unset.*alive", None),
    ('module_config.phases-mapped-twice-refused', ['host', 'module_config.phases-mapped-twice-refused'],
     r"module 'test:ok@0\.1\.0' is refused: its config maps phase 'tick' twice.*alive", None),
    ('declare-statement-outside-scratch-refused', ['host', 'declare-statement-outside-scratch-refused'],
     r"module 'game:wrong@0\.1\.0' did not load: declare: what it states lies outside the scratch arena.*alive", None),
    ('bridge_declaration_t-name-outside-scratch-refused', ['host', 'bridge_declaration_t-name-outside-scratch-refused'],
     r"module 'game:wrong@0\.1\.0' did not load: declare: a name or a descriptor lies outside the scratch arena.*alive", None),
    ('declare-name-with-no-namespace-refused', ['host', 'declare-name-with-no-namespace-refused'],
     r"module 'game:wrong@0\.1\.0' did not load: type 'Odd' has no namespace.*alive", None),
    ('declare-kind-unset-refused', ['host', 'declare-kind-unset-refused'],
     r"module 'game:wrong@0\.1\.0' did not load: type 'game:Odd': its kind is unset.*alive", None),
    ('BRIDGE_TAG-with-bytes-refused', ['host', 'BRIDGE_TAG-with-bytes-refused'],
     r"module 'game:wrong@0\.1\.0' did not load: type 'game:Odd' is a tag, and a tag has no bytes and no descriptor.*alive", None),
    ('declare-on-unload-unset-refused', ['host', 'declare-on-unload-unset-refused'],
     r"module 'game:wrong@0\.1\.0' did not load: type 'game:Odd': on_unload is unset.*alive", None),
    ('declare-size-other-than-descriptor-refused', ['host', 'declare-size-other-than-descriptor-refused'],
     r"module 'game:wrong@0\.1\.0' did not load: type 'game:Odd' is 12 bytes aligned to 4 in the module and 8 aligned to 4 by its descriptor.*alive", None),
    ('add_system-statement-outside-scratch-refused', ['host', 'add_system-statement-outside-scratch-refused'],
     r"module 'game:wrong@0\.1\.0' did not load: add_system: what it states lies outside the scratch arena.*alive", None),
    ('bridge_system_statement_t-phase-outside-scratch-refused', ['host', 'bridge_system_statement_t-phase-outside-scratch-refused'],
     r"module 'game:wrong@0\.1\.0' did not load: add_system: a phase lies outside the scratch arena.*alive", None),
    ('bridge_system_statement_t-name-outside-scratch-refused', ['host', 'bridge_system_statement_t-name-outside-scratch-refused'],
     r"module 'game:wrong@0\.1\.0' did not load: system: a name, a term or a change lies outside the scratch arena.*alive", None),
    ('add_system-more-changes-than-types-refused', ['host', 'add_system-more-changes-than-types-refused'],
     r"module 'game:wrong@0\.1\.0' did not load: a system states 5 changes, and the module declared 3 types.*alive", None),
    ('add_system-name-twice-refused', ['host', 'add_system-name-twice-refused'],
     r"module 'game:wrong@0\.1\.0' did not load: 'act' is registered twice.*alive", None),
    ('observe-no-term-refused', ['host', 'observe-no-term-refused'],
     r"module 'game:wrong@0\.1\.0' did not load: observer 'again' has no term, and so nothing to observe.*alive", None),
    ('observe-statement-outside-scratch-refused', ['host', 'observe-statement-outside-scratch-refused'],
     r"module 'game:wrong@0\.1\.0' did not load: observe: what it states lies outside the scratch arena.*alive", None),
    ('bridge_observer_statement_t-events-unset-refused', ['host', 'bridge_observer_statement_t-events-unset-refused'],
     r"module 'game:wrong@0\.1\.0' did not load: observe: its events are unset, or name one there is not.*alive", None),
    ('bridge_term_t-access-unset-refused', ['host', 'bridge_term_t-access-unset-refused'],
     r"module 'game:wrong@0\.1\.0' did not load: term 0 of system 'again': its access is unset.*alive", None),
    ('bridge_term_t-access-neither-refused', ['host', 'bridge_term_t-access-neither-refused'],
     r"module 'game:wrong@0\.1\.0' did not load: term 0 of system 'again': its access is 3, and there is read and write.*alive", None),
    ('bridge_term_t-operator-unset-refused', ['host', 'bridge_term_t-operator-unset-refused'],
     r"module 'game:wrong@0\.1\.0' did not load: term 0 of system 'again': its operator is unset.*alive", None),
    ('bridge_term_t-slot-undeclared-refused', ['host', 'bridge_term_t-slot-undeclared-refused'],
     r"module 'game:wrong@0\.1\.0' did not load: term 0 of system 'again' names slot 9, which it never declared.*alive", None),
    ('BRIDGE_WRITE-ungranted-refused', ['host', 'BRIDGE_WRITE-ungranted-refused'],
     r"module 'game:wrong@0\.1\.0' did not load: system 'again' may not write 'other:Thing': no grant says so.*alive", None),
    ('add_system-change-of-an-undeclared-slot-refused', ['host', 'add_system-change-of-an-undeclared-slot-refused'],
     r"module 'game:wrong@0\.1\.0' did not load: change 0 of system 'again' names slot 9, which it never declared.*alive", None),
    ('add_system-change-ungranted-refused', ['host', 'add_system-change-ungranted-refused'],
     r"module 'game:wrong@0\.1\.0' did not load: system 'again' may not attach or detach 'other:Thing': no grant says so.*alive", None),
    ('alloc-stated-as-none-refused', ['host', 'alloc-stated-as-none-refused'],
     r"module 'game:wrong@0\.1\.0' did not load: alloc: it stated that it keeps nothing in the persistent arena.*alive", None),
    ('declare-outside-init-refused', ['host', 'declare-outside-init-refused'],
     r"system 'act' failed: declare is called outside init.*alive", None),
    ('add_system-outside-init-refused', ['host', 'add_system-outside-init-refused'],
     r"system 'act' failed: add_system is called outside init.*alive", None),
    ('observe-outside-init-refused', ['host', 'observe-outside-init-refused'],
     r"system 'act' failed: observe is called outside init.*alive", None),
    ('attach-undeclared-slot-refused', ['host', 'attach-undeclared-slot-refused'],
     r"system 'act' failed: attach names slot 9, which it never declared.*alive", None),
    ('attach-ungranted-refused', ['host', 'attach-ungranted-refused'],
     r"system 'act' failed: it may not attach 'other:Thing': no grant says so.*alive", None),
    ('attach-value-outside-scratch-refused', ['host', 'attach-value-outside-scratch-refused'],
     r"system 'act' failed: attach: the value lies outside the scratch arena.*alive", None),
    ('detach-undeclared-slot-refused', ['host', 'detach-undeclared-slot-refused'],
     r"system 'act' failed: detach names slot 9, which it never declared.*alive", None),
    ('BACKEND_ENTRY_COUNT-an-entry-missing-refused', ['host', 'BACKEND_ENTRY_COUNT-an-entry-missing-refused'],
     r"backend 'native': backend library '[^']*bridge_hollow[^']*' has no entry 'backend_create'.*bridge_create refused.*alive", None),
    ('memory_adopt-refused-refuses-the-deck', ['host', 'memory_adopt-refused-refuses-the-deck'],
     r"backend 'native' cannot adopt the shared memory: this backend adopts no block.*alive", None),
    ('module_config.spans-outgrown-at-a-barrier', ['host', 'module_config.spans-outgrown-at-a-barrier'],
     r"module 'game:mover@0\.1\.0' is stopped: what its queries match no longer fits in its spans of 544 bytes.*alive", None),
    ('bridge_header_t-read-by-a-native-module', ['storage', 'stress1'],
     '@tests/storage_stress.txt', None),
    ('bridge_entity_t-index-read-by-a-native-module', ['storage', 'stress1'],
     '@tests/storage_stress.txt', None),
    ('bridge_list_t-triggers-of-a-native-observer', ['storage', 'changes'],
     '@tests/storage_changes.txt', None),
    ('detach-queued-and-applied-at-the-barrier', ['storage', 'changes'],
     '@tests/storage_changes.txt', None),
    ('backend_sig_t-what-each-entry-takes-and-answers', ['host', 'backend_sig_t-what-each-entry-takes-and-answers'],
     r"game:mover@0.1.0: loaded.*moved.*alive", 'not loaded'),
    ('backend_entry_t-what-answers-an-import', ['host', 'backend_entry_t-what-answers-an-import'],
     r"game:mover@0.1.0: loaded.*moved.*alive", 'not loaded'),
    ('backend_memory_import_t-both-memories-handed', ['host', 'backend_memory_import_t-both-memories-handed'],
     r"game:mover@0.1.0: loaded.*moved.*alive", 'not loaded'),
    ('memory_get_shape-the-block-reserved-as-asked', ['storage', 'shape'],
     r"the shared block reserves more than it uses: yes.*a native module's private block reserves what it uses: yes.*the shared block is whole wasm pages: yes.*a native module's private block is not rounded to one: yes.*alive", None),
    ('memory_release-after-the-last-instance', ['host', 'memory_release-after-the-last-instance'],
     r"game:mover@0.1.0: loaded.*moved.*alive", 'not loaded'),
    ('instance_create-with-both-memories', ['host', 'instance_create-with-both-memories'],
     r"game:mover@0.1.0: loaded.*moved.*alive", 'not loaded'),
    ('define_func-called-by-a-module', ['host', 'define_func-called-by-a-module'],
     r"game:mover@0.1.0: loaded.*moved.*alive", 'not loaded'),
    ('call-a-call-that-was-ended-answered', ['host', 'call-a-call-that-was-ended-answered'],
     r"its init trapped: the module asked for it.*test:oob@0.1.0: not loaded.*alive", None),
    ('BRIDGE_ERR-nothing-above-an-error-written', ['host', 'BRIDGE_ERR-nothing-above-an-error-written'],
     r"record.3.: an error\s+alive", r"a warning"),
    ('BRIDGE_WARN-a-warning-and-what-is-worse', ['host', 'BRIDGE_WARN-a-warning-and-what-is-worse'],
     r"an error.*a warning\s+alive", r"some information"),
    ('BRIDGE_INFO-information-and-what-is-worse', ['host', 'BRIDGE_INFO-information-and-what-is-worse'],
     r"an error.*a warning.*some information\s+alive", r"a detail"),
    ('BRIDGE_DEBUG-every-line-written', ['host', 'BRIDGE_DEBUG-every-line-written'],
     r"an error.*a warning.*some information.*a detail.*alive", None),
    ('BRIDGE_LOG_ERR-written-as-severity-3', ['host', 'BRIDGE_LOG_ERR-written-as-severity-3'],
     r"record.3.: an error.*alive", None),
    ('BRIDGE_LOG_WARN-written-as-severity-4', ['host', 'BRIDGE_LOG_WARN-written-as-severity-4'],
     r"record.4.: a warning.*alive", None),
    ('BRIDGE_LOG_INFO-written-as-severity-6', ['host', 'BRIDGE_LOG_INFO-written-as-severity-6'],
     r"record.6.: some information.*alive", None),
    ('BRIDGE_LOG_DEBUG-written-as-severity-7', ['host', 'BRIDGE_LOG_DEBUG-written-as-severity-7'],
     r"record.7.: a detail.*alive", None),
    ('bridge_list_modules-fills-no-more-than-asked', ['host', 'bridge_list_modules-fills-no-more-than-asked'],
     r"2 loaded, and of two rows: test:ok@0\.1\.0, untouched\s+alive", None),
    ('BACKEND_MEMORY_LIMIT-lacking-memory-limit-refused', ['host', 'BACKEND_MEMORY_LIMIT-lacking-memory-limit-refused'],
     r"module 'game:mover@0\.1\.0' is refused: it states memory_limit, which backend 'native' cannot keep.*alive", r"cannot open"),
    ('bridge_value_t-a-value-of-each-kind', ['host', 'bridge_value_t-a-value-of-each-kind'],
     r"vsync width 1 1280.*scale mode 150 2.*show: hello 7 0.*show: some/long/path.cfg 0 0.*flags ratio 3 3.*show: big 2 0.*alive", None),
    ('setting.v-seeds-the-field-its-key-names', ['host', 'setting.v-seeds-the-field-its-key-names'],
     r"vsync width 1 1280.*alive", None),
    ('backend.settings-none-where-one-is-needed-refused',
     ['host', 'backend.settings-none-where-one-is-needed-refused'],
     r"it was not told its 'capabilities', and the shape of a block it asks for.*bridge_create refused.*alive", None),
    ('backend.settings-handed-to-the-backend', ['host', 'backend.settings-handed-to-the-backend'],
     r"backend 'native' would not start: it knows no setting 'cache'.*bridge_create refused.*alive", None),
    ('config.backends-two-in-one-phase', ['host', 'config.backends-two-in-one-phase'],
     r"view:reader@0.1.0/sum: sum 250500 0.*native tally 2 1000.*sum 251000 0.*alive", None),
    ('artifact.len-as-many-bytes-compiled', ['host', 'artifact.len-as-many-bytes-compiled'],
     r"test:ok@0.1.0: loaded.*alive", None),
    ('record.identity-whose-it-is', ['host', 'record.identity-whose-it-is'],
     r"view:reader@0\.1\.0/sum: sum 250500 0.*alive", None),
    ('record.system-which-system-said-it', ['host', 'record.system-which-system-said-it'],
     r"game:mover@0\.1\.0/count: moved.*view:reader@0\.1\.0/sum: saw.*alive", None),
    ('record.text-what-was-said', ['host', 'record.text-what-was-said'],
     r"/sum: sum 250500 0.*alive", None),
    ('record.len-as-long-as-the-text', ['host', 'record.len-as-long-as-the-text'],
     r"/say: kept in the arena 0 0\s+alive", None),
    ('record.when-the-barrier-it-was-said-after', ['host', 'record.when-the-barrier-it-was-said-after'],
     r"b1 \S+ event 3 1.*b3 \S+ event 2 2.*b5 \S+ event 4 57005.*alive", None),
    ('record.arg-the-two-numbers', ['host', 'record.arg-the-two-numbers'],
     r"moved 2 1000.*saw 1 500.*alive", None),
    ('module_info.identity-of-each-that-is-loaded', ['host', 'module_info.identity-of-each-that-is-loaded'],
     r"modules: game:twostep@0\.1\.0=RUNNING test:flaky@0\.1\.0=STOPPED .2..*alive", None),
    ('module_info.state-running-and-stopped', ['host', 'module_info.state-running-and-stopped'],
     r"modules: game:twostep@0\.1\.0=RUNNING test:flaky@0\.1\.0=STOPPED .2..*alive", None),
    ('bridge_list_modules-what-is-loaded-and-whether-it-runs', ['host', 'bridge_list_modules-what-is-loaded-and-whether-it-runs'],
     r"modules: game:twostep@0\.1\.0=RUNNING test:flaky@0\.1\.0=STOPPED .2..*alive", None),
    ('bridge_trigger_t-who-triggered-and-how', ['host', 'bridge_trigger_t-who-triggered-and-how'],
     r"any: event 3 1.*any: event 2 2.*any: event 4 57005.*alive", None),
    ('bridge_query_t-found-by-a-native-system', ['host', 'bridge_query_t-found-by-a-native-system'],
     r"game:mover@0.1.0/tally: native tally 2 1000.*native tally 2 1000.*alive", None),
    ('bridge_text_t-a-string-a-component-keeps', ['host', 'bridge_text_t-a-string-a-component-keeps'],
     r"game:namer@0.1.0/say: kept in the arena.*alive", None),
    ('bridge_log_record_t-written-by-a-module-and-drained', ['host', 'bridge_log_record_t-written-by-a-module-and-drained'],
     r"sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive", None),
    ('BRIDGE_SPAN_SIZE-what-a-span-takes-of-the-spans', ['host', 'BRIDGE_SPAN_SIZE-what-a-span-takes-of-the-spans'],
     r"module 'view:reader@0.1.0' did not load: its spans of 16 bytes do not hold a column_base of 2 components by \d+ tables.*moved 2 1000.*alive", None),
    ('BRIDGE_SCHEMA_TEXT-a-descriptor-of-the-type-s-own-text', ['host', 'BRIDGE_SCHEMA_TEXT-a-descriptor-of-the-type-s-own-text'],
     r"sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive", None),
    ('BRIDGE_SCHEMA_STRUCT_TYPE-laid-out-as-its-descriptor', ['host', 'BRIDGE_SCHEMA_STRUCT_TYPE-laid-out-as-its-descriptor'],
     r"sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive", None),
    ('BRIDGE_SCHEMA_STRUCT_DESC-read-and-agreed-with', ['host', 'BRIDGE_SCHEMA_STRUCT_DESC-read-and-agreed-with'],
     r"sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive", None),
    ('BRIDGE_SCHEMA_ENUM_TYPE-of-the-integer-stated', ['host', 'BRIDGE_SCHEMA_ENUM_TYPE-of-the-integer-stated'],
     r"read: body 2 2.*read: more 3 4.*alive", None),
    ('BRIDGE_SCHEMA_ENUM_DESC-read-and-agreed-with', ['host', 'BRIDGE_SCHEMA_ENUM_DESC-read-and-agreed-with'],
     r"read: body 2 2.*read: more 3 4.*alive", None),
    ('BRIDGE_SCHEMA_BITMASK_TYPE-of-the-integer-stated', ['host', 'BRIDGE_SCHEMA_BITMASK_TYPE-of-the-integer-stated'],
     r"read: body 2 2.*read: more 3 4.*alive", None),
    ('BRIDGE_SCHEMA_BITMASK_DESC-read-and-agreed-with', ['host', 'BRIDGE_SCHEMA_BITMASK_DESC-read-and-agreed-with'],
     r"read: body 2 2.*read: more 3 4.*alive", None),
    ('backend_err_t-why-an-entry-failed', ['host', 'backend_err_t-why-an-entry-failed'],
     r"backend 'native' would not start: it knows no setting 'cache'.*bridge_create refused.*alive", None),
    ('backend_value_t-handed-over-unread', ['host', 'backend_value_t-handed-over-unread'],
     r"backend 'native' would not start: it knows no setting 'cache'.*bridge_create refused.*alive", None),
    ('backend_setting_t-a-key-the-backend-refuses', ['host', 'backend_setting_t-a-key-the-backend-refuses'],
     r"backend 'native' would not start: it knows no setting 'cache'.*bridge_create refused.*alive", None),
    ('backend_import_t-the-two-memories-found', ['host', 'backend_import_t-the-two-memories-found'],
     r"sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive", None),
    ('backend_source_t-no-bytes-refused', ['host', 'backend_source_t-no-bytes-refused'],
     r"module 'view:reader@0.1.0' did not load: the tests' backend opens a module's library by its path, and was handed neither.*alive", None),
    ('backend_destroy-the-library-leaves-the-process', ['host', 'backend_destroy-the-library-leaves-the-process'],
     r"the wasm backend's library is still loaded, the native backend's is gone.*alive", None),
    ('backend_get_capabilities-held-against-what-a-module-states', ['host', 'backend_get_capabilities-held-against-what-a-module-states'],
     r"module 'game:mover@0.1.0' is refused: it is not trusted, and backend 'native' runs only modules that are.*alive", r"cannot open"),
    ('module_compile-no-path-refused', ['host', 'module_compile-no-path-refused'],
     r"module 'game:mover@0.1.0' did not load: the tests' backend opens a module's library by its path, and was handed neither.*alive", None),
    ('module_list_imports-the-two-memories-lent', ['host', 'module_list_imports-the-two-memories-lent'],
     r"sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive", None),
    ('module_destroy-a-failed-load-leaves-nothing', ['host', 'module_destroy-a-failed-load-leaves-nothing'],
     r"game:badterm@0.1.0: not loaded.*game:fp@0.1.0: loaded.*alive", None),
    ('instance_destroy-an-unload-gives-its-share-back', ['host', 'instance_destroy-an-unload-gives-its-share-back'],
     r"game:namer@0.1.0/say: kept in the arena.*game:namer@0.1.0: unloaded.*game:ticker@0.1.0: loaded.*alive", None),
    ('instance_set_work_limit-a-spinning-system-ended', ['host', 'instance_set_work_limit-a-spinning-system-ended'],
     r"test:spin@0.1.0/forever: system 'forever' failed: trapped: the work limit is used up.*still here.*the work limit is used up.*still here.*alive", None),
    ('instance_set_timeout-a-spinning-init-interrupted', ['host', 'instance_set_timeout-a-spinning-init-interrupted'],
     r"module 'test:spin@0.1.0' did not load: its init trapped: the timeout passed.*alive", None),
    ('instance_set_memory_limit-growth-refused', ['host', 'instance_set_memory_limit-growth-refused'],
     r"module 'test:grow@0.1.0' did not load: its init answered 7.*alive", None),
    ('backend.settings-each-kind-read-by-the-backend', ['host', 'backend.settings-each-kind-read-by-the-backend'],
     r"backend 'native' would not start: it was handed cached=true workers=7 ratio=1\.5 where='somewhere'.*alive", None),
    ('BACKEND_BOOL-read-as-it-was-stated', ['host', 'BACKEND_BOOL-read-as-it-was-stated'],
     r"it was handed cached=true.*alive", None),
    ('BACKEND_INT-read-as-it-was-stated', ['host', 'BACKEND_INT-read-as-it-was-stated'],
     r"it was handed.* workers=7.*alive", None),
    ('BACKEND_FLOAT-read-as-it-was-stated', ['host', 'BACKEND_FLOAT-read-as-it-was-stated'],
     r"it was handed.* ratio=1\.5.*alive", None),
    ('BACKEND_STRING-read-as-it-was-stated', ['host', 'BACKEND_STRING-read-as-it-was-stated'],
     r"it was handed.* where='somewhere'.*alive", None),
    ('BACKEND_IMPORT_OTHER-a-global-nobody-answers', ['host', 'BACKEND_IMPORT_OTHER-a-global-nobody-answers'],
     r"module 'test:stray@0\.1\.0' did not load: nothing answers its import 'elsewhere::number'.*alive", None),
    ('BRIDGE_BOOL-seeds-a-bool', ['host', 'BRIDGE_BOOL-seeds-a-bool'],
     r"vsync width 1 1280.*alive", None),
    ('BRIDGE_FLOAT-seeds-a-float', ['host', 'BRIDGE_FLOAT-seeds-a-float'],
     r"scale mode 150 2.*alive", None),
    ('BRIDGE_TRUE-a-module-pinned', ['host', 'BRIDGE_TRUE-a-module-pinned'],
     r"the others used [2-4] thread.s.[^a-z]*pinned: 9 of its 9 calls were on the thread that loaded it.*alive", None),
    ('BRIDGE_FALSE-a-reload-forbidden', ['host', 'BRIDGE_FALSE-a-reload-forbidden'],
     r"is not reloaded: its hot_reload is false.*tick 104 1.*alive", None),
    ('BRIDGE_RUNNING-a-module-that-still-runs', ['host', 'BRIDGE_RUNNING-a-module-that-still-runs'],
     r"game:twostep@0\.1\.0=RUNNING.*alive", None),
    ('BRIDGE_STOPPED-a-module-at-its-failures', ['host', 'BRIDGE_STOPPED-a-module-at-its-failures'],
     r"test:flaky@0\.1\.0=STOPPED.*alive", None),
    ('BRIDGE_HOST_ABI-stated-and-created', ['host', 'BRIDGE_HOST_ABI-stated-and-created'],
     r"test:ok@0.1.0: loaded.*alive", None),
    ('BRIDGE_LEVEL_MOST-every-line-written', ['host', 'BRIDGE_LEVEL_MOST-every-line-written'],
     r"an error.*a warning.*some information.*a detail.*alive", None),
    ('BRIDGE_STRUCT-a-struct-a-field-names', ['host', 'BRIDGE_STRUCT-a-struct-a-field-names'],
     r"read: body 2 2.*read: more 3 4.*alive", None),
    ('BRIDGE_ENUM-an-enum-a-field-names', ['host', 'BRIDGE_ENUM-an-enum-a-field-names'],
     r"read: body 2 2.*read: more 3 4.*alive", None),
    ('BRIDGE_BITMASK-a-bitmask-a-field-names', ['host', 'BRIDGE_BITMASK-a-bitmask-a-field-names'],
     r"read: body 2 2.*read: more 3 4.*alive", None),
    ('BRIDGE_TABLE-walked-by-its-rows', ['host', 'BRIDGE_TABLE-walked-by-its-rows'],
     r"sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive", None),
    ('BRIDGE_READ-a-term-that-reads', ['host', 'BRIDGE_READ-a-term-that-reads'],
     r"sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive", None),
    ('BRIDGE_AND-only-the-entities-that-carry-both', ['host', 'BRIDGE_AND-only-the-entities-that-carry-both'],
     r"moved 2 1000.*saw 1 500.*alive", None),
    ('BRIDGE_OPTIONAL-a-term-an-entity-may-lack', ['host', 'BRIDGE_OPTIONAL-a-term-an-entity-may-lack'],
     r"game:aim@0.1.0/reached: aims 10 5.*game:aim@0.1.0/reached: aimed 20 0.*alive", r"did not load"),
    ('BRIDGE_ON_ENTER-told-with-the-attach-that-made-it', ['host', 'BRIDGE_ON_ENTER-told-with-the-attach-that-made-it'],
     r"event 3 1.*alive", None),
    ('BRIDGE_ON_ATTACH-told-alone-of-an-attach-again', ['host', 'BRIDGE_ON_ATTACH-told-alone-of-an-attach-again'],
     r"event 2 2.*alive", None),
    ('BRIDGE_ON_LEAVE-told-of-what-was-destroyed', ['host', 'BRIDGE_ON_LEAVE-told-of-what-was-destroyed'],
     r"event 4 57005.*alive", None),
    ('BRIDGE_ABI-stated-and-loaded', ['host', 'BRIDGE_ABI-stated-and-loaded'],
     r"test:ok@0.1.0: loaded.*alive", None),
    ('BRIDGE_NAMESPACE-where-every-import-is', ['host', 'BRIDGE_NAMESPACE-where-every-import-is'],
     r"sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive", None),
    ('BRIDGE_SHARED-the-memory-two-modules-read', ['host', 'BRIDGE_SHARED-the-memory-two-modules-read'],
     r"sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive", None),
    ('BRIDGE_PRIVATE-the-memory-of-one-module', ['host', 'BRIDGE_PRIVATE-the-memory-of-one-module'],
     r"sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive", None),
    ('BRIDGE_CARRIED-where-a-tag-would-have-a-value', ['host', 'BRIDGE_CARRIED-where-a-tag-would-have-a-value'],
     r"frozen.*alive", None),
    ('BRIDGE_ZEROED-attached-with-no-value', ['host', 'BRIDGE_ZEROED-attached-with-no-value'],
     r"frozen.*alive", None),
    ('BRIDGE_SPAN_BASE-where-a-span-keeps-its-columns', ['host', 'BRIDGE_SPAN_BASE-where-a-span-keeps-its-columns'],
     r"sum 250500 0.*alive", None),
    ('BACKEND_RELOAD-a-module-replaced-while-the-deck-runs', ['host', 'BACKEND_RELOAD-a-module-replaced-while-the-deck-runs'],
     r"tick 102 1.*game:ticker@0.1.0/-: shutdown 1 0.*game:ticker@0.2.0: reloaded.*tick 112 10.*tick 122 10.*alive", None),
    ('BACKEND_IMPORT_FUNC-an-entry-imported-is-no-memory', ['host', 'BACKEND_IMPORT_FUNC-an-entry-imported-is-no-memory'],
     r"sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive", None),
    ('BACKEND_IMPORT_MEMORY-the-two-memories-lent', ['host', 'BACKEND_IMPORT_MEMORY-the-two-memories-lent'],
     r"sum 250500 0.*moved 2 1000.*sum 251000 0.*sum 251500 0.*alive", None),
    ('BACKEND_ABI-answered-and-the-library-opened', ['host', 'BACKEND_ABI-answered-and-the-library-opened'],
     r"view:reader@0.1.0/sum: sum 250500 0.*native tally 2 1000.*sum 251000 0.*alive", None),
    ('BACKEND_I32-an-argument-of-32-bits', ['host', 'BACKEND_I32-an-argument-of-32-bits'],
     r"game:mover@0.1.0: loaded.*moved.*alive", 'not loaded'),
    ('BACKEND_I64-an-argument-of-64-bits', ['host', 'BACKEND_I64-an-argument-of-64-bits'],
     r"game:mover@0.1.0: loaded.*moved.*alive", 'not loaded'),
    ('config.backends-none-refused', ['host', 'config.backends-none-refused'],
     r"the deployment configures no backend, and a module is run by one.*bridge_create refused.*alive", None),
    ('attach-tag-with-a-value-refused', ['host', 'attach-tag-with-a-value-refused'],
     r"system 'act' failed: attach: 'game:Flag' is a tag, and is handed a value.*alive", None),
    ('BRIDGE_FAIL-refused-before-it-exists-stops-nothing', ['host', 'BRIDGE_FAIL-refused-before-it-exists-stops-nothing'],
     r"module 'test:ok@0\.1\.0' is refused: no backend named 'native'.*test:image@0\.1\.0: loaded.*alive", r"has stopped"),
]

# these again, through the static library: what a host does first, to create a deck and to
# load a module
STATIC = (
    'bridge_load-loaded',
    'BRIDGE_NO_LIMIT-stated-for-every-limit',
    'bridge_load-init-trapped',
    'module_config.init_timeout-init-interrupted',
    'module_config.memory_limit-growth-refused',
    'instance_get_func-no-such-export-refused',
    'bridge_load-no-init-export',
    'module_config.abi-another-refused',
    'artifact.backend-not-configured-refused',
    'module_config.log_level-unset-refused',
    'module_config.memory_limit-unset-refused',
    'BRIDGE_NONE-for-a-limit-refused',
    'config.abi-another-refused',
    'config.abi-unset-refused',
    'BRIDGE_UNSET-flag-refused',
    'module_config.memory_limit-negative-refused',
    'module_config.on_error-fail-stops-and-skip-does-not',
    'backend.library-cannot-open-refused',
    'bridge_create-no-sink-refused',
)
CASES += [('static-' + name, ['host-static', name], match, never)
          for name, argv, match, never in CASES if name in STATIC]
# The cases that are started once whatever --copies says, each with why. Under load a case that
# measures gives another number, and that is no defect; a case that only bounds a time stays in
# the load, where a bound that load breaks is found.
MEASURES = 'it measures, and under load it measures the load'
CLOCKED = 'it decides by the clock whether two naps overlapped'
TREE = 'it reads the tree, and every copy would read the same'
ALONE = {
    'storage-weigh': MEASURES, 'storage-baseline': MEASURES, 'storage-measure': MEASURES,
    'config.threads-two-run-at-once': CLOCKED, 'config.threads-one-runs-in-turn': CLOCKED,
    'c23': TREE, 'style': TREE, 'deps': TREE, 'cover': TREE, 'xref': TREE, 'licence': TREE,
    'package': TREE,
    **{name: TREE for name in ('cla-the-holder-is-not-asked', 'cla-a-name-on-record-passes',
                               'cla-anybody-else-is-asked-and-fails',
                               'cla-the-sentence-by-its-author-is-recorded',
                               'cla-the-sentence-by-another-is-not-taken',
                               'cla-other-words-are-not-taken')},
    'said': 'it reads what the cases of this run said',
}

# and last, what is held against every case before it: the messages of the source against what
# the cases said, and docs/tests.md against the headers and the cases
CASES += [('said', ['said'], None, None), ('cover', ['cover'], None, None),
          ('xref', ['xref'], None, None), ('licence', ['licence'], None, None)]

# what a host is handed: that each package is what the document says, and that a host and a
# module of each backend built from the packages alone run - host_api.md, *What a host is handed*
PACKED = r'packed:module@0\.1\.0: count 3.*destroyed: 0'
CASES += [('package', ['package'], None, None),
          ('package-a-host-a-backend-and-a-module-built-from-them', ['packaged/host'], PACKED, r'refused'),
          ('package-the-same-host-with-the-static-library', ['packaged/host-static'], PACKED,
           r'refused')]

# who is asked to sign the agreement of contributors, decided as the workflow decides it
SIGNS = 'I have read cla.md and I sign it.'
CASES += [
    ('cla-the-holder-is-not-asked', ['cla', 'holder', 'holder', ''], r'^nothing, passes: the holder', None),
    ('cla-a-name-on-record-passes', ['cla', 'ann', 'holder', 'bob,ann'], r'^nothing, passes: ann has signed', None),
    ('cla-anybody-else-is-asked-and-fails', ['cla', 'ann', 'holder', 'bob'], r'^ask, fails: ann has not signed', None),
    ('cla-the-sentence-by-its-author-is-recorded', ['cla', 'ann', 'holder', '', 'ann', SIGNS],
     r'^record, passes: ann signs', None),
    ('cla-the-sentence-by-another-is-not-taken', ['cla', 'ann', 'holder', '', 'bob', SIGNS],
     r'^ask, fails', None),
    ('cla-other-words-are-not-taken', ['cla', 'ann', 'holder', '', 'ann', 'I sign, more or less.'],
     r'^ask, fails', None),
]


if __name__ == '__main__':
    main()
