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
#ifdef __wasm__
extern char c23b_plan[];
static char *c23b_ffind(const char *all, const char *key)
{
    for (; *all; all++) {
        size_t n = 0;

        while (key[n] && all[n] == key[n])
            n++;
        if (!key[n])
            return ((char *)all);
    }
    return (nullptr);
}
static long long c23b_fget(const char *key)
{
    const char *at = c23b_ffind(c23b_plan, key);
    long long n = 0;

    while (at && *key) {
        at++;
        key++;
    }
    while (at && *at >= '0' && *at <= '9')
        n = 10 * n + (*at++ - '0');
    return (n);
}
static int c23b_fmine(const char *parts)
{
    const char *part = c23b_plan;

    while (*part)
        part++;
    return (!part[1] || c23b_ffind(parts, part + 1) != nullptr);
}
static long long c23b_ft = -1;
static void c23b_ftold(void)
{
    char *at = c23b_ffind(c23b_plan, " told=0");

    if (at)
        at[6] = '1';
}
#else
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
    const char *at = getenv("C23_FAIL_PART");
    char want[130];
    size_t len = 0;

    if (!at)
        return (1);
    for (at += strspn(at, " "); *at; at += len + strspn(at + len, " ")) {
        len = strcspn(at, " ");
        if (len + 3 > sizeof(want))
            continue;
        want[0] = ' ';
        memcpy(want + 1, at, len);
        want[len + 1] = ' ';
        want[len + 2] = '\0';
        if (strstr(parts, want))
            return (1);
    }
    return (0);
}
static long long c23b_ft;
static void c23b_ftold(void)
{
    char *all = getenv("C23_FAIL");
    char *at = all ? strstr(all, " told=0") : nullptr;

    if (at)
        at[6] = '1';
}
#endif
[[gnu::weak, gnu::visibility("hidden")]] _Atomic long long c23b_fh;
static long long c23b_fn;
#if defined(__wasm__)
static void c23b_fwait(long long *calls, long long at)
{
    (void)calls;
    (void)at;
}
#else
#if defined(_WIN32)
[[gnu::dllimport]] int __stdcall SwitchToThread(void);
#else
#include <sched.h>
#endif
static void c23b_fwait(long long *calls, long long at)
{
    if (!at || ++*calls != at)
        return;
    while (c23b_fh < c23b_fn) {
#if defined(_WIN32)
        SwitchToThread();
#else
        sched_yield();
#endif
    }
}
#endif
static void c23b_fread(void);
static void c23b_fturn(_Atomic long long *count, long long *counted)
{
    long long now = c23b_fget(" turn=");

    if (now != c23b_ft) {
        c23b_ft = now;
        c23b_fread();
    }
    if (now != *counted) {
        *counted = now;
        *count = 0;
    }
}
'''
COUNT = ('[[gnu::weak, gnu::visibility("hidden")]] _Atomic long long c23b_fc_%(n)s;\n'
         '[[gnu::weak, gnu::visibility("hidden")]] long long c23b_fg_%(n)s;\nstatic long long c23b_fs_%(n)s;\n'
         'static long long c23b_fa_%(n)s;\nstatic long long c23b_fk_%(n)s;\nstatic long long c23b_fu_%(n)s;\n')
READ = ('    c23b_fs_%(n)s = c23b_fget(" %(n)s=");\n    c23b_fa_%(n)s = c23b_fget(" hold@%(p)s@%(n)s=");\n'
        '    c23b_fu_%(n)s = c23b_fget(" until@%(n)s=");\n')
MINE = ('#ifndef __wasm__\n[[gnu::constructor]]\n#endif\nstatic void c23b_fread(void)\n{\n    if (!c23b_fmine("%s"))\n'
        '        return;\n    c23b_fn = c23b_fget(" @times=");\n')
WRAP = ('    c23b_fturn(&c23b_fc_%(n)s, &c23b_fg_%(n)s);\n    c23b_fwait(&c23b_fk_%(n)s, c23b_fa_%(n)s);\n'
        '    if (++c23b_fc_%(n)s == c23b_fs_%(n)s) {\n        c23b_ftold();\n        return (%(v)s);\n    }\n'
        '    if (c23b_fu_%(n)s) {\n        auto c23b_fr = %(c)s;\n\n        c23b_fh++;\n        return (c23b_fr);\n    }\n'
        '    return (%(c)s);\n}\n')
EMPTY = ('    c23b_fturn(&c23b_fc_%(n)s, &c23b_fg_%(n)s);\n    c23b_fwait(&c23b_fk_%(n)s, c23b_fa_%(n)s);\n'
         '    %(c)s;\n    if (c23b_fu_%(n)s)\n        c23b_fh++;\n}\n')


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
           '    c23b_fturn(&c23b_fc_%s, &c23b_fg_%s);' % (name, name),
           '    c23b_fwait(&c23b_fk_%s, c23b_fa_%s);' % (name, name),
           '    if (++c23b_fc_%s == c23b_fs_%s) {' % (name, name),
           '        c23b_ftold();',
           '        return (nullptr);',
           '    }',
           '    c23b_fp = (void *)(%s);' % call,
           '    if (c23b_fu_%s)' % name,
           '        c23b_fh++;']
    for item in loaded:
        out += ['    if (a1 && !strcmp((const char *)a1, "%s")) {' % item,
                '        %s%s = (typeof(*%s) *)c23b_fp;' % (POINT, item, item),
                '        if (c23b_fp)',
                '            return ((%s)%s%s);' % (result, STAND, item),
                '    }']
    out += ['    return ((%s)c23b_fp);' % result, '}']
    return '\n'.join(out) + '\n'


def layer(text, used, declared, calls, parts, path):
    if not used:
        return 0, '', ''
    if ' ' in path:
        die('%s holds a space, and the layer names each file in a key of C23_FAIL' % path)
    at = rewrite.included(text)
    source = text.decode('utf-8', 'replace')
    loaded = loaded_calls(calls, used, source, declared) if any(name in LOADERS for name in used) else []
    every = list(used) + loaded
    block = ''.join(rewrite.RENAME % {'p': PREFIX, 'n': name} for name in used)
    tail = ''.join('#pragma pop_macro("%s")\n' % name for name in used) + STATE
    tail += ''.join(COUNT % {'n': name} for name in every)
    tail += MINE % ' '.join([''] + parts + ['']) + ''.join(READ % {'n': name, 'p': path} for name in every) + '}\n'
    for name in loaded:
        tail += 'static typeof(*%s) *%s%s;\n' % (name, POINT, name)
        head, call = rewrite.signed(name, declared[name], STAND)
        aim = call.replace('(%s)' % name, '(%s%s)' % (POINT, name), 1)
        tail += head + WRAP % {'n': name, 'v': calls[name], 'c': aim}
    for name in used:
        head, call = rewrite.signed(name, declared[name], PREFIX)
        if (declared[name][0] == 'void') != (calls[name] == ''):
            die('%s gives %s, so its value in [fail.calls] is %s' % ((name, 'no result', 'the empty text')
                if declared[name][0] == 'void' else (name, 'a result', 'not the empty text')))
        if declared[name][0] == 'void':
            tail += head + EMPTY % {'n': name, 'c': call}
        elif name in LOADERS and loaded:
            tail += head + loader_wrap(name, call, rewrite.spelled(declared[name][0]), loaded, declared)
        else:
            tail += head + WRAP % {'n': name, 'v': calls[name], 'c': call}
    return at, block + '#line %d\n' % (text.count(b'\n', 0, at) + 1), tail


def wrapped(text, outer):
    at, block, tail = outer
    return text[:at] + block.encode() + text[at:] + b'\n' + tail.encode()


def failed(config, values):
    calls = listed(config, values)
    root, where = Path(values['root']), rewrite.tree(values, 'fail')
    files = rewrite.united(rewrite.each(config, values, lambda top, path, tree: rewrite.found(top, path, tree, calls)))
    rewrite.copied(root, where)
    for (path, used, declared, _), parts in files:
        if not used:
            continue
        text = (root / path).read_bytes()
        (where / path).write_bytes(wrapped(text, layer(text, used, declared, calls, parts, Path(path).as_posix())))
