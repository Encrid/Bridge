# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import concurrent.futures
import json
import re
import shutil
import subprocess
import tempfile
from pathlib import Path

import build
from shared import HERE, die, expanded

TOPS = ('src/', 'include/')
RUNTIME = (HERE / '.toolchain' / 'runtime').as_posix()
TESTED = ('program', 'library', 'archive')
KEPT = TOPS + ('tests/',)
DECLARED = ('FunctionDecl', 'VarDecl', 'TypedefDecl')
SLIM = ('id', 'kind', 'name', 'type', 'isImplicit', 'loc', 'range')
CHUNK = 1 << 24
OPENS = ('if', 'ifdef', 'ifndef')
BETWEEN = re.compile(r'[\s,]*')
DIRECTIVE = re.compile(rb'[ \t]*#[ \t]*(\w+)')
POINTER = re.compile(r'(.+?)\(\s*\*\s*\)\s*\((.*)\)')
NULLABILITY = re.compile(r'\b_(?:Nonnull|Nullable_result|Nullable|Null_unspecified)\b'
                         r'|__attribute__\(\((?:cdecl|stdcall|fastcall|thiscall)\)\)')
RENAME = '''[[maybe_unused]] static typeof(*%(n)s) %(p)s%(n)s;
#pragma push_macro("%(n)s")
#undef %(n)s
#define %(n)s %(p)s%(n)s
'''
WRITER = r'''#ifndef __wasm__
#include "%(runtime)s/counts.h"
[[gnu::destructor]] static void %(p)swrite(void)
{
    static char %(p)sline[C23_COUNTS_ROOM(%(length)du, %(slots)du)];
    long long %(p)scounts[%(slots)d];

    for (unsigned %(p)si = 0; %(p)si < %(slots)du; %(p)si++)
        %(p)scounts[%(p)si] = %(p)s%(kept)s[%(p)si];
    c23_counts_keep(%(p)sline, "%(name)s", %(p)scounts, %(slots)du);
}
#ifdef __APPLE__
typedef struct {
    const char *dli_fname;
    void *dli_fbase;
    const char *dli_sname;
    void *dli_saddr;
} %(p)sinfo_t;
extern int %(p)sdladdr(const void *at, %(p)sinfo_t *info) __asm__("_dladdr");
extern void *%(p)sdlopen(const char *path, int mode) __asm__("_dlopen");
[[gnu::constructor]] static void %(p)skeep(void)
{
    %(p)sinfo_t %(p)sinfo;

    if (%(p)sdladdr((const void *)%(p)s%(kept)s, &%(p)sinfo) && %(p)sinfo.dli_fname)
        (void)%(p)sdlopen(%(p)sinfo.dli_fname, 0x1 | 0x10 | 0x80);
}
#endif
#endif
'''
AREA = r"""[[gnu::weak]] extern unsigned char __start_c23b[];
[[gnu::weak]] extern unsigned char __stop_c23b[];
[[gnu::weak]] extern unsigned char __start_c23l[];
[[gnu::weak]] extern unsigned char __stop_c23l[];
char c23b_plan[256];
struct {
    unsigned version;
    unsigned char *outcomes;
    unsigned char *outcomes_end;
    unsigned char *counts;
    unsigned char *counts_end;
    char *plan;
    unsigned room;
} c23b_outcomes = { 1u, __start_c23b, __stop_c23b, __start_c23l, __stop_c23l, c23b_plan, sizeof(c23b_plan) };
"""


def tree(values, run):
    return Path(values['build']) / run / 'tree'


def located(item, state):
    if 'offset' in item or 'spellingLoc' in item or 'expansionLoc' in item:
        state[0] = item.get('file', state[0])
        item['_file'] = state[0]
    return item


def is_ours(node, root):
    sides = [node.get('loc')] + list(node.get('range', {}).values())
    return any(isinstance(side, dict) and (owned(side, root, KEPT) or owned(side.get('expansionLoc', {}), root, KEPT))
               for side in sides)


def kept(node, root):
    if is_ours(node, root):
        return node
    if node.get('kind') not in DECLARED:
        return None
    slim = {key: node[key] for key in SLIM if key in node}
    slim['inner'] = [{'kind': one['kind'], 'type': one.get('type', {})} for one in node.get('inner', [])
                     if isinstance(one, dict) and one.get('kind') == 'ParmVarDecl']
    return slim


