# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import concurrent.futures
import os
import re
import shutil
import socket
import subprocess
import threading
import tomllib
from pathlib import Path

import branches
import build
import fail
import leaks
import rewrite
import shared
from said import checked
from shared import die, expand, expanded, files, known, located, status


def environment(changes, values):
    env = dict(os.environ)
    for name, value in changes.items():
        env.pop(name, None)
        if value is not False:
            env[name] = expand(str(value), values)
    return env


def free_port():
    with socket.socket() as free:
        free.bind(('127.0.0.1', 0))
        return free.getsockname()[1]


def started(argv, root, timeout, env=None):
    proc = subprocess.Popen(argv, cwd=root, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, env=env)
    late = threading.Event()

    def stop():
        late.set()
        proc.kill()
    watch = threading.Timer(timeout, stop)
    watch.start()
    return {'proc': proc, 'late': late, 'watch': watch, 'said': []}


def waited(running, text):
    for line in iter(running['proc'].stdout.readline, b''):
        running['said'].append(line)
        if text.encode('utf-8') in line:
            return True
    return False


def typed(running, text):
    try:
        running['proc'].stdin.write(text.encode('utf-8'))
        running['proc'].stdin.flush()
    except OSError:
        pass


def ended(running, text):
    typed(running, text)
    try:
        running['proc'].stdin.close()
    except OSError:
        pass
    running['said'].append(running['proc'].stdout.read())
    code = running['proc'].wait()
    running['watch'].cancel()
    said = b''.join(running['said']).decode('utf-8', 'replace').replace('\r', '')
    return None if running['late'].is_set() else code, said


def ordered(wanted, lines):
    rest = iter(lines)
    return all(any(line == one.strip() for one in rest) for line in wanted)


def holding(path, text):
    return path.is_file() and text in path.read_text(encoding='utf-8')


def same(path, said):
    return path.is_file() and path.read_text(encoding='utf-8') == said


def holds(claims, lacks, code, said, values):
    lines = said.split('\n')
    known(claims, 'claims', 'a case')
    return all([code == claims.get('status', code), code != claims.get('status_not', None),
                all(expand(line, values) in lines for line in claims.get('line', [])),
                all(expand(text, values) in said for text in claims.get('has', [])),
                all(expand(text, values) not in said for text in claims.get('lacks', []) + lacks),
                all(re.search(expand(text, values), said, re.S) for text in claims.get('matches', [])),
                not any(re.search(expand(text, values), said, re.S) for text in claims.get('matches_not', [])),
                ordered([expand(line, values) for line in claims.get('order', [])], lines),
                'output' not in claims or same(located(claims['output'], values), said),
                all(holding(located(path, values), expand(text, values))
                    for path, text in claims.get('files', {}).items())])


def source_of(item, outputs, values):
    name = expand(item, values)
    return outputs[name] if name in outputs else Path(values['root']) / name


def placed(use, outputs, values):
    for folder, held in use.get('folders', {}).items():
        located(folder, values).mkdir(parents=True, exist_ok=True)
        for item in held:
            source = source_of(item, outputs, values)
            shutil.copy(source, located(folder, values) / source.name)
    for path, text in use.get('files', {}).items():
        located(path, values).parent.mkdir(parents=True, exist_ok=True)
        located(path, values).write_text(expand(text, values), encoding='utf-8', newline='\n')


def scripted(running, script, outputs, values):
    for step in script:
        if not waited(running, expand(known(step, 'script', 'a case')['wait'], values)):
            return
        for path, item in step.get('put', {}).items():
            shutil.copy(source_of(item, outputs, values), located(path, values))
        for path in step.get('remove', []):
            located(path, values).unlink()
        if 'type' in step:
            typed(running, expand(step['type'], values) + '\n')


def made_false(case):
    false = case.get('false', {})
    merged = dict(case, **false)
    for key in ('files', 'folders'):
        merged[key] = dict(case.get(key, {}), **false.get(key, {}))
    return merged


