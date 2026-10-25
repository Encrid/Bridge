# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import fnmatch
import re
import tomllib
from pathlib import Path

import rewrite
from shared import die, located

BUILT = {'malloc': ('take', 'memory'), 'calloc': ('take', 'memory'), 'realloc': ('move', 'memory'),
         'free': ('give', 'memory')}
PREFIX = 'c23b_l_'
OPENS = ('if', 'ifdef', 'ifndef')
DIRECTIVE = re.compile(rb'[ \t]*#[ \t]*(\w+)')
COUNT = re.compile(r'-?[0-9]+')
RENAME = '''[[maybe_unused]] static typeof(%(n)s) %(p)s%(n)s;
#pragma push_macro("%(n)s")
#undef %(n)s
#define %(n)s %(p)s%(n)s
'''
COUNTS = r'''static _Atomic long long %(p)skept[%(slots)d];
static unsigned %(p)snumber(char *line, unsigned at, long long n, bool spaced)
{
    char digits[24];
    unsigned long long left = n < 0 ? 0ull - (unsigned long long)n : (unsigned long long)n;
    unsigned count = 0;

    if (spaced)
        line[at++] = ' ';
    if (n < 0)
        line[at++] = '-';
    do {
        digits[count++] = (char)('0' + left %% 10u);
        left /= 10u;
    } while (left);
    while (count)
        line[at++] = digits[--count];
    return (at);
}
'''
FILL = r'''    for (unsigned %(p)si = 0; %(p)si < %(slots)du; %(p)si++)
        %(p)sat = %(p)snumber(%(p)sline, %(p)sat, %(p)skept[%(p)si], %(p)si > 0u);
'''


def listed(config, values):
    own = config.get('leaks', {})
    tables = [own]
    for name in own.get('with', []):
        path = located(name, values)
        if not path.is_file():
            die('[leaks] with: %s is not there' % path.as_posix())
        tables.append(tomllib.loads(path.read_text(encoding='utf-8')).get('leaks', {}))
    names = dict(BUILT)
    for table in tables:
        for take in table.get('memory', []):
            names[take] = ('take', 'memory')
        for take, give in table.get('pairs', {}).items():
            names[take], names[give] = ('take', take), ('give', take)
    return names, list(dict.fromkeys(['memory'] + [take for table in tables for take in table.get('pairs', {})]))


def plain(kind):
    return kind.get('desugaredQualType', kind.get('qualType', ''))


def declaration(node):
    whole = plain(node.get('type', {}))
    parameters = [plain(one.get('type', {})) for one in node.get('inner', [])
                  if isinstance(one, dict) and one.get('kind') == 'ParmVarDecl']
    return whole.split('(', 1)[0].strip(), parameters, '...' in whole


def found(root, path, tree, names):
    used, defined, declared = set(), set(), {}

    def walk(node):
        kind, name = node.get('kind'), node.get('name')
        if kind == 'DeclRefExpr':
            name = node.get('referencedDecl', {}).get('name')
            if name in names and rewrite.owned(node.get('range', {}).get('begin', {}), root) == path:
                used.add(name)
        elif kind == 'FunctionDecl' and name in names:
            declared.setdefault(name, declaration(node))
            body = any(isinstance(one, dict) and one.get('kind') == 'CompoundStmt' for one in node.get('inner', []))
            if body and rewrite.owned(node.get('loc', {}), root) == path:
                defined.add(name)
        for one in node.get('inner', []):
            if isinstance(one, dict) and one:
                walk(one)
    walk(tree)
    return path, sorted(used - defined), declared


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


def wrapped(name, role, kinds, declaration):
    returned, parameters, variadic = declaration
    act, kind = role
    if variadic:
        die('%s takes a list of arguments of no fixed length' % name)
    if act != 'give' and not returned.endswith('*'):
        die('%s takes a resource, and does not give a pointer' % name)
    if act == 'give' and not (parameters and parameters[0].endswith('*')):
        die('%s gives back a resource, and does not get a pointer first' % name)
    slot = kinds.index(kind)
    arguments = ', '.join('typeof(%s) a%d' % (one, n) for n, one in enumerate(parameters)) or 'void'
    call = '%s(%s)' % (name, ', '.join('a%d' % n for n in range(len(parameters))))
    head = 'static typeof(%s) %s%s(%s)\n{\n' % (returned, PREFIX, name, arguments)
    if act == 'take':
        body = ('    typeof(%s) r = %s;\n\n    if (r)\n        %skept[%d]++;\n    return (r);\n'
                % (returned, call, PREFIX, slot))
    elif act == 'move':
        body = ('    typeof(%s) r;\n\n    if (!a1)\n        %skept[%d]++;\n    r = %s;\n    if (!a0 && r)\n'
                '        %skept[%d]++;\n    return (r);\n' % (returned, PREFIX, len(kinds), call, PREFIX, slot))
    else:
        body = '    if (a0)\n        %skept[%d]--;\n    %s%s;\n' % (PREFIX, slot, '' if returned == 'void' else 'return ',
                                                               call)
    return head + body + '}\n'


