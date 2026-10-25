# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import concurrent.futures
import fnmatch
import json
import os
import shutil
import subprocess
from pathlib import Path

import build
from shared import die, expanded

DECIDED = {'IfStmt': 0, 'WhileStmt': 0, 'DoStmt': 1, 'ConditionalOperator': 0, 'ForStmt': 2}
LOGICAL = ('&&', '||')
UNMEASURED = ('ConstantExpr', 'UnaryExprOrTypeTraitExpr', 'GenericSelectionExpr', 'StaticAssertDecl', 'EnumDecl')
WRAPPERS = ('ParenExpr', 'ImplicitCastExpr')
TOPS = ('src/', 'include/')
SAID = {'function': ['no case calls this function'], 'case': ['no case reaches this case'],
        'condition': ['no case makes this condition false', 'no case makes this condition true']}
TOP = r'''static _Atomic unsigned char %(p)shits[%(bits)d];
[[maybe_unused]] static bool %(p)s(bool c, unsigned id)
{
    %(p)shits[2u * id + (c ? 1u : 0u)] = 1;
    return (c);
}
[[maybe_unused]] static void %(p)shit_(unsigned id)
{
    %(p)shits[2u * id + 1u] = 1;
}
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
    const char *%(p)spath = getenv("C23_BRANCHES");
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
    for (unsigned %(p)si = 0; %(p)si < %(bits)du; %(p)si++)
        %(p)sline[%(p)sat++] = %(p)shits[%(p)si] ? '1' : '0';
    %(p)sline[%(p)sat++] = '\n';
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

    if (%(p)sdladdr((const void *)%(p)shits, &%(p)sinfo) && %(p)sinfo.dli_fname)
        (void)%(p)sdlopen(%(p)sinfo.dli_fname, 0x1 | 0x10 | 0x80);
}
#endif
'''


def tree(values):
    return Path(values['build']) / 'branches' / 'tree'


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


def placed(loc, end, root):
    if not loc:
        return None, None
    if is_macro(loc):
        loc = loc['expansionLoc']
        if loc.get('isMacroArgExpansion') or 'offset' not in loc:
            return 'macro', owned(loc, root)
    if 'offset' not in loc:
        return None, None
    return loc['offset'] + (loc.get('tokLen', 0) if end else 0), owned(loc, root)


def stripped(node):
    while node.get('kind') in WRAPPERS or (node.get('kind') == 'UnaryOperator' and node.get('opcode') == '!'):
        inner = [child for child in node.get('inner', []) if isinstance(child, dict) and child]
        if not inner:
            return node
        node = inner[0]
    return node


def is_logical(node):
    node = stripped(node)
    return node.get('kind') == 'BinaryOperator' and node.get('opcode') in LOGICAL


def child(raw, index):
    one = raw[index] if len(raw) > index else None
    return one if isinstance(one, dict) and one else None


