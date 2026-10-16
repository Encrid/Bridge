# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import re

from shared import die, expanded, located, run, sha256

DIGEST = re.compile(r'[0-9a-f]{64}')


def listed(path):
    keys, digests = {}, {}
    for line in path.read_text(encoding='utf-8').splitlines():
        head, _, rest = line.partition(' ')
        if DIGEST.fullmatch(head):
            digests[rest.strip()] = head
        elif head:
            keys[head] = rest.strip()
    return keys, digests


def copies(found):
    out = []
    for taken in found.values():
        for file, digest in taken['digests'].items():
            source, copy = taken['source'] / file, taken['into'] / file
            if copy.is_file() and sha256(copy.read_bytes()) == digest:
                continue
            if not source.is_file():
                die('%s is not there' % source.as_posix())
            if sha256(source.read_bytes()) != digest:
                die('refused: %s is not what %s states' % (source.as_posix(), taken['listing'].as_posix()))
            out.append((copy, source.read_bytes()))
    return out


def take(config, values):
    found = {}
    for dependency in config.get('dependency', []):
        if 'make' in dependency:
            run(expanded(dependency['make'], values), values['root'])
        source, into = located(dependency['from'], values), located(dependency['into'], values)
        listing = source / dependency['list']
        values[dependency['name']] = into.as_posix()
        if not listing.is_file() and not dependency.get('optional'):
            die('%s is not there' % listing.as_posix())
        if not listing.is_file():
            print('not taken: %s is not there' % listing.as_posix())
            continue
        keys, digests = listed(listing)
        found[dependency['name']] = {'source': source, 'listing': listing, 'into': into, 'keys': keys,
                                     'digests': digests}
    for rule in config.get('agree', []):
        said = {key: found[key.split('.')[0]]['keys'].get(key.split('.')[1], '')[:rule['length']]
                for key in rule['keys'] if key.split('.')[0] in found}
        if len(set(said.values())) > 1:
            die('refused: the dependencies do not agree: ' + ', '.join('%s %s' % pair for pair in said.items()))
    for path, data in copies(found):
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)
    for name, taken in found.items():
        print('taken: %s, %s' % (name, ', '.join('%s %s' % pair for pair in taken['keys'].items())))
