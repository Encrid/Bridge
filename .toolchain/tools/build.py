# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import graphlib
import os
import platform
import shutil
import stat
import zipfile
from pathlib import Path

import fetch
import shared
import take
from shared import die, expand, expanded, files, known, located, run, sha256

CACHES = {'ZIG_GLOBAL_CACHE_DIR': 'global', 'ZIG_LOCAL_CACHE_DIR': 'local'}


def tools(config, values):
    for name, path in config.get('tools', {}).items():
        found = shutil.which(located(path, values)) or die('the tool %s is not at %s' % (name, expand(path, values)))
        values[name] = Path(found).as_posix()


def leb(n):
    return bytes((n >> shift & 0x7f) | (0x80 if n >> shift + 7 else 0) for shift in range(0, max(n.bit_length(), 1), 7))


def run_in(step, values):
    root, folder = Path(values['root']), located(step['cwd'], values)
    if not folder.is_dir():
        die('%s is not there, and a step of %s runs in it' % (folder.as_posix(), values['name']))
    whole = dict(values, sources=[(root / path).as_posix() for path in values['sources']],
                 include=['-I' + (root / path[2:]).as_posix() for path in values['include']])
    caches = {name: str(root / os.environ[name]) for name in CACHES}
    run(expanded(step['run'], whole), folder, dict(os.environ, **caches))


def do(step, values, unstamped):
    root = Path(values['root'])
    if 'run' in step and 'cwd' in step:
        run_in(step, values)
    elif 'run' in step:
        run(expanded(step['run'], values), values['root'])
    if 'section' in step and not unstamped:
        name = step['section'].encode('utf-8')
        content = leb(len(name)) + name + expand(step['text'], values).encode('utf-8')
        with open(values['output'], 'ab') as f:
            f.write(b'\0' + leb(len(content)) + content)
    if 'contains' in step and expand(step['contains'], values).encode('utf-8') not in Path(values['output']).read_bytes():
        die('%s does not hold %r' % (values['output'], expand(step['contains'], values)))
    if 'copy' in step:
        shutil.copyfile(*[root / path for path in expanded(step['copy'], values)])
    if 'rename' in step:
        os.replace(*[root / path for path in expanded(step['rename'], values)])


def put(path, source):
    data = source.read_bytes()
    if not path.is_file() or path.read_bytes() != data:
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)
    path.chmod(stat.S_IMODE(source.stat().st_mode) | stat.S_IWUSR)


def moved(path, tree):
    if tree and path.split('/', 1)[0] in ('src', 'include'):
        return (Path(tree) / path).as_posix()
    return path


def planned(config, values, artifact, tree=None):
    kind = config['kinds'][artifact['kind']]
    mine = dict(values)
    if 'target' in artifact:
        mine.update(config['targets'][artifact['target']], target=artifact['target'])
    name = expand(artifact['name'], mine)
    mine['name'] = name
    sources = files(Path(values['root']), expanded(artifact['sources'], mine))
    if not sources:
        die('no file matches the sources of %s' % name)
    output = Path(values['out']) / artifact.get('folder', '') / expand(kind['output'], mine)
    work = Path(values['out']) / 'obj' / name
    mine.update(sources=[moved(source, tree) for source in sources], output=output.as_posix(), work=work.as_posix(),
                defines=['-D' + define for define in expanded(artifact.get('defines', []), mine)],
                include=['-I' + moved(folder, tree) for folder in expanded(artifact.get('include', []), mine)],
                link=expanded(artifact.get('link', []), mine), manifest=expanded(artifact.get('manifest', []), mine))
    mine['flags'] = expanded(kind.get('flags', []) + artifact.get('flags', []), mine)
    return kind, mine


def built(config, values, artifact, tree=None):
    kind, mine = planned(config, values, artifact, tree)
    name, output, work = mine['name'], Path(mine['output']), Path(mine['work'])
    output.parent.mkdir(parents=True, exist_ok=True)
    work.parent.mkdir(parents=True, exist_ok=True)
    missing = [file for file in mine['link'] if not Path(file).is_file()]
    if missing and 'unlinked' not in kind:
        die('%s is not there, and %s is linked with it' % (missing[0], name))
    if missing:
        print('compiled, not linked: %s: %s is not there' % (name, missing[0]))
    for step in kind['unlinked' if missing else 'steps']:
        do(known(step, 'step', artifact['kind']), mine, artifact.get('unstamped', False))
    if missing:
        return name, []
    besides = [Path(file) for file in expanded(artifact.get('beside', []), mine)]
    for file in besides:
        put(output.parent / file.name, file)
    return name, [output] + [output.parent / file.name for file in besides]


def packaged(config, values, made):
    out = Path(values['out'])
    lines = ['package %s' % shared.project_name(config), 'target %s' % values['target']]
    lines += ['%s  %s' % (sha256(path.read_bytes()), path.relative_to(out).as_posix()) for path in dict.fromkeys(made)]
    out.mkdir(parents=True, exist_ok=True)
    (out / 'package.txt').write_text('\n'.join(lines) + '\n', encoding='utf-8', newline='\n')


