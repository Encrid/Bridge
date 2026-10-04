#!/usr/bin/env python3
"""Runs of eight words appearing in more than one document.

Finds a document re-arguing a conclusion that lives elsewhere, which is how two
copies drift until neither is clearly wrong. One shared passage is one hit:
overlapping runs over the same pair of files are grown into the longest run and
reported once, so the count is passages and not windows. Run from docs/.

Not every hit is a defect. A signature stated in both ABI documents, a shared
boilerplate line, and two documents citing one source are all fine. Read the
hits; do not chase the number to zero.
"""
import collections
import glob
import os
import re

N = 8
files = glob.glob('**/*.md', recursive=True)
grams = collections.defaultdict(set)
words_of = {}

for f in files:
    text = open(f, encoding='utf-8').read().lower()
    text = re.sub(r'[`*_#\[\]()]', ' ', text)
    words = re.findall(r"[a-z0-9']+", text)
    words_of[f] = words
    for i in range(len(words) - (N - 1)):
        grams[' '.join(words[i:i + N])].add(f)

# grow each shared window rightwards while every file still shares it, then keep
# only the windows nothing longer already covers. a passage quoted in two places
# is one hit rather than len(passage) - 7 of them.
out = {}
for f, words in sorted(words_of.items()):
    i, n = 0, len(words)
    while i <= n - N:
        fs = grams[' '.join(words[i:i + N])]
        if len(fs) < 2:
            i += 1
            continue
        end = i + N
        while end < n and grams[' '.join(words[end - N + 1:end + 1])] >= fs:
            end += 1
        out[' '.join(words[i:end])] = fs
        i = end - N + 1

runs = sorted(out, key=lambda x: -len(x))
runs = [g for g in runs if not any(g in o and g != o for o in runs)]

for g in sorted(runs, key=lambda x: sorted(out[x])):
    names = ','.join(x.replace(os.sep, '/').replace('.md', '') for x in sorted(out[g]))
    print('%-32s %s' % (names, g))
print('\n%d duplicated passages' % len(runs))
