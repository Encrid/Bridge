# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import ast
import io
import re
import tokenize

import build
import fetch
from shared import arguments, config, die, files, header, located, ours, report

QUOTED = re.compile(r'"""[\s\S]*?"""|\'\'\'[\s\S]*?\'\'\'|"(?:\\.|[^"\\\n])*"|\'[^\'\n]*\'|#')


def notice():
    return re.compile(header('#'))


def python(text, notice):
    start = notice.match(text)
    last = text.count('\n', 0, start.end()) if start else 0
    found = [] if start else [(1, 'the file does not start with the legal notice')]
    found += [(token.start[0], 'a comment') for token in tokenize.generate_tokens(io.StringIO(text).readline)
              if token.type == tokenize.COMMENT and token.start[0] > last]
    return found + [(node.lineno, 'a string statement') for node in ast.walk(ast.parse(text))
                    if isinstance(node, ast.Expr) and isinstance(node.value, ast.Constant)
                    and isinstance(node.value.value, str)]


def faults(path, data, notice, unicode):
    text = data.decode('utf-8', 'replace')
    found = [] if unicode else [(n, 'a character that is not ASCII')
                                for n, line in enumerate(data.split(b'\n'), 1) if not line.isascii()]
    if path.endswith('.py'):
        found += python(text, notice)
    if path.endswith(('.toml', '.yml')):
        found += [(text.count('\n', 0, m.start()) + 1, 'a comment') for m in QUOTED.finditer(text) if m.group() == '#']
    if path.endswith('.md'):
        found += [(n, 'a comment') for n, line in enumerate(text.split('\n'), 1) if '<!--' in line]
    return found


def retired(text, words):
    found = []
    for word, new in words.items():
        for m in re.finditer(r'\b%s\b' % re.escape(word), text, re.IGNORECASE):
            what = "'%s' is retired: write '%s'" % (m.group(), new) if new else "'%s' is retired" % m.group()
            found.append((text.count('\n', 0, m.start()) + 1, what))
    return found


def refused(args, settings):
    lists = settings.get('check', {}).get('refuse', [])
    if not lists:
        return []
    _, values = build.prepared(args)
    fetch.sources(settings, values, False)
    names = []
    for written in lists:
        path = located(written, values)
        if not path.is_file():
            die('[check] refuse: %s is not there' % path.as_posix())
        names += [(name.strip(), written) for name in path.read_text(encoding='utf-8').splitlines() if name.strip()]
    return names


def called(text, names):
    return [(text.count('\n', 0, m.start()) + 1, "'%s' is refused by %s" % (name, written))
            for name, written in names for m in re.finditer(r'(?<!\w)%s *\(' % re.escape(name), text)]


def main():
    args = arguments()
    root = args.root
    settings = config(root)
    paths = ours(root)
    unicode = set(files(root, settings.get('check', {}).get('unicode', [])))
    words = settings.get('check', {}).get('retired', {})
    worded = [path for path in ours(root, tests=True) if not path.startswith('.toolchain/')]
    names = refused(args, settings)
    code = [path for path in paths if path.startswith(('src/', 'include/')) and path.endswith(('.c', '.h'))]
    legal = notice()
    found = sorted([(path, n, what) for path in paths
                    for n, what in faults(path, (root / path).read_bytes(), legal, path in unicode)]
                   + [(path, n, what) for path in worded
                      for n, what in retired((root / path).read_text(encoding='utf-8', errors='replace'), words)]
                   + [(path, n, what) for path in code
                      for n, what in called((root / path).read_text(encoding='utf-8', errors='replace'), names)])
    report(['%s:%d: %s' % fault for fault in found], len(set(paths) | set(worded)))


if __name__ == '__main__':
    main()
