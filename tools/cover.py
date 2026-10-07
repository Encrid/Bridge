#!/usr/bin/env python3
"""Every thing Bridge-ASM700 exposes has a case named for it, and every case is of one.

    python tools/cover.py              hold docs/tests.md against the headers and the cases
    python tools/cover.py --surface    print what the headers expose, a name a line

What is exposed is read from the headers of the three seams and of the two SDKs: a function, an
entry, a macro that takes arguments, a constant, a layout, and each key of a struct a host fills.
docs/tests.md lists each under a heading of its name, with the cases that show it; a case is
named for its heading, then a hyphen, then what it shows.

It fails on an exposed name with no heading, a heading with no case, a heading in a seam's part
that the headers do not expose, a heading there is twice, a case under no heading or under one
it is not named for, a case listed that is not run, and a document that points at a case that
is not run.
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
LIST = ROOT / 'docs/tests.md'
SEAMS = {
    'The host seam': ['include/bridge.h'],
    'The module seam': ['include/bridge_abi.h', 'include/bridge_schema.h'] +
                       ['backends/%s/sdk/%s' % (b, h) for b, hs in [
                           ('wasm', 'bridge_wasm bridge_wasm_acc bridge_wasm_entity bridge_wasm_kept '
                                    'bridge_wasm_log bridge_wasm_observer bridge_wasm_scratch '
                                    'bridge_wasm_span bridge_wasm_toggle'),
                           ('native', 'bridge_native bridge_native_api bridge_native_kept '
                                      'bridge_native_log')] for h in (n + '.h' for n in hs.split())],
    'The backend seam': ['include/bridge_backend.h'],
}
# a struct a host fills is exposed key by key: each is stated, and each is refused in its own
# words. Any other struct is a layout, and is exposed whole.
BY_KEY = 'include/bridge.h'


def strip(text):
    """The text with no comment, and with every continued line joined."""
    text = re.sub(r'/\*.*?\*/', '', text, flags=re.S)
    return text.replace('\\\n', ' ')


def keys(name, body):
    """The members of a struct, as `struct.member`. A member named pad is padding, one named
    for a count is said with the array it counts, and one that is a struct of keys is said by
    its own."""
    short = re.sub(r'^bridge_|_t$', '', name)
    found = re.findall(r'(\w+) \**(\w+)(?:\[[^\]]*\])?\s*;', body)
    return ['%s.%s' % (short, m) for kind, m in found
            if not re.match(r'pad\d*$', m) and not m.endswith('_count')
            and kind not in ('bridge_module_config_t', 'bridge_budget_t')]


def entries(body):
    """The members of a struct that are functions: the entries of a table."""
    return re.findall(r'\(\*(\w+)\)\(', body)


def constants(text):
    """The constants a header states: each member of an enum, and each constexpr."""
    names = []
    for body in re.findall(r'enum \w+(?: : \w+)? \{(.*?)\}', text, re.S):
        names += re.findall(r'(\w+)(?: = [^,}]+)?\s*(?:,|$)', body.strip())
    return names + re.findall(r'^constexpr [\w ]+?(\w+)(?:\[\])? =', text, re.M)


def exposed(path):
    """Every name a header exposes."""
    text = strip((ROOT / path).read_text(encoding='utf-8'))
    names = []
    for kind, body, name in re.findall(r'typedef (struct|union)\s*\w*\s*\{(.*?)\}\s*(\w+);', text, re.S):
        by_key = path == BY_KEY and kind == 'struct'
        names += entries(body) or (keys(name, body) if by_key else [name])
    for name, body in re.findall(r'^struct (\w+) \{(.*?)\};', text, re.S | re.M):
        names += entries(body) + [name + '_t']
    names += constants(text)
    names += re.findall(r'^\s*#define (\w+)\(', text, re.M)
    names += re.findall(r'^(?:BRIDGE_API|static inline)[^;{]*?(\w+)\(', text, re.S | re.M)
    return [n for n in names if not n.endswith('_')]


def surface():
    """What each seam exposes, in the order the headers state it."""
    return {seam: list(dict.fromkeys(n for h in headers for n in exposed(h)))
            for seam, headers in SEAMS.items()}


def cases():
    """The names of the cases build.py runs."""
    sys.path.insert(0, str(ROOT))
    import build
    return [c[0] for c in build.CASES]


def listed():
    """docs/tests.md, as its parts: each a list of headings, each with its lines."""
    parts, part, head = {}, None, None
    for line in LIST.read_text(encoding='utf-8').splitlines():
        if line.startswith('## '):
            part, head = parts.setdefault(line[3:].strip(), {}), None
        elif line.startswith('### ') and part is not None:
            head = part.setdefault(line[4:].strip(), [])
            head.append(None) if len(head) else None
        elif head is not None and re.match(r'    \S+$', line):
            head.append(line.strip())
    return parts


def pointed():
    """Every case a document points at, as case `name`, with the document."""
    found = []
    for doc in sorted((ROOT / 'docs').rglob('*.md')):
        if 'plan' in doc.parts:
            continue
        text = re.sub(r'\n(?=\S)', ' ', doc.read_text(encoding='utf-8'))
        for run in re.findall(r'\bcases? ((?:`[^`\s]+`(?:,? and |, )?)+)', text):
            found += [(doc.name, name) for name in re.findall(r'`([^`]+)`', run)]
    return found


def check_headings(parts, said):
    """Holds each seam's part against its headers, and no heading twice."""
    seen = []
    for seam, names in surface().items():
        part = parts.get(seam, {})
        said += ['%s is exposed, and has no heading in "%s"' % (n, seam) for n in names if n not in part]
        said += ['"%s" has the heading %s, which no header of it exposes' % (seam, h)
                 for h in part if h not in names]
    for part in parts.values():
        said += ['the heading %s is there twice' % h for h in part if h in seen or None in part[h]]
        seen += list(part)


def check_cases(heads, run, said):
    """Holds what is under each heading against the cases that are run."""
    for head, under in heads.items():
        mine = [u for u in under if u]
        said += ['%s has no case' % head] if not mine else []
        said += ['%s is listed under %s, and is not named for it' % (c, head)
                 for c in mine if not c.startswith(head + '-') and c != head]
        said += ['%s is listed, and no case of that name is run' % c for c in mine if c not in run]
    everywhere = [c for under in heads.values() for c in under if c]
    said += ['%s is run, and is under no heading' % c for c in run if c not in everywhere]
    said += ['%s is listed twice' % c for c in sorted(set(everywhere)) if everywhere.count(c) > 1]


def check():
    said, run, parts = [], cases(), listed()
    heads = {h: c for part in parts.values() for h, c in part.items()}
    check_headings(parts, said)
    check_cases(heads, run, said)
    said += ['%s points at the case %s, and no case of that name is run' % (doc, c)
             for doc, c in pointed() if c not in run]
    for line in said:
        print(line)
    print('%d exposed names, %d headings, %d cases: %d faults'
          % (sum(len(n) for n in surface().values()), len(heads), len(run), len(said)))
    return 1 if said else 0


if __name__ == '__main__':
    if sys.argv[1:] == ['--surface']:
        for seam, names in surface().items():
            print('%s: %d' % (seam, len(names)))
            for n in names:
                print('    ' + n)
        sys.exit(0)
    sys.exit(check())
