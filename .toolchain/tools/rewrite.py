# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import concurrent.futures
import json
import os
import shutil
import subprocess
from pathlib import Path

import build
from shared import die, expanded

TOPS = ('src/', 'include/')
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


def placing(text, at):
    return text.count(b'\n', 0, at) + 1, at - (text.rfind(b'\n', 0, at) + 1) + 1


def writer(values):
    return WRITER % dict(values, fill=values['fill'] % values)
