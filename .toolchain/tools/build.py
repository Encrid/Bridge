# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import os
import platform
import shutil
import zipfile
from pathlib import Path

import fetch
import shared
import take
from shared import die, expand, expanded, files, known, located, run, sha256


def tools(config, values):
    for name, path in config.get('tools', {}).items():
        found = shutil.which(located(path, values)) or die('the tool %s is not at %s' % (name, expand(path, values)))
        values[name] = Path(found).as_posix()


def leb(n):
    return bytes((n >> shift & 0x7f) | (0x80 if n >> shift + 7 else 0) for shift in range(0, max(n.bit_length(), 1), 7))


def do(step, values, unstamped):
    root = Path(values['root'])
    if 'run' in step:
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


def put(path, data):
    if not path.is_file() or path.read_bytes() != data:
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)


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
    sources = files(Path(values['root']), [expand(pattern, mine) for pattern in artifact['sources']])
    if not sources:
        die('no file matches the sources of %s' % name)
    output = Path(values['out']) / artifact.get('folder', '') / expand(kind['output'], mine)
    work = Path(values['out']) / 'obj' / name
    mine.update(sources=[moved(source, tree) for source in sources], output=output.as_posix(), work=work.as_posix(),
                defines=['-D' + define for define in expanded(artifact.get('defines', []), mine)],
                include=['-I' + moved(folder, tree) for folder in expanded(artifact.get('include', []), mine)],
                flags=expanded(artifact.get('flags', []), mine),
                link=expanded(artifact.get('link', []), mine), manifest=expanded(artifact.get('manifest', []), mine))
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
        put(output.parent / file.name, file.read_bytes())
    return name, [output] + [output.parent / file.name for file in besides]


def packaged(config, values, made):
    out = Path(values['out'])
    lines = ['package %s' % shared.project_name(config), 'target %s' % values['target']]
    lines += ['%s  %s' % (sha256(path.read_bytes()), path.relative_to(out).as_posix()) for path in dict.fromkeys(made)]
    out.mkdir(parents=True, exist_ok=True)
    (out / 'package.txt').write_text('\n'.join(lines) + '\n', encoding='utf-8', newline='\n')


def packed(config, values):
    out, root = Path(values['out']), Path(values['root'])
    names = {name: out / name for name in ['package.txt'] + list(take.listed(out / 'package.txt')[1])}
    names.update((name, root / name) for name in files(root, ['include/**/*.h', 'license', 'third_party/*/license']))
    zipped = Path(values['build']) / ('%s-%s.zip' % (shared.project_name(config), values['target']))
    with zipfile.ZipFile(zipped, 'w', zipfile.ZIP_DEFLATED) as packing:
        for name, path in names.items():
            packing.write(path, name)
    print(zipped.as_posix())
    print('\n'.join(names))


def artifacts(config):
    for artifact in config.get('artifact', []):
        yield artifact
        for variant in artifact.get('variants', []):
            yield dict(artifact, **variant)


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
    artifact = {'name': 'proof', 'kind': 'program', 'sources': [proof],
                'include': [(clean / 'include').as_posix()],
                'link': [(clean / name).as_posix() for name in archives(config, values)]}
    _, made = built(config, dict(values, out=(clean / 'host').as_posix()), artifact)
    if values['target'] == shared.this_target(config['targets']):
        run([made[0].as_posix()], values['root'])
    print('package proof: %s %s' % (shared.project_name(config),
          'ran' if values['target'] == shared.this_target(config['targets']) else 'built'))


def prepared(args, here=False):
    config = shared.config(args.root)
    home = shared.this_target(config['targets'])
    target = args.target or home or die('no target runs on %s %s: name one' % (platform.system(), platform.machine()))
    if target not in config['targets']:
        die('no target %s; the targets: %s' % (target, ' '.join(config['targets'])))
    if here and target != home:
        die('%s does not run here: build it, and test it where it runs' % target)
    cache = shared.HERE / config['build']['cache']
    os.environ['ZIG_GLOBAL_CACHE_DIR'] = str(cache / 'global')
    os.environ['ZIG_LOCAL_CACHE_DIR'] = str(cache / 'local')
    zig = fetch.zig_found(config['zig']) or die('zig %s is neither fetched nor on the path' % config['zig']['version'])
    return config, shared.variables(config, args, target, zig)


def ready(args, here=False):
    config, values = prepared(args, here)
    fetch.sources(config, values, False)
    tools(config, values)
    take.take(config, values)
    return config, values


def made(config, values, tree=None, part=None):
    done, shipped = [], []
    for artifact in artifacts(config):
        if part is not None and artifact.get('part') not in (None, part):
            continue
        done.append(built(config, values, artifact, tree))
        shipped += [] if artifact.get('test') else done[-1][1]
    packaged(config, values, shipped)
    return done


def build(args, here=False):
    config, values = ready(args, here)
    return config, values, made(config, values, part=args.part)


def main():
    args = shared.arguments(pack='--pack')
    config, values, made = build(args)
    print('\n'.join(paths[0].as_posix() for _, paths in made if paths))
    if args.pack:
        packed(config, values)
        prove(config, values)


if __name__ == '__main__':
    main()
