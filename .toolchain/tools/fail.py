# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
from pathlib import Path

import rewrite
from shared import die, tables

BUILT = {'malloc': 'nullptr', 'calloc': 'nullptr', 'realloc': 'nullptr'}
PREFIX = 'c23b_f_'
STATE = r'''#include <stdlib.h>
#include <string.h>
static long long c23b_fget(const char *key)
{
    const char *all = getenv("C23_FAIL");
    const char *at = all ? strstr(all, key) : nullptr;

    return (at ? atoll(at + strlen(key)) : 0);
}
'''
COUNT = '[[gnu::weak, gnu::visibility("hidden")]] _Atomic long long c23b_fc_%(n)s;\nstatic long long c23b_fs_%(n)s;\n'
READ = '    c23b_fs_%(n)s = c23b_fget(" %(n)s=");\n'
WRAP = '    if (++c23b_fc_%(n)s == c23b_fs_%(n)s)\n        return (%(v)s);\n    return (%(c)s);\n}\n'


def listed(config, values):
    calls = dict(BUILT)
    for table in tables(config, values, 'fail'):
        for name, value in table.get('calls', {}).items():
            if name in calls:
                die('[fail.calls] lists %s two times' % name)
            calls[name] = value
    return calls


def layer(text, used, declared, calls):
    if not used:
        return 0, '', ''
    at = rewrite.included(text)
    block = ''.join(rewrite.RENAME % {'p': PREFIX, 'n': name} for name in used)
    tail = ''.join('#pragma pop_macro("%s")\n' % name for name in used) + STATE
    tail += ''.join(COUNT % {'n': name} for name in used)
    tail += '[[gnu::constructor]] static void c23b_fread(void)\n{\n%s}\n' % ''.join(READ % {'n': name} for name in used)
    for name in used:
        head, call = rewrite.signed(name, declared[name], PREFIX)
        if declared[name][0] == 'void':
            die('%s gives no result' % name)
        tail += head + WRAP % {'n': name, 'v': calls[name], 'c': call}
    return at, block + '#line %d\n' % (text.count(b'\n', 0, at) + 1), tail


def failed(config, values):
    calls = listed(config, values)
    root, where = Path(values['root']), rewrite.tree(values, 'fail')
    files = rewrite.each(config, values, lambda top, path, tree: rewrite.found(top, path, tree, calls))
    rewrite.copied(root, where)
    for path, used, declared, _ in files:
        if not used:
            continue
        text = (root / path).read_bytes()
        at, block, tail = layer(text, used, declared, calls)
        (where / path).write_bytes(text[:at] + block.encode() + text[at:] + b'\n' + tail.encode())
