# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import re

from shared import arguments, config, document, ours, report

GUIDES = ('agreement', 'coding_style', 'commits', 'decisions', 'documentation_style', 'needs', 'pull_request', 'tests')
NEEDED = (('.github/workflows/ci.yml', '.github/workflows/release.yml', '.github/workflows/cla.yml', '.gitattributes',
           '.gitignore', 'cla.md', 'license', 'project.py', '.toolchain/project.toml', 'readme.md')
          + tuple('docs/contributing/%s.md' % guide for guide in GUIDES))
FOLDERS = ('.toolchain/tools',)
DOCUMENT = r'docs/(?:extern|intern|tools)/.+\.md'
ALLOWED = (r'architecture\.md|' + DOCUMENT + r'|include/.+\.h|src/.+|tools/.+'
           r'|\.toolchain/tools/[^/]+\.py|\.toolchain/toolchain\.toml|\.toolchain/config/[^/]+\.toml'
           r'|\.toolchain/header\.txt|\.toolchain/docs/[^/]+\.md|\.toolchain/presets/[^/]+/[^/]+\.toml'
           r'|(?:docs/extern|docs/intern|include)/\.gitkeep')
SNAKE = re.compile(r'[a-z0-9_]+(?:\.[a-z0-9]+)*')
DOTTED = ('.github', '.gitattributes', '.gitignore', '.gitkeep', '.toolchain')
MADE = re.compile(r'.*\.(?:o|obj|a|so|dll|dylib|exe|gch|d|pyc)|.*~|#.*#|a\.out')


def guides(root):
    if not (root / '.toolchain' / 'project.toml').is_file():
        return ()
    return tuple('docs/contributing/%s' % name for name in config(root).get('check', {}).get('guides', []))


def faults(root, paths):
    allowed = re.compile('|'.join(map(re.escape, NEEDED + guides(root))) + '|' + ALLOWED)
    found = ['%s: the layout needs this file' % path for path in NEEDED if path not in paths]
    found += ['%s/: the layout needs this folder' % folder for folder in FOLDERS
              if not any(path.startswith(folder + '/') for path in paths)]
    described = {}
    for path in paths:
        if path.startswith('third_party/'):
            continue
        if not allowed.fullmatch(path):
            found.append('%s: the layout does not allow this file' % path)
        if MADE.fullmatch(path.rsplit('/', 1)[-1]):
            found.append('%s: a compiled or temporary file' % path)
        found += ["%s: '%s' is not in snake_case" % (path, name) for name in path.split('/')
                  if name not in DOTTED and not SNAKE.fullmatch(name)]
        mirror = document(path)
        if mirror:
            described[mirror] = path
            if mirror not in paths:
                found.append('%s: no document %s' % (path, mirror))
    found += ['%s: no file has this document' % path for path in paths
              if re.fullmatch(DOCUMENT, path) and path not in described]
    for mirror in described.keys() & paths:
        if (root / mirror).read_text(encoding='utf-8', errors='replace').split('\n')[0] != '# ' + described[mirror]:
            found.append('%s: the first line is not # %s' % (mirror, described[mirror]))
    return found


def main():
    root = arguments().root
    paths = ours(root)
    report(sorted(faults(root, set(paths))), len(paths))


if __name__ == '__main__':
    main()
