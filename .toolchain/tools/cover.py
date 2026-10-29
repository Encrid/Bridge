# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import re

from shared import arguments, config, document, ours, report


def named(name, text):
    return re.search(r'(?<!\w)%s(?!\w)' % re.escape(name), text) is not None


def inside(body):
    if ';' in body:
        return [pointer or field for pointer, field in re.findall(r'\(\s*\*\s*(\w+)\s*\)|(\w+)\s*(?:\[[^\]]*\]\s*)*;', body)]
    return [item.split('=')[0].strip() for item in body.split(',') if item.strip()]


def names(text, makes):
    text = re.sub(r'\[\[.*?\]\]\s*', '', text.replace('\\\n', ' '))
    found = re.findall(r'^\s*#\s*define\s+(\w+)\(', text, re.M)
    for name, body in re.findall(r'^\s*#\s*define\s+(\w+)\s+\{(.*)\}\s*$', text, re.M):
        found += [name] + inside(body)
    found += [name + makes[macro] for macro, name in re.findall(r'^\s*(\w+)\s*\(\s*(\w+)', text, re.M) if macro in makes]
    found += re.findall(r'^(?!typedef\b)[A-Za-z_][^;{}()#=\n]*?\b(\w+)\s*\(', text, re.M)
    found += re.findall(r'\bconstexpr\b[^;=]*?\b(\w+)\s*(?:\[[^\]]*\]\s*)*=', text)
    for body in re.findall(r'\benum\b[^{;]*\{([^}]*)\}', text):
        found += inside(body)
    for tag, body, name in re.findall(r'\b(?:struct|union)\s*(\w*)\s*\{([^}]*)\}\s*(\w*)', text):
        found += [name or tag] + inside(body)
    return list(dict.fromkeys(found))


def spans(root, doc):
    return '\n'.join(re.findall(r'`([^`\n]+)`', (root / doc).read_text(encoding='utf-8'))) if (root / doc).is_file() else ''


def main():
    root = arguments().root
    makes = config(root).get('check', {}).get('makes', {})
    headers = [path for path in sorted(ours(root)) if path.startswith('include/') and path.endswith('.h')]
    found = []
    for path in headers:
        doc, source = document(path), (root / path).read_text(encoding='utf-8')
        text = spans(root, doc)
        found += ['%s: %s is not in %s' % (path, name, doc) for name in names(source, makes) if not named(name, text)]
    report(found, len(headers))


if __name__ == '__main__':
    main()