def parsed(path, root):
    state = [None]
    decoder = json.JSONDecoder(object_hook=lambda item: located(item, state))
    with open(path, encoding='utf-8', newline='') as file:
        text = file.read(CHUNK)
        at = text.index('[', text.index('"inner"')) + 1
        top = json.loads(text[:at] + ']}')
        while True:
            at = BETWEEN.match(text, at).end()
            if at < len(text) and text[at] == ']':
                return top
            was = state[0]
            try:
                node, at = decoder.raw_decode(text, at)
            except ValueError:
                state[0] = was
                more = file.read(max(CHUNK, len(text) - at))
                if not more:
                    raise
                text, at = text[at:] + more, 0
                continue
            node = kept(node, root)
            if node:
                top['inner'].append(node)
            if at > CHUNK:
                text, at = text[at:], 0


def is_macro(loc):
    return isinstance(loc, dict) and 'expansionLoc' in loc


def owned(loc, root, tops=TOPS):
    path = (loc.get('_file') or '').replace('\\', '/')
    whole = root.as_posix() + '/'
    if path.lower().startswith(whole.lower()):
        path = path[len(whole):]
    while path.startswith('./'):
        path = path[2:]
    return path if tops is None or path.startswith(tops) else None


def dumped(command, root):
    folder = (Path(root) / command[command.index('-o') + 1]).parent
    folder.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(dir=folder, suffix='.tree', delete=False) as out:
        done = subprocess.run(command, cwd=root, stdout=out, stderr=subprocess.PIPE)
    tree = Path(out.name)
    errors = [line for line in done.stderr.decode('utf-8', 'replace').splitlines()
              if ' error: ' in line and not line.endswith('FileNotFound')]
    try:
        if errors:
            die('the tree of %s cannot be read:\n%s' % (command[-1], '\n'.join(errors)))
        found = parsed(tree, Path(root))
    except ValueError:
        die('the tree of %s is not JSON' % command[-1])
    except MemoryError:
        die('the tree of %s needs more memory than this machine has free' % command[-1])
    finally:
        tree.unlink(missing_ok=True)
    return found


def stood_in(artifact, mine):
    folder = Path(mine['work'] + '.inputs')
    names = [Path(file).name for file in build.read(artifact, mine)]
    for name in names:
        folder.mkdir(parents=True, exist_ok=True)
        (folder / name).write_bytes(b'\0')
    return ['--embed-dir=' + folder.as_posix()] if names else []


def sources(config, values, tests=False):
    jobs, parts = {}, {}
    for artifact in build.artifacts(config):
        if 'target' in artifact:
            continue
        kind, mine = build.planned(config, values, artifact)
        Path(mine['work']).parent.mkdir(parents=True, exist_ok=True)
        mine['inputs'] = stood_in(artifact, mine)
        tops = ('src/', 'tests/') if tests and artifact['kind'] in TESTED else ('src/',)
        for source in mine['sources']:
            if source.startswith(tops) and source.endswith('.c'):
                command = expanded(kind.get('dump', config['rewrite']['dump']), dict(mine, source=source))
                at = command.index('-o')
                key = source, tuple(command[:at] + command[at + 2:])
                jobs.setdefault(key, (command, parts.setdefault(source, set())))
            if source in parts and 'part' in artifact:
                parts[source].add(artifact['part'])
    return jobs


def each(config, values, use, tests=False):
    root = Path(values['root'])
    jobs = sources(config, values, tests)
    with concurrent.futures.ThreadPoolExecutor(values['jobs']) as pool:
        return list(pool.map(lambda job: (use(root, job[0][0], dumped(job[1][0], root)), sorted(job[1][1])),
                             jobs.items()))


def united(files):
    found = {}
    for (path, used, declared, ends), parts in files:
        _, was, known, at, _ = found.get(path, (path, [], {}, {}, parts))
        found[path] = path, sorted(set(was) | set(used)), {**declared, **known}, {**ends, **at}, parts
    return [(one[:4], one[4]) for one in found.values()]


def copied(root, where, more=()):
    shutil.rmtree(where, ignore_errors=True)
    where.mkdir(parents=True)
    (where / 'c23b_area.c').write_text(AREA, encoding='utf-8')
    for top in ('src', 'include'):
        if (root / top).is_dir():
            shutil.copytree(root / top, where / top)
    for folder in sorted({Path(path).parent.as_posix() for path in more if not path.startswith(TOPS)}):
        shutil.copytree(root / folder, where / folder, dirs_exist_ok=True)