def tried(config, values, outputs, where, case, broken, counted=False, failing=None):
    use = made_false(case) if broken else case
    values, outputs = failing if failing and use.get('fail') else (values, outputs)
    folder = Path(values['out']) / 'cases' / where
    shutil.rmtree(folder, ignore_errors=True)
    folder.mkdir(parents=True, exist_ok=True)
    mine = dict(values, case=folder.as_posix())
    if '{port}' in str(use):
        mine['port'] = free_port()
    counting = {'C23_LEAKS': folder.as_posix() + '.leaks'} if counted else {}
    plan = ''.join(' %s=%s' % pair for pair in use.get('fail', {}).items()) or False
    placed(use, outputs, mine)
    root, timeout, helper = values['root'], config['test']['timeout'], use.get('helper')
    beside = started(expanded(helper['run'], mine), root, timeout, environment(counting, mine)) if helper else None
    if helper:
        waited(beside, expand(helper['wait'], mine))
    running = started(expanded(use['run'], mine), root, timeout,
                      environment({**counting, 'C23_FAIL': plan, **use.get('env', {})}, mine))
    scripted(running, use.get('script', []), outputs, mine)
    ends = [ended(running, expand(use.get('stdin', ''), mine))]
    if helper:
        ends.append(ended(beside, expand(helper.get('stdin', ''), mine)))
    return ends, [use.get('claims', {})] + ([helper.get('claims', {})] if helper else []), mine


def judged(config, group, broken, run):
    ends, claims, mine = run
    lacks = [rule['lacks'] for rule in config['test'].get('rule', []) if group not in rule.get('exempt', [])]
    if any(code is None for code, _ in ends):
        return False, 'did not end in %d seconds' % config['test']['timeout'], ends
    held = all(holds(claim, lacks, code, said, mine) for claim, (code, said) in zip(claims, ends))
    if broken:
        return not held, 'still as written' if held else 'fails, as it must', ends
    return held, 'as written' if held else 'differs', ends


def verdict(config, values, outputs, group, index, case, broken, copies, counted=False, failing=None):
    count = 1 if case.get('alone') else copies
    where = '%s/%d' % (group, index)
    wheres = [where] if count == 1 else ['%s/%d' % (where, copy) for copy in range(count)]
    with concurrent.futures.ThreadPoolExecutor(count) as pool:
        runs = list(pool.map(lambda where: tried(config, values, outputs, where, case, broken, counted, failing),
                             wheres))
    verdicts = [judged(config, group, broken, run) for run in runs]
    bad = [one for one in verdicts if not one[0]]
    if not bad:
        return verdicts[0]
    ok, how, ends = bad[0]
    return ok, how + (' in %d of %d copies' % (len(bad), count) if count > 1 else ''), ends


def test(config, values, outputs, groups, broken, said, copies, counted=False, failing=None):
    wrong, texts = [], []
    for group, cases in groups:
        right = 0
        for index, case in enumerate(cases):
            ok, how, ends = verdict(config, values, outputs, group, index, case, broken, copies, counted, failing)
            texts += [said for _, said in ends]
            print('%s%s: %s' % (case['name'], ', made false' if broken else '', how))
            right += ok
            if not ok:
                wrong.append(case['name'])
                for whose, (code, said) in zip(('', 'the helper '), ends):
                    print('  %sended with %s, and printed:\n%s' % (whose, 'no status' if code is None else status(code),
                                                                  said.rstrip()))
        print('%s: %d claims made false: %d cases failed' % (group, len(cases), right) if broken else
              '%s: %d cases: %d as written' % (group, len(cases), right))
    if said:
        (Path(values['out']) / 'said.txt').write_text('\n'.join(texts), encoding='utf-8', newline='\n')
    return wrong


def groups_of(config, values, folder):
    root, found, calls = Path(values['root']), [], fail.listed(config, values)
    for path in files(root, [folder + '/**/*.toml']):
        cases = tomllib.loads((root / path).read_text(encoding='utf-8')).get('case', [])
        for case in cases:
            known(case, 'case', path)
            known(case.get('false', {}), 'false', path)
            known(case.get('helper', {}), 'helper', path)
            if not case.get('claims'):
                die('%s: the case %s claims nothing' % (path, case['name']))
            if 'false' not in case:
                die('%s: the case %s has no false variant' % (path, case['name']))
            for call in {**case.get('fail', {}), **case['false'].get('fail', {})}:
                if call not in calls:
                    die('%s: the case %s makes %s fail, and [fail.calls] does not list it' % (path, case['name'], call))
        found.append((Path(path).relative_to(folder).with_suffix('').as_posix(), cases))
    return [(group, cases) for group, cases in found if cases]