def leaked(values, kit):
    folders = {Path(values['root']).resolve(), Path(values['build']).resolve()}
    forms = sorted({form for folder in folders for form in (folder.as_posix(), str(folder))})
    for name in kit:
        data = (Path(values['dist']) / name).read_bytes()
        for form in forms:
            if form.encode('utf-8') in data:
                die('%s holds %s, a path of the machine that built it' % (name, form))


def packed(config, values, kit):
    leaked(values, kit)
    dist = Path(values['dist'])
    zipped = Path(values['build']) / ('%s-%s.zip' % (shared.project_name(config), values['target']))
    with zipfile.ZipFile(zipped, 'w', zipfile.ZIP_DEFLATED) as packing:
        for name in kit:
            packing.write(dist / name, name)
    print(zipped.as_posix())
    print('\n'.join(kit))


def artifacts(config):
    defaults, found = config.get('module', {}), []
    for row in [dict(defaults, **module) for module in config.get('modules', [])] + config.get('artifact', []):
        found += [row] + [dict(row, **variant) for variant in row.get('variants', [])]
    rank = ranked(config)
    return sorted(found, key=lambda artifact: rank.get(artifact.get('part'), 0))


def ranked(config):
    uses = {part: table.get('uses', []) for part, table in config.get('parts', {}).items()}
    try:
        return {part: rank for rank, part in enumerate(graphlib.TopologicalSorter(uses).static_order())}
    except graphlib.CycleError as cycle:
        die('the uses of the parts make a cycle: %s' % ' '.join(cycle.args[1]))


def path_defines(config):
    found = ['BUILT_DIR="{out}/built"', 'BUILT_SUFFIX="{library}"']
    for artifact in artifacts(config):
        folder = artifact.get('folder')
        if artifact.get('kind') != 'library' or folder not in ('built', 'backends'):
            continue
        if folder == 'built' and not artifact.get('path'):
            continue
        macro = artifact['name'].removeprefix('bridge_').upper().replace('-', '_') + '_PATH'
        found.append('%s="{out}/%s/%s{library}"' % (macro, folder, artifact['name']))
    return found


def archives(config, values):
    out = Path(values['out'])
    found = []
    for artifact in artifacts(config):
        if not artifact.get('test') and artifact['kind'] == 'archive':
            found.append(Path(planned(config, values, artifact)[1]['output']).relative_to(out))
    return found


def prove(config, values):
    proof = config.get('package', {}).get('proof')
    if not proof:
        return
    clean = Path(values['out']) / 'proof'
    shutil.rmtree(clean, ignore_errors=True)
    zipped = Path(values['build']) / ('%s-%s.zip' % (shared.project_name(config), values['target']))
    with zipfile.ZipFile(zipped) as packing:
        packing.extractall(clean)
    included = clean / 'include'
    includes = [included.as_posix()]
    includes += sorted(sub.as_posix() for sub in included.rglob('*') if sub.is_dir())
    includes += expanded(config['package'].get('include', []), values)
    artifact = {'name': 'proof', 'kind': 'program', 'sources': [proof],
                'include': includes,
                'link': [(clean / name).as_posix() for name in archives(config, values)]
                + expanded(config['package'].get('link', []), values)}
    _, made = built(config, dict(values, out=(clean / 'host').as_posix()), artifact)
    if values['target'] == shared.this_target(config['targets']):
        run([made[0].as_posix()], values['root'])
    print('package proof: %s %s' % (shared.project_name(config),
          'ran' if values['target'] == shared.this_target(config['targets']) else 'built'))


def composed(config, values):
    dist, out, root, kit = Path(values['dist']), Path(values['out']), Path(values['root']), []
    for artifact in artifacts(config):
        if artifact.get('test'):
            continue
        _, mine = planned(config, values, artifact)
        outs = [Path(mine['output'])] + [Path(file) for file in expanded(artifact.get('beside', []), mine)]
        for path in outs:
            put(dist / path.name, path)
            kit.append(path.name)
    for table in config.get('compose', {}).values():
        if not isinstance(table, dict):
            continue
        start, into = located(table.get('from', ''), values), expand(table.get('into', ''), values)
        for name in files(start, table.get('headers', [])):
            put(dist / into / name, start / name)
            kit.append((Path(into) / name).as_posix())
        for asset in table.get('assets', []):
            put(dist / Path(asset).name, out / asset)
            kit.append(Path(asset).name)
    named = ['third_party/%s/license' % folder for folder in config.get('third_party', ['*'])]
    missing = [name for name in named if '*' not in name and not (root / name).is_file()]
    if missing:
        die('%s is not there, and the deliverable needs it' % missing[0])
    for name in files(root, ['license'] + named):
        put(dist / name, root / name)
        kit.append(name)
    kit = list(dict.fromkeys(kit))
    print('\n'.join(kit))
    return kit


