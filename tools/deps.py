# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import fnmatch
import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
LIST = ROOT / 'docs' / 'extern' / 'os' / 'os.h.md'
SYSTEMS = {'windows': ('windows', 'every'), 'linux': ('linux', 'posix', 'every'),
           'macos': ('macos', 'posix', 'every')}
PROCESSORS = {0x8664: 'x86_64', 0xaa64: 'aarch64', 62: 'x86_64', 183: 'aarch64',
              0x01000007: 'x86_64', 0x0100000c: 'aarch64'}


def text(data, at):
    return data[at:data.index(b'\0', at)].decode('ascii')


def pe(data):
    header = struct.unpack_from('<I', data, 0x3c)[0]
    sections = struct.unpack_from('<H', data, header + 6)[0]
    optional = header + 24
    table = optional + struct.unpack_from('<H', data, header + 20)[0]
    rows = [struct.unpack_from('<IIII', data, table + 40 * i + 8) for i in range(sections)]

    def offset(rva):
        for size, at, _, raw in rows:
            if at <= rva < at + size:
                return rva - at + raw
        raise ValueError('an address outside every section')

    found = set()
    at = offset(struct.unpack_from('<I', data, optional + 120)[0])
    while True:
        lookup, _, _, name, first = struct.unpack_from('<IIIII', data, at)
        if not name:
            return found
        library = text(data, offset(name)).lower()
        entry = offset(lookup or first)
        while True:
            value = struct.unpack_from('<Q', data, entry)[0]
            if not value:
                break
            if not value >> 63:
                found.add((library, text(data, offset(value & 0x7fffffff) + 2)))
            entry += 8
        at += 20


def elf(data):
    start, = struct.unpack_from('<Q', data, 0x28)
    size, count = struct.unpack_from('<HH', data, 0x3a)
    heads = [struct.unpack_from('<IIQQQQIIQQ', data, start + size * i) for i in range(count)]
    found = set()
    for head in heads:
        if head[1] != 11:
            continue
        names = heads[head[6]][4]
        for at in range(head[4], head[4] + head[5], 24):
            name, _, _, section = struct.unpack_from('<IBBH', data, at)
            if name and not section:
                found.add(('', text(data, names + name)))
    return found


def macho(data):
    commands, = struct.unpack_from('<I', data, 16)
    found, at = set(), 32
    for _ in range(commands):
        kind, size = struct.unpack_from('<II', data, at)
        if kind == 2:
            symbols, count, names = struct.unpack_from('<III', data, at + 8)
            for entry in range(symbols, symbols + 16 * count, 16):
                name, flags = struct.unpack_from('<IB', data, entry)
                if flags & 0x0f == 1:
                    found.add(('', text(data, names + name).removeprefix('_')))
        at += size
    return found


def imports(path):
    data = path.read_bytes()
    if data[:2] == b'MZ':
        system, read = 'windows', pe
        machine, = struct.unpack_from('<H', data, struct.unpack_from('<I', data, 0x3c)[0] + 4)
    elif data[:4] == b'\x7fELF':
        system, read = 'linux', elf
        machine, = struct.unpack_from('<H', data, 18)
    else:
        system, read = 'macos', macho
        machine, = struct.unpack_from('<I', data, 4)
    if machine not in PROCESSORS:
        sys.exit('deps.py: %s is for a processor that PROCESSORS does not map: 0x%x' % (path.as_posix(), machine))
    return system, PROCESSORS[machine], read(data)


def listed(kinds):
    if not LIST.is_file():
        sys.exit('deps.py: %s is not there' % LIST.relative_to(ROOT).as_posix())
    document = LIST.read_text(encoding='utf-8')
    block = re.search(r'^## The imports\n.*?^```text\n(.*?)^```', document, re.M | re.S)
    if not block:
        sys.exit('deps.py: %s has no block of text under ## The imports' % LIST.relative_to(ROOT).as_posix())
    rows = [line.split() for line in block.group(1).split('\n') if line.strip()]
    return {row[1] for row in rows if row[0] in kinds}


def compare(asked, known, what):
    unlisted = sorted(a for a in asked if not any(fnmatch.fnmatchcase(a, k) for k in known))
    unused = sorted(k for k in known if not any(fnmatch.fnmatchcase(a, k) for a in asked))
    for name in unlisted:
        print('%s: %s is not in the list' % (what, name))
    for name in unused:
        print('%s: %s is in the list, and nothing imports it' % (what, name))
    return len(unlisted) + len(unused)


def main():
    folders = [argument for argument in sys.argv[1:] if argument != '--check']
    if not folders:
        sys.exit('deps.py: name the folder of a build')
    built = Path(folders[0])
    check = '--check' in sys.argv[1:]
    if built.name.endswith('-debug') and check:
        return print('a debug build is not what ships: its imports are not held to the list')
    found = [p for p in sorted(built.glob('bridge.*')) if p.suffix in ('.dll', '.so', '.dylib')]
    if not found:
        sys.exit('deps.py: no library bridge in %s' % built.as_posix())
    system, processor, imported = imports(found[0])
    if not check:
        for library, function in sorted(imported):
            print('%-24s %-28s %s' % ('bridge', library, function))
        return
    asked = {function for _, function in imported}
    one = '%s_%s' % (system, processor)
    if compare(asked, listed(SYSTEMS[system] + (one,)), 'an import on ' + one):
        sys.exit('deps.py: the imports of the library and the list differ')
    print('%d imports: each is in the list, and each one in the list is imported' % len(asked))


if __name__ == '__main__':
    main()