def chosen(groups, part):
    found = [(group, [case for case in cases if part in case['name']]) for group, cases in groups]
    return [(group, cases) for group, cases in found if cases] or die('no case has %s in its name' % part)


def changed(args, run, write, prepared=None):
    config, values = prepared or build.ready(args, True)
    values = dict(values, out=(Path(values['build']) / run / values['target']).as_posix())
    written = write(config, values)
    made = build.made(config, values, rewrite.tree(values, run))
    unlinked = [name for name, paths in made if not paths]
    if unlinked:
        die('the cases need what is linked: ' + ' '.join(unlinked))
    groups = groups_of(config, values, config.get('test', {}).get('cases', 'tests/cases'))
    groups = chosen(groups, args.k) if args.k and groups else groups
    return config, values, written, {name: paths[0] for name, paths in made}, groups


def measured(args):
    if args.broken or args.toolchain or args.copies or args.rounds:
        die('--branches runs each case one time: it takes no --break, --toolchain, --copies or --rounds')
    config, values, (probes, gaps), outputs, groups = changed(args, 'branches', branches.probed)
    hits = Path(values['out']) / 'hits.bin'
    hits.write_bytes(bytes(branches.parts(probes)[1]))
    os.environ['C23_BRANCHES'] = str(hits)
    test(config, values, outputs, groups, False, False, 1)
    branches.reported(probes, gaps, hits, config.get('branches', {}).get('exception', []), values['system'])


def tallied(args):
    if args.broken or args.toolchain or args.copies or args.rounds or args.branches:
        die('--leaks runs each case one time: it takes no --break, --toolchain, --copies, --rounds or --branches')
    config, values, (files, kinds), outputs, groups = changed(args, 'leaks', leaks.counted)
    folder = Path(values['out']) / 'cases'
    shutil.rmtree(folder, ignore_errors=True)
    test(config, values, outputs, groups, False, False, 1, True)
    cases = [(case['name'], folder / group / ('%d.leaks' % index)) for group, listed in groups
             for index, case in enumerate(listed)]
    leaks.reported(files, kinds, cases, config.get('leaks', {}).get('exception', []), values['system'])


def main():
    args = shared.arguments('-k', ('--copies', int), ('--rounds', int), broken='--break', toolchain='--toolchain',
                            branches='--branches', leaks='--leaks')
    if args.leaks:
        return tallied(args)
    if args.branches:
        return measured(args)
    if args.toolchain:
        config, values = build.prepared(args, True)
        made = []
    else:
        config, values, made = build.build(args, True)
    unlinked = [name for name, paths in made if not paths]
    if unlinked:
        die('the cases need what is linked: ' + ' '.join(unlinked))
    folder = '.toolchain/tests/cases' if args.toolchain else config.get('test', {}).get('cases', 'tests/cases')
    groups = groups_of(config, values, folder)
    if not groups:
        return print('no case: %s holds no case' % folder)
    groups = chosen(groups, args.k) if args.k else groups
    copies = args.copies or config['test'].get('copies') or die('.toolchain/project.toml: write [test] copies')
    own = not (args.broken or args.toolchain or args.k)
    rounds = args.rounds or 1
    failing = None
    if any((made_false(case) if args.broken else case).get('fail') for _, cases in groups for case in cases):
        _, mine, _, outputs, _ = changed(args, 'fail', fail.failed, (config, values))
        failing = mine, outputs
    for done in range(rounds):
        if rounds > 1:
            print('round %d of %d' % (done + 1, rounds))
        wrong = test(config, values, {name: paths[0] for name, paths in made}, groups, args.broken, own, copies,
                     failing=failing)
        if wrong:
            die(('still as written with their claims made false: ' if args.broken else 'not as written: ')
                + ', '.join(wrong))
    if own:
        checked(Path(values['root']), Path(values['out']))


if __name__ == '__main__':
    main()
