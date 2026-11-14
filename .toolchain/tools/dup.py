# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import collections
import fnmatch
import re

import prose
from shared import arguments, config, report

CITATION = re.compile(r'`[^`\n]*\.md`(?:,\s*\*[^*\n]+\*)?')
PUNCTUATION = re.compile(r'[^a-z0-9]')
JOIN = '\x01'


def unwrap(line):
    return prose.SPAN.sub(lambda m: JOIN + re.sub(r'[\s|]+', JOIN, m.group()[1:-1]) + JOIN,
                          CITATION.sub(' ', line))


def words(sentence):
    found = []
    for token in prose.WORD.findall(sentence):
        parts = [part for part in (PUNCTUATION.sub('', piece.lower()) for piece in token.split(JOIN)) if part]
        found += [(part, ' '.join(parts) if JOIN in token else '') for part in parts]
    return found


def units(lines):
    text = list(lines)
    for start, end, _ in prose.blocks(lines):
        text[start:end + 1] = [''] * (end + 1 - start)
    found = []
    for n, unit, _ in prose.units([unwrap(line) for line in text]):
        for m in prose.SENTENCE.finditer(unit):
            pairs = words(m.group())
            found.append((n, [word for word, _ in pairs], [span for _, span in pairs]))
    return found


def quoted(places):
    spans = [next(iter(found)) if len(found) == 1 else '' for found in places.values()]
    return all(spans) and len(set(spans)) == len(spans)


def shared_runs(units_by_file, size):
    grams = collections.defaultdict(dict)
    for name, found in sorted(units_by_file.items()):
        for line, words, held in found:
            for i in range(len(words) - size + 1):
                span = held[i] if len(set(held[i:i + size])) == 1 else ''
                grams[' '.join(words[i:i + size])].setdefault((name, line), set()).add(span)

    def shared(words, i):
        return len({name for name, _ in grams[' '.join(words[i:i + size])]}) > 1

    runs = {}
    for name, found in sorted(units_by_file.items()):
        for line, words, _ in found:
            i = 0
            while i <= len(words) - size:
                if not shared(words, i):
                    i += 1
                    continue
                end = i
                while end < len(words) - size and shared(words, end + 1):
                    end += 1
                places = {}
                for k in range(i, end + 1):
                    for place, spans in grams[' '.join(words[k:k + size])].items():
                        places.setdefault(place, set()).update(spans)
                run = ' '.join(words[i:end + size])
                runs[(run, tuple(sorted(places)))] = places
                i = end + 1
    kept = [(run, set(places)) for (run, _), places in runs.items() if not quoted(places)]
    return [(run, sorted(places)) for run, places in kept
            if not any(run != other and ' %s ' % run in ' %s ' % other and places <= more for other, more in kept)]


def excused(places, exceptions):
    names = {name for name, _ in places}
    found = False
    for exception in exceptions:
        if all(any(fnmatch.fnmatch(name, path) for path in exception['paths']) for name in names):
            exception['excused'] += 1
            found = True
    return found


def exceptions(settings):
    listed = settings['dup'].get('exception', [])
    return [{'paths': entry.get('paths', []), 'why': entry.get('why', ''), 'excused': 0} for entry in listed]


def stale(exceptions):
    found = []
    for exception in exceptions:
        named = ', '.join(exception['paths'])
        if not exception['why']:
            found.append("the exception for '%s' states no reason" % named)
        if not exception['excused']:
            found.append("the exception for '%s' excuses nothing" % named)
    return found


def scopes(documents):
    toolchain = {path: lines for path, lines in documents if path.startswith('.toolchain/')}
    return [{path: lines for path, lines in documents if path not in toolchain}, toolchain]


def main():
    root = arguments().root
    settings = config(root)
    documents = prose.documents(root)
    listed = exceptions(settings)
    faults = []
    for scope in scopes(documents):
        found = shared_runs({path: units(lines) for path, lines in scope.items()}, settings['dup']['words'])
        faults += ['%s: %s' % (', '.join('%s:%d' % place for place in places), run)
                   for run, places in sorted(found, key=lambda item: item[1]) if not excused(places, listed)]
    report(faults + stale(listed), len(documents), 'documents')


if __name__ == '__main__':
    main()
