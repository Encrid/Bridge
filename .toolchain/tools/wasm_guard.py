# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import sys
from pathlib import Path

TRUNCATED = 'ends inside a section'


def leb(data, at):
    result = 0
    shift = 0
    while True:
        byte = data[at]
        at += 1
        result |= (byte & 0x7f) << shift
        if not byte & 0x80:
            return result, at
        shift += 7


def name(data, at):
    length, at = leb(data, at)
    return data[at:at + length].decode('utf-8', 'replace'), at + length


def refuse(path, why):
    sys.stderr.write('%s is refused: it %s\n' % (path, why))
    sys.exit(1)


def guard(path):
    data = Path(path).read_bytes()
    if data[:4] != b'\x00asm':
        refuse(path, 'is not a wasm module')
    try:
        guard_imports(path, data)
    except IndexError:
        refuse(path, TRUNCATED)


def guard_imports(path, data):
    at = 8
    while at < len(data):
        section = data[at]
        size, at = leb(data, at + 1)
        if section != 2:
            at += size
            if at > len(data):
                refuse(path, TRUNCATED)
            continue
        count, at = leb(data, at)
        for _ in range(count):
            module, at = name(data, at)
            field, at = name(data, at)
            kind = data[at]
            at += 1
            if kind == 0:
                _, at = leb(data, at)
            elif kind == 1:
                flags = data[at + 1]
                _, at = leb(data, at + 2)
                if flags & 0x01:
                    _, at = leb(data, at)
            elif kind == 2:
                refuse(path, "imports the memory '%s.%s'; a module reaches shared "
                    "data only through the accessors" % (module, field))
            elif kind == 3:
                refuse(path, "imports the global '%s.%s'; a module imports no global"
                    % (module, field))
            else:
                refuse(path, "imports the tag '%s.%s'; a module imports no tag"
                    % (module, field))
        return


if __name__ == '__main__':
    guard(sys.argv[1])
