# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import hashlib
import re
import shutil
import subprocess
import urllib.request
import zipfile
from pathlib import Path

import shared
import take
from shared import die, expand, located


def zig_found(zig):
    fetched = [p for p in (shared.HERE / zig['folder']).glob('*/zig*') if p.name in ('zig', 'zig.exe')]
    path = str(fetched[0]) if fetched else shutil.which('zig')
    version = path and subprocess.run([path, 'version'], capture_output=True, text=True).stdout.strip()
    return Path(path).as_posix() if version == zig['version'] else None


def fetched(url, digest, folder):
    shutil.rmtree(folder, ignore_errors=True)
    folder.mkdir(parents=True)
    path = folder / url.rsplit('/', 1)[1]
    with urllib.request.urlopen(url) as got, open(path, 'wb') as f:
        shutil.copyfileobj(got, f)
    with open(path, 'rb') as f:
        found = hashlib.file_digest(f, 'sha256').hexdigest()
    if found != digest:
        path.unlink()
        die('%s is not what the maker publishes: its sha256 is %s' % (path.name, found))
    unpacked(path, folder)
    path.unlink()


def unpacked(path, folder):
    if path.suffix != '.zip':
        return shutil.unpack_archive(path, folder)
    with zipfile.ZipFile(path) as zipped:
        for member in zipped.infolist():
            made = Path(zipped.extract(member, folder))
            if member.external_attr >> 16 & 0o111:
                made.chmod(made.stat().st_mode | member.external_attr >> 16 & 0o111)


def fetch_zig(zig, home):
    archive = zig['archives'].get(home) or die('zig is fetched on a machine that runs a target')
    url = zig['url'].format(version=zig['version'], host=archive['host'], format=archive['format'])
    fetched(url, archive['sha256'], shared.HERE / zig['folder'])
    return zig_found(zig) or die('the archive holds no zig %s' % zig['version'])


def hunks(part):
    lines = part.split('\n')
    for at, line in enumerate(lines):
        m = re.match(r'@@ -(\d+)(?:,(\d+))? \+\d+(?:,(\d+))? @@', line)
        old, new = [], []
        for body in lines[at + 1:] if m else []:
            if len(old) == int(m.group(2) or 1) and len(new) == int(m.group(3) or 1):
                break
            if not body.startswith('\\'):
                old += [body[1:]] if body[:1] in ('', ' ', '-') else []
                new += [body[1:]] if body[:1] in ('', ' ', '+') else []
        if m:
            yield int(m.group(1)) - 1, old, new


def patched(tree, patch):
    for part in re.split(r'(?m)^diff --git ', patch.read_text(encoding='utf-8').replace('\r\n', '\n'))[1:]:
        name = re.search(r'(?m)^\+\+\+ b/(.+)$', part).group(1)
        lines, shift = (tree / name).read_bytes().decode('utf-8').split('\n'), 0
        for at, old, new in hunks(part):
            if lines[at + shift:at + shift + len(old)] != old:
                die('%s does not apply to %s' % (patch.as_posix(), name))
            lines[at + shift:at + shift + len(old)] = new
            shift += len(new) - len(old)
        (tree / name).write_bytes('\n'.join(lines).encode('utf-8'))


def sources(config, values, fetching):
    for source in config.get('source', []):
        shared.known(source, 'source', 'a source')
        into, patches = located(source['into'], values), [located(p, values) for p in source.get('patches', [])]
        tree, stamp = into / expand(source.get('top', ''), values), expand(source['sha256'], values)
        values[source['name']] = tree.as_posix()
        stamp = ' '.join([stamp] + [shared.sha256(patch.read_bytes()) for patch in patches])
        if (into / 'fetched.txt').is_file() and (into / 'fetched.txt').read_text(encoding='utf-8') == stamp:
            continue
        if not fetching:
            die('%s is not fetched as project.toml states: run fetch' % source['name'])
        url = expand(source['url'], values)
        fetched(url if '://' in url else located(url, values).as_uri(), stamp.split(' ')[0], into)
        for patch in patches:
            patched(tree, patch)
        (into / 'fetched.txt').write_text(stamp, encoding='utf-8')


def main():
    args = shared.arguments()
    config = shared.config(args.root)
    home = shared.this_target(config['targets']) or die('fetch runs on a machine of a target')
    zig = zig_found(config['zig']) or fetch_zig(config['zig'], home)
    print(zig)
    values = shared.variables(config, args, home, zig)
    sources(config, values, True)
    take.take(config, values)


if __name__ == '__main__':
    main()