def plain(kind):
    return kind.get('desugaredQualType', kind.get('qualType', ''))


def spelled(text):
    return ' '.join(NULLABILITY.sub('', text).split())


def declaration(node, typedefs):
    whole = plain(node.get('type', {}))
    returned = whole.split('(', 1)[0].strip()
    parameters = [one.get('type', {}) for one in node.get('inner', [])
                  if isinstance(one, dict) and one.get('kind') == 'ParmVarDecl']
    return (typedefs.get(returned, returned), [plain(one) for one in parameters], '...' in whole,
            [spelled(one.get('qualType', '')) for one in parameters])


def split_top(text):
    parts, depth, start = [], 0, 0
    for n, char in enumerate(text):
        depth += (char == '(') - (char == ')')
        if char == ',' and not depth:
            parts.append(text[start:n].strip())
            start = n + 1
    parts.append(text[start:].strip())
    return [part for part in parts if part and part != 'void']


def pointed(node):
    match = POINTER.fullmatch(spelled(plain(node.get('type', {}))))
    if not match:
        return None
    parameters = split_top(match.group(2))
    return (match.group(1).strip(), parameters, '...' in parameters, parameters)


def found(root, path, tree, names, callbacks=()):
    used, defined, declared, ends, typedefs = set(), set(), {}, {}, {}

    def walk(node):
        kind, name = node.get('kind'), node.get('name')
        if kind == 'DeclRefExpr':
            referenced = node.get('referencedDecl', {})
            name = referenced.get('name') if referenced.get('kind') in ('FunctionDecl', 'VarDecl') else None
            if referenced.get('kind') == 'VarDecl' and name not in declared:
                name = None
            if name in names and owned(node.get('range', {}).get('begin', {}), root, None) == path:
                used.add(name)
        elif kind == 'TypedefDecl':
            typedefs.setdefault(name, plain(node.get('type', {})))
        elif kind == 'VarDecl' and name in names and pointed(node):
            declared.setdefault(name, pointed(node))
        elif kind == 'FunctionDecl' and name in names and not node.get('isImplicit'):
            declared.setdefault(name, declaration(node, typedefs))
            body = any(isinstance(one, dict) and one.get('kind') == 'CompoundStmt' for one in node.get('inner', []))
            if body and owned(node.get('loc', {}), root, None) == path:
                defined.add(name)
                end = node.get('range', {}).get('end', {})
                ends[name] = (end['expansionLoc'] if is_macro(end) else end).get('offset', 0)
        for one in node.get('inner', []):
            if isinstance(one, dict) and one:
                walk(one)
    walk(tree)
    named = used - (defined - set(callbacks))
    return path, sorted(named), declared, {name: ends[name] for name in named & defined}


def directive(line):
    match = DIRECTIVE.match(line)
    if match:
        return match.group(1).decode('ascii', 'replace')
    return '' if not line.strip() else None


def included(text):
    lines = text.split(b'\n')
    marks = [directive(line) for line in lines]
    end = max((n for n, mark in enumerate(marks) if mark == 'include'), default=-1)
    closes, opened = {}, []
    for n, mark in enumerate(marks):
        if mark in OPENS:
            opened.append(n)
        elif mark == 'endif':
            closes[opened.pop()] = n
    for start in sorted((start for start, close in closes.items() if start < end < close), reverse=True):
        if None in marks[start:closes[start]]:
            break
        end = closes[start]
    return sum(len(line) + 1 for line in lines[:end + 1])


def signed(name, declaration, prefix):
    _, _, variadic, parameters = declaration
    if variadic:
        die('%s takes a list of arguments of no fixed length' % name)
    arguments = ', '.join('typeof(%s) a%d' % (one, n) for n, one in enumerate(parameters)) or 'void'
    call = '(%s)(%s)' % (name, ', '.join('a%d' % n for n in range(len(parameters))))
    result = '((typeof(*%s) *)nullptr)(%s)' % (name, ', '.join('(%s){}' % one for one in parameters))
    return 'static typeof(%s) %s%s(%s)\n{\n' % (result, prefix, name, arguments), call


def placing(text, at):
    return text.count(b'\n', 0, at) + 1, at - (text.rfind(b'\n', 0, at) + 1) + 1


def writer(values):
    return WRITER % dict(values, runtime=RUNTIME)
