# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import os
import shutil
import stat
import sys
from pathlib import Path

import shared
from shared import arguments, die, located


def writable(func, path, _):
    os.chmod(path, stat.S_IWRITE)
    func(path)


def scrub(path):
    shutil.rmtree(path, **({'onexc': writable} if sys.version_info >= (3, 12) else {'onerror': writable}))


def removed(folder, kept):
    for path in sorted(folder.iterdir()):
        if path in kept:
            continue
        if any(path in keep.parents for keep in kept):
            removed(path, kept)
        elif path.is_dir():
            scrub(path)
        else:
            os.chmod(path, stat.S_IWRITE)
            path.unlink()


def main():
    args = arguments(everything='--all')
    config = shared.config(args.root)
    target = args.target or shared.this_target(config['targets']) or die('no target runs here: name one')
    values = shared.variables(config, args, target, '')
    build = Path(values['build'])
    if not build.is_dir():
        return print('nothing to clean: %s is not there' % build.as_posix())
    if args.everything:
        scrub(build)
    else:
        kept = {(shared.HERE / config['zig']['folder']).resolve()}
        kept |= {located(source['into'], values).resolve() for source in config.get('source', [])}
        removed(build, kept)
    print('cleaned %s' % build.as_posix())


if __name__ == '__main__':
    main()
