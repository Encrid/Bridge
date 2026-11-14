# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import sys

from shared import arguments, config, die, expand, this_target


def installs(needs, system, declared):
    keys = {'linux': 'packages', 'darwin': 'brews'}
    if system not in keys:
        return []
    named = sorted(value for name in needs
                   for value in declared[name].get(system, {}).get(keys[system], []))
    if not named:
        return []
    if system == 'linux':
        return ['sudo apt-get update', 'sudo apt-get install -y ' + ' '.join(named)]
    return ['brew install ' + ' '.join(named)]


def unpack(archive, folder, file, members, system):
    depths = sorted({member.count('/') for member in members})
    if len(depths) > 1:
        die('the members of the archive into %s drop different counts of folders' % folder)
    parts = ['--strip-components=%d' % depths[0]] if members and depths[0] else []
    parts += members
    tail = ''.join(' ' + part for part in parts)
    if system == 'windows':
        return ['/c/Windows/System32/tar.exe -xf "$(cygpath -w "$%s/%s")" -C "$(cygpath -w "$%s")"%s'
                % (folder, file, folder, tail)]
    return ['tar -xf "$%s/%s" -C "$%s"%s' % (folder, file, folder, tail)]


def folder_lines(folder, system):
    if system == 'windows':
        return ['%s="$(cygpath -u "$RUNNER_TEMP")/%s"' % (folder, folder),
                'mkdir -p "$%s"' % folder]
    return ['%s="$RUNNER_TEMP/%s"' % (folder, folder), 'mkdir -p "$%s"' % folder]


def archive_lines(archive, values, system, folder):
    for key in ('url', 'sha256'):
        if key not in archive:
            die('an archive into %s has no %s' % (folder, key))
    url = expand(archive['url'], values)
    file = expand(archive.get('file', url.rsplit('/', 1)[-1]), values)
    members = [expand(member, values) for member in archive.get('members', [])]
    lines = ['curl -fsSL -o "$%s/%s" %s' % (folder, file, url)]
    lines.append('echo "%s  $%s/%s" | sha256sum -c' % (expand(archive['sha256'], values), folder, file))
    return lines + unpack(archive, folder, file, members, system)


def need_lines(name, system, cpu, declared):
    table = declared[name].get(system, {})
    values = dict(table.get('values', {}).get(cpu, {}))
    lines, seen = [], set()
    for archive in table.get('archive', []):
        folder = archive.get('into', name)
        if not folder.isidentifier():
            die('the archive into %s is not a name of a shell variable' % folder)
        if folder not in seen:
            lines += folder_lines(folder, system)
            seen.add(folder)
        lines += archive_lines(archive, values, system, folder)
    return lines + [expand(command, values) for command in table.get('commands', [])]


def commands(needs, machine, settings):
    system, cpu = machine[0].lower(), machine[1]
    declared = settings.get('need', {})
    lines = installs(needs, system, declared)
    for name in needs:
        lines += need_lines(name, system, cpu, declared)
    return lines


def main():
    args = arguments()
    settings = config(args.root)
    target = args.target or this_target(settings['targets'])
    machine = settings['targets'].get(target, {}).get('runs_on') or die('no machine runs the target %s' % target)
    needs = settings.get('ci', {}).get('needs', [])
    declared = settings.get('need', {})
    for need in needs:
        if need not in declared:
            die('[ci] needs: %s is not a need: %s' % (need, ' '.join(sorted(declared))))
    sys.stdout.buffer.write(''.join(line + '\n' for line in commands(needs, machine, settings)).encode('utf-8'))


if __name__ == '__main__':
    main()
