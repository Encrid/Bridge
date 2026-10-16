# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import collections
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
RUN = 8


def words_of(path):
    text = re.sub(r'[`*_#\[\]()]', ' ', path.read_text(encoding='utf-8').lower())
    return re.findall(r"[a-z0-9']+", text)


def shared_runs(words_by_file):
    grams = collections.defaultdict(set)
    for name, words in words_by_file.items():
        for i in range(len(words) - (RUN - 1)):
            grams[' '.join(words[i:i + RUN])].add(name)
    found = {}
    for name, words in sorted(words_by_file.items()):
        i, count = 0, len(words)
        while i <= count - RUN:
            files = grams[' '.join(words[i:i + RUN])]
            if len(files) < 2:
                i += 1
                continue
            end = i + RUN
            while end < count and grams[' '.join(words[end - RUN + 1:end + 1])] >= files:
                end += 1
            found[' '.join(words[i:end])] = files
            i = end - RUN + 1
    return found


def main():
    paths = sorted(ROOT.glob('docs/**/*.md'))
    found = shared_runs({p.relative_to(ROOT).as_posix(): words_of(p) for p in paths})
    runs = sorted(found, key=lambda run: -len(run))
    runs = [run for run in runs if not any(run in other and run != other for other in runs)]
    for run in sorted(runs, key=lambda run: sorted(found[run])):
        print('%s  %s' % (','.join(sorted(found[run])), run))
    print('%d passages are in more than one document' % len(runs))


if __name__ == '__main__':
    main()
