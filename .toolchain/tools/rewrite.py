# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import concurrent.futures
import json
import os
import re
import shutil
import subprocess
from pathlib import Path

import build
from shared import die, expanded

TOPS = ('src/', 'include/')
OPENS = ('if', 'ifdef', 'ifndef')
DIRECTIVE = re.compile(rb'[ \t]*#[ \t]*(\w+)')
NULLABILITY = re.compile(r'\b_(?:Nonnull|Nullable_result|Nullable|Null_unspecified)\b'
                         r'|__attribute__\(\((?:cdecl|stdcall|fastcall|thiscall)\)\)')
RENAME = '''[[maybe_unused]] static typeof(%(n)s) %(p)s%(n)s;
#pragma push_macro("%(n)s")
#undef %(n)s
#define %(n)s %(p)s%(n)s
'''
WRITER = r'''#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif
[[gnu::destructor]] static void %(p)swrite(void)
{
    static char %(p)sline[%(line)d];
    const char *%(p)spath = getenv("%(variable)s");
    unsigned %(p)sat = %(name_len)du;
    int %(p)sfd;
    int %(p)sn;

    if (!%(p)spath)
        return;
#ifdef _WIN32
    %(p)sfd = _open(%(p)spath, _O_WRONLY | _O_CREAT | _O_APPEND | _O_BINARY, _S_IREAD | _S_IWRITE);
#else
    %(p)sfd = open(%(p)spath, O_WRONLY | O_CREAT | O_APPEND, 0644);
#endif
    if (%(p)sfd < 0)
        return;
    memcpy(%(p)sline, "%(name)s ", %(p)sat);
%(fill)s    %(p)sline[%(p)sat++] = '\n';
#ifdef _WIN32
    %(p)sn = _write(%(p)sfd, %(p)sline, %(p)sat);
    _close(%(p)sfd);
#else
    %(p)sn = (int)write(%(p)sfd, %(p)sline, %(p)sat);
    close(%(p)sfd);
#endif
    (void)%(p)sn;
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
'''


def tree(values, run):
    return Path(values['build']) / run / 'tree'


def marked(node, state):
    def mark(loc):
        if not isinstance(loc, dict) or not loc:
            return
        for side in ('spellingLoc', 'expansionLoc'):
            mark(loc.get(side))
        if 'file' in loc:
            state[0] = loc['file']
        loc['_file'] = state[0]
    mark(node.get('loc'))
    mark(node.get('range', {}).get('begin'))
    mark(node.get('range', {}).get('end'))
    for child in node.get('inner', []):
        if isinstance(child, dict):
            marked(child, state)


def is_macro(loc):
    return isinstance(loc, dict) and 'expansionLoc' in loc


def owned(loc, root):
    path = (loc.get('_file') or '').replace('\\', '/')
    whole = root.as_posix() + '/'
    if path.lower().startswith(whole.lower()):
        path = path[len(whole):]
    while path.startswith('./'):
        path = path[2:]
    return path if path.startswith(TOPS) else None


def dumped(command, root):
    done = subprocess.run(command, cwd=root, capture_output=True)
    errors = [line for line in done.stderr.decode('utf-8', 'replace').splitlines()
              if ' error: ' in line and not line.endswith('FileNotFound')]
    if errors:
        die('the tree of %s cannot be read:\n%s' % (command[-1], '\n'.join(errors)))
    try:
        found = json.loads(done.stdout)
    except ValueError:
        die('the tree of %s is not JSON' % command[-1])
    marked(found, [None])
    return found


def sources(config, values):
    jobs = {}
    for artifact in build.artifacts(config):
        if 'target' in artifact:
            continue
        _, mine = build.planned(config, values, artifact)
        Path(mine['work']).parent.mkdir(parents=True, exist_ok=True)
        for source in mine['sources']:
            if source.startswith('src/') and source.endswith('.c') and source not in jobs:
                jobs[source] = expanded(config['rewrite']['dump'], dict(mine, source=source))
    return jobs


def each(config, values, use):
    root = Path(values['root'])
    jobs = sources(config, values)
    with concurrent.futures.ThreadPoolExecutor(min(8, os.cpu_count() or 1)) as pool:
        return list(pool.map(lambda job: use(root, job[0], dumped(job[1], root)), jobs.items()))


def copied(root, where):
    shutil.rmtree(where, ignore_errors=True)
    for top in ('src', 'include'):
        if (root / top).is_dir():
            shutil.copytree(root / top, where / top)


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


def found(root, path, tree, names, callbacks=()):
    used, defined, declared, ends, typedefs = set(), set(), {}, {}, {}

    def walk(node):
        kind, name = node.get('kind'), node.get('name')
        if kind == 'DeclRefExpr':
            name = node.get('referencedDecl', {}).get('name')
            if name in names and owned(node.get('range', {}).get('begin', {}), root) == path:
                used.add(name)
        elif kind == 'TypedefDecl':
            typedefs.setdefault(name, plain(node.get('type', {})))
        elif kind == 'FunctionDecl' and name in names and not node.get('isImplicit'):
            declared.setdefault(name, declaration(node, typedefs))
            body = any(isinstance(one, dict) and one.get('kind') == 'CompoundStmt' for one in node.get('inner', []))
            if body and owned(node.get('loc', {}), root) == path:
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
    result = '((typeof(%s) *)nullptr)(%s)' % (name, ', '.join('(%s){}' % one for one in parameters))
    return 'static typeof(%s) %s%s(%s)\n{\n' % (result, prefix, name, arguments), call


def placing(text, at):
    return text.count(b'\n', 0, at) + 1, at - (text.rfind(b'\n', 0, at) + 1) + 1


def writer(values):
    return WRITER % dict(values, fill=values['fill'] % values)
