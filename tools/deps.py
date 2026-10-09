"""What the built library and its two backends ask of the system they run on.

    python tools/deps.py <built directory>             every import, by artifact
    python tools/deps.py <built directory> --check     fail on one docs/design/dependencies.md
                                                       does not list, and on one it lists
                                                       that nothing imports

Read from the artifacts themselves: the import table of a PE file, the dynamic symbols of an
ELF file, the undefined symbols of a Mach-O file. A function the compiler's own runtime
provides is linked into the artifact and is not an import, so it is not here.
"""
import fnmatch
import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ARTIFACTS = ('bridge',)
LISTED = re.compile(r'    (\w+)\s+(\S+)')
SYSTEMS = {'windows': ('windows', 'every'), 'linux': ('linux', 'posix', 'every'),
           'macos': ('macos', 'posix', 'every')}
# the processor an artifact is for, by the number its format has for it
PROCESSORS = {0x8664: 'x86_64', 0xaa64: 'aarch64', 62: 'x86_64', 183: 'aarch64',
              0x01000007: 'x86_64', 0x0100000c: 'aarch64'}


def text(data, at):
    return data[at:data.index(b'\0', at)].decode('ascii')


def pe(data):
    """(library, function) for every entry of the import directory."""
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
    """('', function) for every undefined dynamic symbol."""
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
    """('', function) for every undefined external symbol."""
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
    """The system and the processor an artifact is for, and what it imports."""
    data = path.read_bytes()
    if data[:2] == b'MZ':
        machine, = struct.unpack_from('<H', data, struct.unpack_from('<I', data, 0x3c)[0] + 4)
        return 'windows', PROCESSORS[machine], pe(data)
    if data[:4] == b'\x7fELF':
        return 'linux', PROCESSORS[struct.unpack_from('<H', data, 18)[0]], elf(data)
    return 'macos', PROCESSORS[struct.unpack_from('<I', data, 4)[0]], macho(data)


def listed(kinds):
    """What docs/design/dependencies.md lists under one of these first words."""
    document = (ROOT / 'docs/design/dependencies.md').read_text(encoding='utf-8')
    return {m.group(2) for m in map(LISTED.match, document.split('\n'))
            if m and m.group(1) in kinds}


def compare(asked, known, what):
    """What is asked for and not listed, and what is listed and not asked for. A listed name
    may hold `*`."""
    unlisted = sorted(a for a in asked if not any(fnmatch.fnmatchcase(a, k) for k in known))
    unused = sorted(k for k in known if not any(fnmatch.fnmatchcase(a, k) for a in asked))
    for name in unlisted:
        print('%s: %s is not listed in docs/design/dependencies.md' % (what, name))
    for name in unused:
        print('%s: %s is listed, and nothing asks for it' % (what, name))
    return len(unlisted) + len(unused)


def main():
    built = Path(sys.argv[1])
    if built.name.endswith('-debug') and '--check' in sys.argv:
        # dependencies.md, *What the built artifacts import*: the list is of what ships
        print('a debug build is not what ships: what it imports is not held to the list')
        return
    asked, system, processor = set(), None, None
    for name in ARTIFACTS:
        path = next(p for p in built.glob(name + '.*') if p.suffix in ('.dll', '.so', '.dylib'))
        system, processor, found = imports(path)
        asked |= {function for _, function in found}
        if '--check' not in sys.argv:
            for library, function in sorted(found):
                print('%-24s %-28s %s' % (name, library, function))
    if '--check' not in sys.argv:
        return
    tools = set(re.findall(r"tool\('([\w-]+)'", (ROOT / 'build.py').read_text(encoding='utf-8')))
    one = '%s_%s' % (system, processor)
    wrong = compare(asked, listed(SYSTEMS[system] + (one,)), 'an import on ' + one)
    wrong += compare(tools, listed(('tool',)), 'a tool of build.py')
    if wrong:
        sys.exit('FAIL: %d dependencies out of step with the list' % wrong)
    print('%d imports and %d tools: each is listed, and each one listed is asked for'
          % (len(asked), len(tools)))


if __name__ == '__main__':
    main()