def part_table(config, part):
    parts = config.get('parts', {})
    for name in [part] + parts.get(part, {}).get('uses', []):
        if name not in parts:
            die('no part %s; the parts: %s' % (name, ' '.join(parts)))
    table = known(parts[part], 'part', '[parts.%s]' % part)
    missing = [key for key in ('name', 'uses', 'third_party') if key not in table]
    if missing:
        die('[parts.%s] states no %s' % (part, missing[0]))
    return table


def owned(config):
    parts = config.get('parts', {})
    for part in parts:
        part_table(config, part)
    tables = [(artifact['name'], artifact) for artifact in artifacts(config)]
    tables += [('[compose.%s]' % name, table) for name, table in config.get('compose', {}).items()
               if isinstance(table, dict)]
    for name, table in tables:
        if parts and 'part' not in table:
            die('%s states no part, and the project has parts' % name)
        if 'part' in table and table['part'] not in parts:
            die('%s states the part %s, and [parts] does not have it' % (name, table['part']))


def needed(config, parts):
    found, waiting = set(), list(parts)
    while waiting:
        part = waiting.pop()
        if part not in found:
            found.add(part)
            waiting += part_table(config, part)['uses']
    return found


def kept(config, parts):
    return dict(config, module={}, modules=[],
                artifact=[dict(artifact, variants=[]) for artifact in artifacts(config) if artifact.get('part') in parts],
                compose={name: table for name, table in config.get('compose', {}).items()
                         if isinstance(table, dict) and table.get('part') in parts})


def viewed(config, part):
    table = part_table(config, part)
    return dict(kept(config, {part}), project={'name': table['name']}, package=table.get('package', {}),
                third_party=table['third_party'])


def views(config, parts):
    return [viewed(config, part) for part in parts or config.get('parts', {})] or [config]


def joined(args):
    config = shared.config(args.root)
    target = args.target or shared.this_target(config['targets']) or die('no target runs here: name one')
    build = Path(shared.variables(config, args, target, '')['build'])
    for part in args.part or []:
        name = part_table(config, part)['name']
        zips = {each: build / ('%s-%s.zip' % (name, each)) for each in config['targets']}
        missing = [each for each, zipped in zips.items() if not zipped.is_file()]
        if missing:
            die('%s is not there: pack %s for %s first' % (zips[missing[0]].as_posix(), part, missing[0]))
        archive = build / (name + '.zip')
        with zipfile.ZipFile(archive, 'w', zipfile.ZIP_DEFLATED) as joining:
            for each, zipped in zips.items():
                with zipfile.ZipFile(zipped) as packing:
                    for member in packing.infolist():
                        copy = zipfile.ZipInfo('%s/%s' % (each, member.filename), member.date_time)
                        copy.external_attr = member.external_attr
                        joining.writestr(copy, packing.read(member), zipfile.ZIP_DEFLATED)
        print(archive.as_posix())


def prepared(args, here=False):
    config = shared.config(args.root)
    home = shared.this_target(config['targets'])
    target = args.target or home or die('no target runs on %s %s: name one' % (platform.system(), platform.machine()))
    if target not in config['targets']:
        die('no target %s; the targets: %s' % (target, ' '.join(config['targets'])))
    if here and target != home:
        die('%s does not run here: build it, and test it where it runs' % target)
    cache, root = (shared.HERE / config['build']['cache']).resolve(), Path(args.root).resolve()
    cache = cache.relative_to(root) if cache.is_relative_to(root) else cache
    os.environ.update({name: str(cache / leaf) for name, leaf in CACHES.items()})
    zig = fetch.zig_found(config['zig']) or die('zig %s is neither fetched nor on the path' % config['zig']['version'])
    values = shared.variables(config, args, target, zig)
    owned(config)
    values['paths'] = path_defines(config)
    return (kept(config, needed(config, args.part)) if args.part else config), values


def ready(args, here=False):
    config, values = prepared(args, here)
    fetch.sources(config, values, False)
    tools(config, values)
    take.take(config, values)
    return config, values


def made(config, values, tree=None, tests=True):
    done, shipped = [], []
    for artifact in artifacts(config):
        if not tests and artifact.get('test'):
            continue
        done.append(built(config, values, artifact, tree))
        shipped += [] if artifact.get('test') else done[-1][1]
    packaged(config, values, shipped)
    return done


def build(args, here=False, tests=True):
    config, values = ready(args, here)
    return config, values, made(config, values, tests=tests)


def main():
    args = shared.arguments(pack='--pack', join='--join')
    if args.join:
        return joined(args)
    config, values, made = build(args, tests=False)
    print('\n'.join(paths[0].as_posix() for _, paths in made if paths))
    for view in views(config, args.part):
        kit = composed(view, values)
        if args.pack:
            packed(view, values, kit)
            prove(view, values)


if __name__ == '__main__':
    main()