def rewritten(text, path, used, declared, names, kinds):
    text = text.rstrip(b'\r\n') + b'\n'
    at = included(text)
    slots = len(kinds) + 1
    values = {'p': PREFIX, 'slots': slots, 'line': len(path) + 2 + 22 * slots, 'name': path,
              'name_len': len(path) + 1, 'variable': 'C23_LEAKS', 'fill': FILL, 'kept': 'kept'}
    head = ''.join(RENAME % {'p': PREFIX, 'n': name} for name in used)
    head += '#line %d\n' % (text.count(b'\n', 0, at) + 1)
    tail = ''.join('#pragma pop_macro("%s")\n' % name for name in used) + COUNTS % values
    tail += ''.join(wrapped(name, names[name], kinds, declared[name]) for name in used)
    return text[:at] + head.encode() + text[at:] + b'\n' + (tail + rewrite.writer(values)).encode()


def counted(config, values):
    names, kinds = listed(config, values)
    root, where = Path(values['root']), rewrite.tree(values, 'leaks')
    files = rewrite.each(config, values, lambda top, path, tree: found(top, path, tree, names))
    rewrite.copied(root, where)
    counting = []
    for path, used, declared in files:
        if not used:
            continue
        (where / path).write_bytes(rewritten((root / path).read_bytes(), path, used, declared, names, kinds))
        counting.append(path)
    return counting, kinds


def added(path, files, slots):
    sums = [0] * slots
    lines = path.read_text(encoding='utf-8', errors='replace').splitlines() if path.is_file() else []
    for line in lines:
        name, _, rest = line.partition(' ')
        if name not in files:
            continue
        counts = rest.split(' ')
        if len(counts) != slots or not all(COUNT.fullmatch(one) for one in counts):
            die('%s: a line of %s does not match its counts' % (path.as_posix(), name))
        sums = [one + int(two) for one, two in zip(sums, counts)]
    return sums


def excused(listed, system, case, kind):
    for entry in listed:
        if (entry.get('why') and entry.get('kind') == kind and fnmatch.fnmatch(case, entry.get('case', ''))
                and (not entry.get('system') or entry['system'] == system)):
            entry['hits'] = entry.get('hits', 0) + 1
            return True
    return False


def reported(files, kinds, cases, excepted=(), system=None):
    listed, faults, exempt, balanced = [dict(entry) for entry in excepted], [], 0, 0
    for name, path in cases:
        sums, own, excuses = added(path, files, len(kinds) + 1), [], 0
        for kind, count in zip(kinds, sums):
            if count > 0 and excused(listed, system, name, kind):
                excuses += 1
            elif count > 0:
                own.append('leaks: %s keeps %d %s' % (name, count, kind))
            elif count < 0:
                own.append('leaks: %s gives back %d %s that it did not take' % (name, -count, kind))
        if sums[-1]:
            own.append('leaks: %s calls realloc with 0 bytes' % name)
        faults += own
        balanced += not own and not excuses
        exempt += not own and bool(excuses)
    stale = []
    for entry in listed:
        if entry.get('system') and entry['system'] != system:
            continue
        where = (entry.get('case', '?'), entry.get('kind', '?'))
        if not entry.get('why'):
            stale.append('leaks: the exemption of %s for %s states no reason' % where)
        elif not entry.get('hits'):
            stale.append('leaks: the exemption of %s for %s excuses no kept resource' % where)
    for line in faults + stale:
        print(line)
    print('leaks: %d cases, %d give back what they take%s'
          % (len(cases), balanced, ', %d exempt' % exempt if exempt else ''))
    if faults or stale:
        die('cases that do not give back what they take: %d; exemptions that are faults: %d'
            % (len(cases) - balanced - exempt, len(stale)))
