# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
from pathlib import Path

import rewrite
from shared import die, tables

BUILT = {'malloc': 'nullptr', 'calloc': 'nullptr', 'realloc': 'nullptr',
         'GetProcAddress': 'nullptr', 'dlsym': 'nullptr'}
PREFIX = 'c23b_f_'
STAND = 'c23b_ft_'
POINT = 'c23b_fp_'
LOADERS = ('GetProcAddress', 'dlsym')
STATE = r'''#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
static long long c23b_fget(const char *key)
{
    const char *all = getenv("C23_FAIL");
    const char *at = all ? strstr(all, key) : nullptr;

    return (at ? atoll(at + strlen(key)) : 0);
}
static int c23b_fmine(const char *parts)
{
    const char *part = getenv("C23_FAIL_PART");

    return (!part || strstr(parts, part) != nullptr);
}
'''
COUNT = '[[gnu::weak, gnu::visibility("hidden")]] _Atomic long long c23b_fc_%(n)s;\nstatic long long c23b_fs_%(n)s;\n'
READ = '    c23b_fs_%(n)s = c23b_fget(" %(n)s=");\n'
MINE = '[[gnu::constructor]] static void c23b_fread(void)\n{\n    if (!c23b_fmine("%s"))\n        return;\n'
WRAP = '    if (++c23b_fc_%(n)s == c23b_fs_%(n)s)\n        return (%(v)s);\n    return (%(c)s);\n}\n'


def listed(config, values):
    calls = dict(BUILT)
    for table in tables(config, values, 'fail'):
        for name, value in table.get('calls', {}).items():
            if name in calls:
                die('[fail.calls] lists %s two times' % name)
            calls[name] = value
    return calls


def loaded_calls(calls, used, source, declared):
    return [name for name in calls if name not in used and name not in LOADERS
            and name in declared and ('"%s"' % name) in source]


def loader_wrap(name, call, result, loaded, declared):
    out = ['    void *c23b_fp;',
           '    if (++c23b_fc_%s == c23b_fs_%s)' % (name, name),
           '        return (nullptr);',
           '    c23b_fp = (void *)(%s);' % call]
    for item in loaded:
        out += ['    if (a1 && !strcmp((const char *)a1, "%s")) {' % item,
                '        %s%s = (typeof(*%s) *)c23b_fp;' % (POINT, item, item),
                '        if (c23b_fp)',
                '            return ((%s)%s%s);' % (result, STAND, item),
                '    }']
    out += ['    return ((%s)c23b_fp);' % result, '}']
    return '\n'.join(out) + '\n'


def layer(text, used, declared, calls, parts):
    if not used:
        return 0, '', ''
    at = rewrite.included(text)
    source = text.decode('utf-8', 'replace')
    loaded = loaded_calls(calls, used, source, declared) if any(name in LOADERS for name in used) else []
    every = list(used) + loaded
    block = ''.join(rewrite.RENAME % {'p': PREFIX, 'n': name} for name in used)
    tail = ''.join('#pragma pop_macro("%s")\n' % name for name in used) + STATE
    tail += ''.join(COUNT % {'n': name} for name in every)
    tail += MINE % ' '.join([''] + parts + ['']) + ''.join(READ % {'n': name} for name in every) + '}\n'
    for name in loaded:
        tail += 'static typeof(*%s) *%s%s;\n' % (name, POINT, name)
        head, call = rewrite.signed(name, declared[name], STAND)
        aim = call.replace('(%s)' % name, '(%s%s)' % (POINT, name), 1)
        tail += head + WRAP % {'n': name, 'v': calls[name], 'c': aim}
    for name in used:
        head, call = rewrite.signed(name, declared[name], PREFIX)
        if declared[name][0] == 'void':
            die('%s gives no result' % name)
        if name in LOADERS and loaded:
            tail += head + loader_wrap(name, call, rewrite.spelled(declared[name][0]), loaded, declared)
        else:
            tail += head + WRAP % {'n': name, 'v': calls[name], 'c': call}
    return at, block + '#line %d\n' % (text.count(b'\n', 0, at) + 1), tail


def failed(config, values):
    calls = listed(config, values)
    root, where = Path(values['root']), rewrite.tree(values, 'fail')
    files = rewrite.each(config, values, lambda top, path, tree: rewrite.found(top, path, tree, calls))
    rewrite.copied(root, where)
    for (path, used, declared, _), parts in files:
        if not used:
            continue
        text = (root / path).read_bytes()
        at, block, tail = layer(text, used, declared, calls, parts)
        (where / path).write_bytes(text[:at] + block.encode() + text[at:] + b'\n' + tail.encode())