class Probing:
    def __init__(self, root):
        self.root = root
        self.edits = {}
        self.skips = {}
        self.texts = {}

    def add(self, path, key, edit):
        self.edits.setdefault(path, {})[key] = edit

    def skip(self, loc, kind):
        at, path = placed(loc, False, self.root)
        loc = loc.get('expansionLoc', loc) if isinstance(loc, dict) else {}
        at = loc.get('offset') if not isinstance(at, int) else at
        if path and isinstance(at, int):
            self.skips.setdefault(path, {})[at] = kind

    def walk(self, node, inside=False, depth=0):
        kind = node.get('kind')
        if kind in UNMEASURED or (kind == 'VarDecl' and (node.get('storageClass') == 'static' or node.get('constexpr'))):
            return
        raw = node.get('inner', [])
        if kind == 'FunctionDecl':
            return self.function(node, raw, depth)
        joining = kind == 'BinaryOperator' and node.get('opcode') in LOGICAL
        if inside and (kind in DECIDED or joining) and is_macro(node.get('loc')):
            return self.skip(node.get('loc'), 'condition')
        if inside and kind in DECIDED:
            self.condition(child(raw, DECIDED[kind]), depth)
        elif inside and joining:
            for operand in raw[:2]:
                self.condition(operand, depth + 1)
        elif inside and kind in ('CaseStmt', 'DefaultStmt'):
            self.label(node, raw)
        for one in raw:
            if isinstance(one, dict) and one:
                self.walk(one, inside, depth + 1)

    def function(self, node, raw, depth):
        body = next((one for one in raw if isinstance(one, dict) and one.get('kind') == 'CompoundStmt'), None)
        begin = body.get('range', {}).get('begin') if body else None
        at, path = placed(begin, False, self.root)
        if at == 'macro':
            self.skip(begin, 'function')
        if not isinstance(at, int) or not path:
            return
        named = placed(node.get('loc'), False, self.root)[0]
        self.add(path, ('function', at), ('function', at + 1, named if isinstance(named, int) else at))
        self.walk(body, True, depth + 1)

    def label(self, node, raw):
        shown, path = placed(node.get('range', {}).get('begin'), False, self.root)
        if node['kind'] == 'CaseStmt':
            first = child(raw, 0) or {}
            at, path = placed(first.get('range', {}).get('end'), True, self.root)
        else:
            at = shown + len('default') if isinstance(shown, int) else shown
        if 'macro' in (at, shown):
            self.skip(node.get('range', {}).get('begin'), 'case')
        elif isinstance(at, int) and isinstance(shown, int) and path:
            self.add(path, ('case', at), ('case', at, shown))

    def condition(self, node, depth):
        if not isinstance(node, dict) or not node or is_logical(node):
            return
        ends = node.get('range', {})
        begin, path = placed(ends.get('begin'), False, self.root)
        end, last = placed(ends.get('end'), True, self.root)
        if isinstance(begin, int) and path and is_macro(ends.get('begin')):
            named = placed(ends.get('begin'), True, self.root)[0]
            call = self.called(path, named)
            end, last = (call, path) if call != named else (end, last)
        if 'macro' in (begin, end):
            return self.skip(ends.get('begin') if begin == 'macro' else ends.get('end'), 'condition')
        if not isinstance(begin, int) or not isinstance(end, int) or not path or path != last:
            return
        end = self.called(path, end) if is_macro(ends.get('end')) else end
        self.add(path, ('condition', begin, end), ('condition', begin, end, depth))

    def called(self, path, at):
        text = self.texts.setdefault(path, (self.root / path).read_bytes())
        start = at
        while text[at:at + 1] in (b' ', b'\t'):
            at += 1
        if text[at:at + 1] != b'(':
            return start
        level = 0
        while at < len(text):
            level += {b'(': 1, b')': -1}.get(text[at:at + 1], 0)
            at += 1
            if not level:
                return at
        return start


def dumped(command, root):
    done = subprocess.run(command, cwd=root, capture_output=True)
    errors = [line for line in done.stderr.decode('utf-8', 'replace').splitlines()
              if ' error: ' in line and not line.endswith('FileNotFound')]
    if errors:
        die('the tree of %s cannot be read:\n%s' % (command[-1], '\n'.join(errors)))
    try:
        return json.loads(done.stdout)
    except ValueError:
        die('the tree of %s is not JSON' % command[-1])


def walked(root, command):
    found = dumped(command, root)
    marked(found, [None])
    probing = Probing(root)
    probing.walk(found)
    return probing


def placing(text, at):
    return text.count(b'\n', 0, at) + 1, at - (text.rfind(b'\n', 0, at) + 1) + 1


def rewritten(text, path, edits, prefix):
    rows, pieces = [], []
    for edit in sorted(edits.values(), key=lambda one: (one[1], one[0])):
        number = len(rows)
        if edit[0] == 'condition':
            _, begin, end, depth = edit
            rows.append(('condition', begin))
            pieces += [((begin, 1, depth), prefix + '(('), ((end, 0, -depth), '), %d)' % number)]
            continue
        kind, at, shown = edit
        at = text.index(b':', at) + 1 if kind == 'case' else at
        rows.append((kind, shown))
        pieces.append(((at, 2, 0), ' %shit_(%d);' % (prefix, number)))
    out, last = bytearray(), 0
    for (at, _, _), piece in sorted(pieces, key=lambda one: one[0]):
        out += text[last:at] + piece.encode()
        last = at
    out += text[last:]
    bits = 2 * max(1, len(rows))
    values = {'p': prefix, 'bits': bits, 'line': len(path) + bits + 2, 'name': path, 'name_len': len(path) + 1}
    if path.endswith('.h'):
        guard = prefix.upper() + 'GUARD'
        block = '#ifndef %s\n    #define %s\n%s%s#endif\n#line 1\n' % (guard, guard, TOP % values, WRITER % values)
        out = block.encode() + bytes(out)
    else:
        out = (TOP % values + '#line 1\n').encode() + bytes(out) + b'\n' + (WRITER % values).encode()
    return out, [dict(zip(('kind', 'line', 'col'), (kind,) + placing(text, at))) for kind, at in rows]


