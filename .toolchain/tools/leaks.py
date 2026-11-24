# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import fnmatch
import re
from pathlib import Path

import fail
import rewrite
import shared
from shared import die, known

BUILT = {**dict.fromkeys(('malloc', 'calloc', 'aligned_alloc', 'strdup', 'strndup'), ('take', 'memory', 'result')),
         'realloc': ('move', 'memory', None), 'free': ('give', 'memory', '1')}
PLACES = {'take': re.compile(r'result|\*?[1-9][0-9]*|\*[1-9][0-9]*\.[A-Za-z_]\w*'),
          'give': re.compile(r'result|\*?[1-9][0-9]*')}
VERBS = {'take': 'take', 'give': 'give back'}
PREFIX = 'c23b_l_'
TESTED = '    if (%s' + PREFIX + 'held(&(%s), sizeof(%s)))\n        %skept[%d]%s;\n'
COUNT = re.compile(r'-?[0-9]+')
QUALIFIED = re.compile(r'(\s*\b(?:const|volatile|restrict|__restrict|__restrict__)\b)+$')
COUNTS = r'''#ifdef __wasm__
[[gnu::section("c23l"), gnu::used]] static struct {
    unsigned slots;
    unsigned length;
    _Atomic long long kept[%(slots)d];
    char path[%(size)d];
} %(p)schunk = { %(slots)du, %(length)du, {}, "%(name)s" };
static _Atomic long long *const %(p)skept = %(p)schunk.kept;
#else
static _Atomic long long %(p)skept[%(slots)d];
#endif
[[maybe_unused]] static bool %(p)sheld(const void *at, typeof(sizeof(0)) size)
{
    const unsigned char *byte = at;

    for (typeof(sizeof(0)) i = 0; i < size; i++)
        if (byte[i])
            return (true);
    return (false);
}
'''

def listed(config, values):
    tables = shared.tables(config, values, 'leaks')
    names ={name: [role + (True,)] for name, role in BUILT.items()}
    for table in tables:
        for take in table.get('memory', []):
            names[take] = [('take', 'memory', 'result', True)]
        for take, give in table.get('pairs', {}).items():
            names[take], names[give] = [('take', take, 'result', True)], [('give', take, '1', True)]
    for table in tables:
        for kind, acts in table.get('kinds', {}).items():
            kinded(names, kind, acts)
    return names, list(dict.fromkeys(['memory'] + [kind for table in tables
                                                   for kind in [*table.get('pairs', {}), *table.get('kinds', {})]]))


def kinded(names, kind, acts):
    known(acts, 'kind', '[leaks.kinds.%s]' % kind)
    for act in ('take', 'give'):
        for name, places in acts.get(act + 's', {}).items():
            for place in map(str, places if isinstance(places, list) else [places]):
                if not PLACES[act].fullmatch(place):
                    die('[leaks.kinds.%s]: %s cannot %s at %s' % (kind, name, VERBS[act], place))
                effects = names.setdefault(name, [])
                if (act, kind, place, False) not in effects:
                    effects.append((act, kind, place, False))


def pointer(kind):
    return QUALIFIED.sub('', kind).endswith('*')


def fitted(name, effects, returned, parameters):
    for act, _, place, strict in effects:
        if strict and act != 'give' and not pointer(returned):
            die('%s takes a resource, and does not give a pointer' % name)
        if strict and act == 'give' and not (parameters and pointer(parameters[0])):
            die('%s gives back a resource, and does not get a pointer first' % name)
        if place == 'result' and returned == 'void':
            die('%s gives no result' % name)
        if place in (None, 'result'):
            continue
        number = int(place.lstrip('*').partition('.')[0])
        if number > len(parameters):
            die('%s has no argument %d' % (name, number))
        if place.startswith('*') and not pointer(parameters[number - 1]):
            die('%s does not get a pointer as argument %d' % (name, number))


def tested(effect, kinds):
    act, kind, place, _ = effect
    number, _, field = place.lstrip('*').partition('.')
    value = 'r' if place == 'result' else 'a%d' % (int(number) - 1)
    guard = value + ' && ' if place.startswith('*') else ''
    value = ('(*%s)' % value if guard else value) + ('.' + field if field else '')
    return TESTED % (guard, value, value, PREFIX, kinds.index(kind), '++' if act == 'take' else '--')


def wrapped(name, effects, kinds, declaration):
    head, call = rewrite.signed(name, declaration, PREFIX)
    returned, parameters, _, _ = declaration
    fitted(name, effects, returned, parameters)
    if effects[0][0] == 'move':
        body = ('    typeof(%s) r;\n\n    if (!a1)\n        %skept[%d]++;\n    r = %s;\n    if (!a0 && r)\n'
                '        %skept[%d]++;\n    return (r);\n'
                % (call, PREFIX, len(kinds), call, PREFIX, kinds.index(effects[0][1])))
        return head + body + '}\n'
    before = ''.join(tested(one, kinds) for one in effects if one[0] == 'give' and one[2] != 'result')
    after = ''.join(tested(one, kinds) for one in effects if one[0] == 'take' or one[2] == 'result')
    if returned == 'void':
        return head + before + '    %s;\n' % call + after + '}\n'
    return head + before + '    typeof(%s) r = %s;\n\n' % (call, call) + after + '    return (r);\n}\n'


def rewritten(text, path, used, ends, declared, names, kinds, outer=(0, '', '')):
    text = text.rstrip(b'\r\n') + b'\n'
    at = rewrite.included(text)
    slots = len(kinds) + 1
    values = {'p': PREFIX, 'slots': slots, 'name': path, 'length': len(path), 'size': len(path) + 1, 'kept': 'kept'}
    places = {name: max(at, text.index(b'\n', ends[name]) + 1) if name in ends else at for name in used}
    out, last = b'', 0
    for where in sorted(set(places.values()) | {at}):
        block = ''.join(rewrite.RENAME % {'p': PREFIX, 'n': name} for name in used if places[name] == where)
        block = (outer[1] if where == at else '') + block
        out += text[last:where] + (block + '#line %d\n' % (text.count(b'\n', 0, where) + 1)).encode()
        last = where
    tail = ''.join('#pragma pop_macro("%s")\n' % name for name in used) + COUNTS % values
    tail += ''.join(wrapped(name, names[name], kinds, declared[name]) for name in used)
    return out + text[last:] + b'\n' + (tail + outer[2] + rewrite.writer(values)).encode()


def counted(config, values):
    names, kinds = listed(config, values)
    calls = fail.listed(config, values)
    callbacks = {name for name, effects in names.items()
                 if any((act, place) == ('give', 'result') or (act == 'take' and place[0].isdigit())
                        for act, _, place, _ in effects)}
    root, where = Path(values['root']), rewrite.tree(values, 'leaks')
    files = rewrite.united(rewrite.each(config, values, lambda top, path, tree: rewrite.found(
        top, path, tree, {**calls, **names}, callbacks), True))
    rewrite.copied(root, where, [path for (path, _, _, _), _ in files])
    counting = []
    for (path, used, declared, ends), parts in files:
        if not used:
            continue
        text = (root / path).read_bytes()
        failing = [name for name in used if name in calls and path.startswith('src/')]
        outer = fail.layer(text, failing, declared, calls, parts, Path(path).as_posix())
        (where / path).write_bytes(rewritten(text, path, [name for name in used if name in names], ends, declared,
                                             names, kinds, outer))
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