def probed(config, values):
    root = Path(values['root'])
    jobs = {}
    for artifact in build.artifacts(config):
        if 'target' in artifact:
            continue
        _, mine = build.planned(config, values, artifact)
        Path(mine['work']).parent.mkdir(parents=True, exist_ok=True)
        for source in mine['sources']:
            if source.startswith('src/') and source.endswith('.c') and source not in jobs:
                jobs[source] = expanded(config['branches']['dump'], dict(mine, source=source))
    with concurrent.futures.ThreadPoolExecutor(min(8, os.cpu_count() or 1)) as pool:
        found = list(pool.map(lambda command: walked(root, command), jobs.values()))
    edits, skips = {}, {}
    for probing in found:
        for path, table in probing.edits.items():
            edits.setdefault(path, {}).update(table)
        for path, table in probing.skips.items():
            skips.setdefault(path, {}).update(table)
    return written(root, tree(values), edits, skips)


def written(root, where, edits, skips):
    shutil.rmtree(where, ignore_errors=True)
    for top in ('src', 'include'):
        if (root / top).is_dir():
            shutil.copytree(root / top, where / top)
    headers = sorted(path for path in edits if path.endswith('.h'))
    probes = {}
    for path in sorted(edits):
        prefix = 'c23b_h%d_' % headers.index(path) if path in headers else 'c23b_'
        out, probes[path] = rewritten((root / path).read_bytes(), path, edits[path], prefix)
        (where / path).write_bytes(out)
    gaps = {path: [dict(zip(('kind', 'line', 'col'), (kind,) + placing((root / path).read_bytes(), at)))
                   for at, kind in sorted(table.items())] for path, table in skips.items()}
    return probes, gaps


def joined(probes, hits):
    seen = {}
    lines = hits.read_text(encoding='utf-8', errors='replace').splitlines() if hits.is_file() else []
    for line in lines:
        name, _, bits = line.partition(' ')
        if name not in probes:
            continue
        if len(bits) != 2 * max(1, len(probes[name])) or set(bits) - set('01'):
            die('%s: a line of %s does not match its probes' % (hits, name))
        old = seen.get(name, '0' * len(bits))
        seen[name] = ''.join('1' if '1' in pair else '0' for pair in zip(old, bits))
    return seen


BRANCH_SIDE = {'true': 'no case makes this condition true',
               'false': 'no case makes this condition false',
               'function': 'no case calls this function',
               'case': 'no case reaches this case'}


def excused(listed, system, name, line, col, said):
    for entry in listed:
        if (entry.get('why') and entry.get('line') == line and entry.get('col') == col
                and BRANCH_SIDE.get(entry.get('side')) == said and fnmatch.fnmatch(name, entry['path'])
                and (not entry.get('system') or entry['system'] == system)):
            entry['hits'] = entry.get('hits', 0) + 1
            return True
    return False


def reported(probes, gaps, hits, excepted=(), system=None):
    seen, outcomes, total = joined(probes, hits), [], 0
    for name, rows in probes.items():
        bits = seen.get(name, '0' * (2 * max(1, len(rows))))
        for number, row in enumerate(rows):
            sides = SAID[row['kind']]
            for side, said in zip(range(2 - len(sides), 2), sides):
                total += 1
                if bits[2 * number + side] == '0':
                    outcomes.append((name, row['line'], row['col'], said))
    macros = [(name, row['line'], row['col'], 'this %s is in a macro, and is not measured' % row['kind'])
              for name, rows in gaps.items() for row in rows]
    listed = [dict(entry) for entry in excepted]
    missed = [fault for fault in outcomes if not excused(listed, system, *fault)]
    exempt = len(outcomes) - len(missed)
    stale = []
    for entry in listed:
        if entry.get('system') and entry['system'] != system:
            continue
        where = (entry.get('path', '?'), entry.get('line', 0), entry.get('col', 0))
        if not entry.get('why'):
            stale.append(where + ('this exemption states no reason',))
        elif not entry.get('hits'):
            stale.append(where + ('this exemption excuses no unreached outcome',))
    for fault in sorted(missed + macros + stale):
        print('%s:%d:%d: %s' % fault)
    print('branches: %d outcomes, %d reached%s'
          % (total, total - len(outcomes), ', %d exempt' % exempt if exempt else ''))
    if missed or macros or stale:
        die('outcomes not reached: %d; branches not measured: %d' % (len(missed), len(macros)))
