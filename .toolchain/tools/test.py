# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import concurrent.futures
import fnmatch
import os
import platform
import re
import shutil
import socket
import stat
import subprocess
import threading
import time
import tomllib
from pathlib import Path

import branches
import build
import fail
import leaks
import locks
import rewrite
import scope
import shared
import signals
from said import checked
from shared import die, expand, expanded, files, known, located, status


def environment(changes, values):
    env = dict(os.environ)
    for name, value in changes.items():
        env.pop(name, None)
        if value is not False:
            env[name] = expand(str(value), values)
    return env


def bound(kind, port):
    with socket.socket(socket.AF_INET, kind) as free:
        try:
            free.bind(('127.0.0.1', port))
        except OSError:
            return 0
        return free.getsockname()[1]


def free_port():
    port = 0
    while port == 0:
        port = bound(socket.SOCK_STREAM, bound(socket.SOCK_DGRAM, 0))
    return port


def watched(timeout, stop):
    watch = threading.Timer(timeout, stop)
    watch.start()
    return watch


def started(argv, root, timeout, env=None, use=None):
    proc = signals.started(argv, root, env, use or {})
    late = threading.Event()

    def stop():
        late.set()
        proc.kill()
    return {'proc': proc, 'late': late, 'watch': watched(timeout, stop), 'said': [],
            'again': lambda: watched(timeout, stop)}


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


def fed(running, text):
    typed(running, text)
    try:
        running['proc'].stdin.close()
    except OSError:
        pass


def paced(running):
    for line in iter(running['proc'].stdout.readline, b''):
        running['said'].append(line)
        if not line.strip():
            running['watch'].cancel()
            running['watch'] = running['again']()


def ended(running, text, rows=False):
    feed = threading.Thread(target=fed, args=(running, text))
    feed.start()
    if rows:
        paced(running)
    if not running['proc'].stdout.closed:
        running['said'].append(running['proc'].stdout.read())
    feed.join()
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


def claimed(claims, lacks, code, answer, values):
    rest = {key: claim for key, claim in claims.items() if key != 'output'}
    return holds(rest, lacks, code, answer, values) \
        and ('output' not in claims or same(located(claims['output'], values), answer + '\n'))


def source_of(item, outputs, values):
    name = expand(item, values)
    return outputs[name] if name in outputs else Path(values['root']) / name


def cleared(folder):
    if not folder.exists():
        return
    for path in folder.rglob('*'):
        if os.name == 'nt' and path.is_file() and not os.access(path, os.W_OK):
            os.chmod(path, stat.S_IWRITE)
    shutil.rmtree(folder)


def put(source, target, values):
    target.unlink(missing_ok=True)
    if Path(source).resolve().is_relative_to(Path(values['build'])):
        try:
            os.link(source, target)
            return True
        except OSError:
            pass
    shutil.copy(source, target)
    return False


def stamp(path):
    try:
        found = os.stat(path)
    except OSError:
        return None
    return found.st_size, found.st_mtime_ns


def sources_of(item, outputs, values):
    name = expand(item, values)
    if '*' not in name:
        return [source_of(item, outputs, values)]
    found = sorted(path for path in Path(values['root']).glob(name) if path.is_file())
    return found or shared.die('%s: no file matches the pattern' % name)


def placed(use, outputs, values):
    linked = {}
    for folder, held in use.get('folders', {}).items():
        located(folder, values).mkdir(parents=True, exist_ok=True)
        for item in held:
            for source in sources_of(item, outputs, values):
                if put(source, located(folder, values) / source.name, values):
                    linked[Path(source)] = stamp(source)
    for path, text in use.get('files', {}).items():
        located(path, values).parent.mkdir(parents=True, exist_ok=True)
        located(path, values).unlink(missing_ok=True)
        located(path, values).write_text(expand(text, values), encoding='utf-8', newline='\n')
    return linked


def scripted(running, script, outputs, values):
    for step in script:
        if 'wait' in step and not waited(running, expand(step['wait'], values)):
            return
        for path, item in step.get('put', {}).items():
            located(path, values).unlink(missing_ok=True)
            shutil.copy(source_of(item, outputs, values), located(path, values))
        for path in step.get('remove', []):
            located(path, values).unlink()
        if 'type' in step:
            typed(running, expand(step['type'], values) + '\n')
        if 'close' in step:
            running['proc'].stdout.close()
        if 'signal' in step:
            signals.sent(running['proc'], step['signal'])
        if 'pause' in step:
            time.sleep(step['pause'])


def made_false(case):
    false = case.get('false', {})
    merged = dict(case, **false)
    for key in ('files', 'folders'):
        merged[key] = dict(case.get(key, {}), **false.get(key, {}))
    return merged


def typed_rows(use, broken, order, values):
    return ''.join(expand(use['rows'][at]['false' if broken else 'in'], values) + '\n' for at in order)


def tried(config, values, outputs, where, case, broken, counted=False, failing=None, part=False, order=None):
    use = made_false(case) if broken else case
    values, outputs = failing if failing and (use.get('fail') or use.get('sweep')) else (values, outputs)
    folder = Path(values['out']) / 'cases' / where
    cleared(folder)
    folder.mkdir(parents=True, exist_ok=True)
    mine = dict(values, case=folder.as_posix())
    if '{port}' in str(use):
        mine['port'] = free_port()
    counting = {'C23_LEAKS': folder.as_posix() + '.leaks'} if counted else {}
    plan = ''.join(' %s=%s' % pair for pair in use.get('fail', {}).items()) or False
    linked = placed(use, outputs, mine)
    root, timeout, helper = values['root'], config['test']['timeout'], use.get('helper')
    beside = started(expanded(helper['run'], mine), root, timeout, environment(counting, mine)) if helper else None
    if helper:
        waited(beside, expand(helper['wait'], mine))
    running = started(expanded(use['run'], mine), root, timeout,
                      environment({**counting, 'C23_FAIL': plan, 'C23_FAIL_PART': bool(plan or use.get('sweep')) and part,
                                   **use.get('env', {})},
                                  mine), use)
    scripted(running, use.get('script', []), outputs, mine)
    typing = expand(use.get('stdin', ''), mine) if order is None else typed_rows(use, broken, order, mine)
    ends = [ended(running, typing, order is not None)]
    if helper:
        ends.append(ended(beside, expand(helper.get('stdin', ''), mine)))
    mine[WROTE] = sorted(path.as_posix() for path, was in linked.items() if stamp(path) != was)
    return ends, [use.get('claims', {})] + ([helper.get('claims', {})] if helper else []), mine


def judged(config, group, broken, run):
    ends, claims, mine = run
    if mine[WROTE]:
        return False, 'wrote into the build output %s' % ', '.join(mine[WROTE]), ends
    lacks = [rule['lacks'] for rule in config['test'].get('rule', []) if group not in rule.get('exempt', [])]
    if any(code is None for code, _ in ends):
        return False, 'did not end in %d seconds' % config['test']['timeout'], ends
    held = all(holds(claim, lacks, code, said, mine) for claim, (code, said) in zip(claims, ends))
    if broken:
        return not held, 'still as written' if held else 'fails, as it must', ends
    return held, 'as written' if held else 'differs', ends


def answered(config, group, broken, run, rows, order):
    ends, claims, mine = run
    code, said = ends[0]
    if mine[WROTE]:
        return [(False, 'wrote into the build output %s' % ', '.join(mine[WROTE]))] * len(rows)
    lacks = [rule['lacks'] for rule in config['test'].get('rule', []) if group not in rule.get('exempt', [])]
    blocks = said.split('\n\n')[:-1]
    if any(end is None for end, _ in ends):
        return [(False, 'did not end in %d seconds' % config['test']['timeout'])] * len(rows)
    if len(blocks) > len(rows):
        return [(False, 'is one of %d rows, and the command gave %d answers' % (len(rows), len(blocks)))] * len(rows)
    if not all(holds(claim, lacks, end, text, mine) for claim, (end, text) in zip(claims, ends)):
        return [(False, 'is in a run that differs')] * len(rows)
    found = [(False, 'has no answer')] * len(rows)
    for place, at in enumerate(order[:len(blocks) + 1]):
        if place == len(blocks):
            found[at] = (False, 'has no answer: the command ended at this row')
            continue
        held = claimed(rows[at]['claims'], lacks, code, blocks[place], mine)
        found[at] = (not held, 'still as written' if held else 'fails, as it must') if broken \
            else (held, 'as written' if held else 'differs')
    return found


def joined(founds, count):
    rows = []
    for verdicts in zip(*founds):
        bad = [one for one in verdicts if not one[0]]
        several = bad and count > 1
        rows.append((False, '%s in %d of %d copies' % (bad[0][1], len(bad), count)) if several else (bad or verdicts)[0])
    return rows


def raced(config, values, outputs, group, where, case, broken, count, counted, failing, order):
    wheres = [where] if count == 1 else ['%s/%d' % (where, copy) for copy in range(count)]
    with concurrent.futures.ThreadPoolExecutor(count) as pool:
        runs = list(pool.map(lambda place: tried(config, values, outputs, place, case, broken, counted, failing,
                                                 owner(config, group), order), wheres))
    founds = [answered(config, group, broken, run, case['rows'], order) for run in runs]
    failed = [run for run, found in zip(runs, founds) if not all(ok for ok, _ in found)]
    return joined(founds, count), (failed or runs)[0][0]


def tabled(config, values, outputs, group, index, case, broken, copies, counted=False, failing=None):
    where, forward = '%s/%d' % (group, index), list(range(len(case['rows'])))
    count = 1 if case.get('alone') else copies
    found, ends = raced(config, values, outputs, group, where, case, broken, count, counted, failing, forward)
    if broken or counted or len(forward) < 2 or not all(ok for ok, _ in found):
        return found, ends
    back, again = raced(config, values, outputs, group, where + '/reverse', case, broken, 1, counted, failing,
                        forward[::-1])
    if all(ok for ok, _ in back):
        return found, ends
    return [(ok, how if ok else how + ' in the reverse order') for ok, how in back], again


def verdict(config, values, outputs, group, index, case, broken, copies, counted=False, failing=None):
    count = 1 if case.get('alone') else copies
    where = '%s/%d' % (group, index)
    wheres = [where] if count == 1 else ['%s/%d' % (where, copy) for copy in range(count)]
    with concurrent.futures.ThreadPoolExecutor(count) as pool:
        runs = list(pool.map(lambda where: tried(config, values, outputs, where, case, broken, counted, failing,
                                                 owner(config, group)), wheres))
    verdicts = [judged(config, group, broken, run) for run in runs]
    bad = [one for one in verdicts if not one[0]]
    if not bad:
        return verdicts[0]
    ok, how, ends = bad[0]
    return ok, how + (' in %d of %d copies' % (len(bad), count) if count > 1 else ''), ends


WROTE = 'the outputs that a case wrote'
WAIT = 0.2
USE = re.compile(r'[A-Za-z0-9_-]+')


def uses_of(config, values, running):
    table = config.get('test', {}).get('uses', {})
    if not table:
        return {}
    if not isinstance(table, dict):
        die('[test] uses: write a table of the uses')
    root = Path(values['root'])
    paths = files(root, [top + '**/*' for top in scope.CODE + scope.BUILT]) + files(root, list(scope.FILES))
    found, named = scope.groups(config, root), {}
    run = [group for group in found if group['name'] in running]
    for name, patterns in sorted(table.items()):
        if not USE.fullmatch(name):
            die('[test] uses: %s is not a name of letters, digits, _ and -' % name)
        if not (isinstance(patterns, list) and all(isinstance(pattern, str) for pattern in patterns)):
            die('[test] uses: %s is a list of patterns' % name)
        if not patterns:
            die('[test] uses: %s has no pattern' % name)
        for pattern in patterns:
            if not any(fnmatch.fnmatch(path, pattern) for path in paths):
                die('[test] uses: %s matches no file' % pattern)
        mine = [path for path in paths if any(fnmatch.fnmatch(path, pattern) for pattern in patterns)]
        for group in run:
            whole = group['whole'] and group['code'] is None
            if whole or any(scope.tests(config, root, found, group, path) for path in mine):
                named.setdefault(group['name'], []).append(name)
    return named


def test(config, values, outputs, groups, broken, said, copies, counted=False, failing=None, workers=1):
    wrong, texts, uses = [], [], uses_of(config, values, {group for group, _ in groups})
    jobs = [[(group, index, case, concurrent.futures.Future()) for index, case in enumerate(cases)]
            for group, cases in groups]
    lanes, stop, room = lanes_of(jobs, uses), threading.Event(), Room(floor_of(config))

    def ran(lane):
        for group, index, case, future in lane:
            if stop.is_set():
                return
            try:
                with room, locks.held(uses.get(group, []), group):
                    future.set_result(one(config, values, outputs, group, index, case, broken, copies, counted,
                                          failing))
            except BaseException as error:
                future.set_exception(error)

    with concurrent.futures.ThreadPoolExecutor(workers) as pool:
        for lane in lanes:
            pool.submit(ran, lane)
        try:
            for (group, _), mine in zip(groups, jobs):
                right, count = 0, 0
                for *_, future in mine:
                    named, ends = future.result()
                    texts += [said for _, said in ends]
                    told(named, ends, broken)
                    right += sum(ok for _, ok, _ in named)
                    count += len(named)
                    wrong += [name for name, ok, _ in named if not ok]
                print('%s: %d claims made false: %d cases failed' % (group, count, right) if broken else
                      '%s: %d cases: %d as written' % (group, count, right))
        except BaseException:
            stop.set()
            raise
    if said:
        (Path(values['out']) / 'said.txt').write_text('\n'.join(texts), encoding='utf-8', newline='\n')
    return wrong


def free_memory():
    if os.name == 'nt':
        import ctypes

        class Status(ctypes.Structure):
            _fields_ = [('length', ctypes.c_ulong), ('load', ctypes.c_ulong)] + [
                (name, ctypes.c_ulonglong) for name in ('phys', 'free_phys', 'commit', 'free_commit', 'virtual',
                                                        'free_virtual', 'free_extended')]
        status = Status(length=ctypes.sizeof(Status))
        return status.free_commit if ctypes.windll.kernel32.GlobalMemoryStatusEx(ctypes.byref(status)) else None
    if platform.system() == 'Darwin':
        try:
            said = subprocess.run(['vm_stat'], capture_output=True, text=True, check=True).stdout
            size = int(re.search(r'page size of (\d+)', said).group(1))
        except (OSError, subprocess.CalledProcessError, AttributeError):
            return None
        return size * sum(int(n) for n in re.findall(r'Pages (?:free|inactive|speculative|purgeable):\s+(\d+)', said))
    try:
        with open('/proc/meminfo', encoding='ascii') as info:
            return next(int(line.split()[1]) * 1024 for line in info if line.startswith('MemAvailable:'))
    except (OSError, StopIteration, ValueError):
        return None


def floor_of(config):
    floor = config['test'].get('memory_floor', 2 << 30)
    if not (isinstance(floor, int) and not isinstance(floor, bool) and floor >= 0):
        die('[test] memory_floor is a count of bytes, 0 or more')
    return floor


class Room:
    def __init__(self, floor):
        self.floor, self.busy, self.gate = floor, 0, threading.Condition()

    def __enter__(self):
        with self.gate:
            while self.floor and self.busy and (free_memory() or self.floor) < self.floor:
                self.gate.wait(WAIT)
            self.busy += 1

    def __exit__(self, *_):
        with self.gate:
            self.busy -= 1
            self.gate.notify_all()


def lanes_of(jobs, uses):
    shared, free = {}, []
    for job in (job for mine in jobs for job in mine):
        key = tuple(sorted(uses.get(job[0], [])))
        if key:
            shared.setdefault(key, []).append(job)
        else:
            free.append([job])
    return list(shared.values()) + free


def one(config, values, outputs, group, index, case, broken, copies, counted, failing):
    if 'rows' in case:
        found, ends = tabled(config, values, outputs, group, index, case, broken, copies, counted, failing)
        return [(row['name'], ok, how) for row, (ok, how) in zip(case['rows'], found)], ends
    ok, how, ends = verdict(config, values, outputs, group, index, case, broken, copies, counted, failing)
    return [(case['name'], ok, how)], ends


def told(named, ends, broken):
    for name, ok, how in named:
        print('%s%s: %s' % (name, ', made false' if broken else '', how))
    if not all(ok for _, ok, _ in named):
        for whose, (code, said) in zip(('', 'the helper '), ends):
            print('  %sended with %s, and printed:\n%s' % (whose, 'no status' if code is None else status(code),
                                                          said.rstrip()))


def rowed(case, path):
    for key in ('false', 'stdin', 'script'):
        if key in case:
            die('%s: the table %s has rows, and a table has no key %s' % (path, case['name'], key))
    if not case['rows']:
        die('%s: the table %s has no row' % (path, case['name']))
    for row in case['rows']:
        known(row, 'row', path)
        missing = [key for key in ('name', 'in', 'claims', 'false') if key not in row]
        if missing:
            die('%s: a row of the table %s has no key %s' % (path, case['name'], missing[0]))
        if not known(row['claims'], 'claim of a row', path):
            die('%s: the row %s claims nothing' % (path, row['name']))


PROCESS = ('status', 'signal', 'helper', 'command', 'state', 'script')


def ruled(path, cases):
    by_hand = {}
    for case in cases:
        if 'rows' in case and 'process' in case:
            die('%s: the table %s states process, and only a case with no rows has its own process'
                % (path, case['name']))
        if 'rows' not in case and 'process' not in case:
            die('%s: the case %s has its own process and states no reason: write process, or make it a row of a table'
                % (path, case['name']))
        if len(case.get('fail', {})) == 1:
            by_hand.setdefault((tuple(case['run']), *case['fail']), []).append(case['name'])
    for (_, call), names in by_hand.items():
        if len(names) > 2:
            die('%s: %d cases refuse the calls of %s one after the other, from the case %s: write a sweep'
                % (path, len(names), call, names[0]))


def groups_of(config, values, folder):
    root, found, calls = Path(values['root']), [], fail.listed(config, values)
    for path in files(root, [folder + '/**/*.toml']):
        cases = tomllib.loads((root / path).read_text(encoding='utf-8')).get('case', [])
        for case in cases:
            known(case, 'case', path)
            known(case.get('false', {}), 'false', path)
            known(case.get('helper', {}), 'helper', path)
            signals.allowed(case, path)
            signals.allowed(made_false(case), path)
            if not case.get('claims'):
                die('%s: the case %s claims nothing' % (path, case['name']))
            if 'rows' in case:
                rowed(case, path)
            elif 'false' not in case:
                die('%s: the case %s has no false variant' % (path, case['name']))
            if case.get('process', PROCESS[0]) not in PROCESS:
                die('%s: the case %s states the process %s, and a process is %s'
                    % (path, case['name'], case['process'], ', '.join(PROCESS)))
            if case.get('process') == 'script' and 'script' not in case and 'stdin' not in case:
                die('%s: the case %s states the process script, and it has no script and no stdin' % (path, case['name']))
            for call in [*case.get('fail', {}), *case.get('false', {}).get('fail', {}), *case.get('sweep', [])]:
                if call not in calls:
                    die('%s: the case %s makes %s fail, and [fail.calls] does not list it' % (path, case['name'], call))
        if config.get('test', {}).get('tables'):
            ruled(path, cases)
        found.append((Path(path).relative_to(folder).with_suffix('').as_posix(), cases))
    return [(group, cases) for group, cases in found if cases]


def narrowed(case, text):
    if 'rows' not in case or text in case['name']:
        return case if text in case['name'] else None
    rows = [row for row in case['rows'] if text in row['name']]
    return dict(case, rows=rows) if rows else None


def chosen(groups, text):
    found = [(group, [one for one in (narrowed(case, text) for case in cases) if one]) for group, cases in groups]
    return [(group, cases) for group, cases in found if cases] or die('no case has %s in its name' % text)


def owner(config, group):
    found = shared.part_of(config, group)
    return ' %s ' % found if found else False


def picked(config, groups, args):
    if args.part:
        folders = [config['parts'][part]['folder'] for part in args.part]
        groups = [(group, cases) for group, cases in groups if any(shared.held(group, folder) for folder in folders)] \
            or die('no case group is in the parts %s' % ' '.join(args.part))
    if args.group:
        missing = [name for name in args.group if name not in dict(groups)]
        if missing:
            die('no case group %s' % missing[0])
        groups = [(group, cases) for group, cases in groups if group in args.group]
    return chosen(groups, args.k) if args.k else groups


def changed(args, run, write, prepared=None):
    config, values = prepared or build.ready(args, True)
    values = dict(values, out=(Path(values['build']) / run / values['target']).as_posix())
    written = write(build.kept(config, set(args.part)) if args.part else config, values)
    made = build.made(config, values, rewrite.tree(values, run))
    unlinked = [name for name, paths in made if not paths]
    if unlinked:
        die('the cases need what is linked: ' + ' '.join(unlinked))
    groups = groups_of(config, values, config.get('test', {}).get('cases', 'tests/cases'))
    groups = picked(config, groups, args) if groups else groups
    return config, values, written, {name: paths[0] for name, paths in made}, groups


def scoped(args, entries, key, names):
    if not (args.part or args.group):
        return entries
    return [entry for entry in entries if any(fnmatch.fnmatch(name, entry.get(key, '')) for name in names)]


def measured(args):
    if args.broken or args.toolchain or args.copies or args.rounds:
        die('--branches runs each case one time: it takes no --break, --toolchain, --copies or --rounds')
    named = args.part or []
    config, values, (probes, gaps), outputs, groups = changed(
        args, 'branches', lambda config, values: branches.probed(
            config, values, named, args.group and scope.claimed(config, Path(values['root']), args.group)))
    hits = Path(values['out']) / 'hits.bin'
    hits.write_bytes(bytes(branches.parts(probes)[1]))
    os.environ['C23_BRANCHES'] = str(hits)
    test(config, values, outputs, groups, False, False, 1, workers=workers_of(args, config))
    excepted = scoped(args, config.get('branches', {}).get('exception', []), 'path', probes)
    if args.group:
        print('branches: measured the code of the groups %s' % ', '.join(args.group))
    branches.reported(probes, gaps, hits, excepted, values['system'], named)


def workers_of(args, config):
    count = args.workers if args.workers is not None else config.get('test', {}).get('workers', os.cpu_count() or 1)
    if not (isinstance(count, int) and not isinstance(count, bool) and count > 0):
        die('[test] workers and --workers are a count of 1 or more')
    return count


def tallied(args):
    if args.broken or args.toolchain or args.copies or args.rounds or args.branches:
        die('--leaks runs each case one time: it takes no --break, --toolchain, --copies, --rounds or --branches')
    config, values, (files, kinds), outputs, groups = changed(args, 'leaks', leaks.counted)
    folder = Path(values['out']) / 'cases'
    cleared(folder)
    test(config, values, outputs, groups, False, False, 1, True, workers=workers_of(args, config))
    cases = [(case['name'], folder / group / ('%d.leaks' % index)) for group, listed in groups
             for index, case in enumerate(listed)]
    excepted = scoped(args, config.get('leaks', {}).get('exception', []), 'case', [name for name, _ in cases])
    leaks.reported(files, kinds, cases, excepted, values['system'])


def main():
    args = shared.arguments('-k', ('--copies', int), ('--rounds', int), ('--workers', int), broken='--break', toolchain='--toolchain',
                            branches='--branches', leaks='--leaks', built='--build')
    if args.built and (args.toolchain or args.branches or args.leaks or args.broken):
        die('--build builds each artifact and runs no case: it takes no --toolchain, --branches, --leaks or --break')
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
    if args.built:
        return print('built %d artifacts' % len(made))
    folder = '.toolchain/tests/cases' if args.toolchain else config.get('test', {}).get('cases', 'tests/cases')
    groups = groups_of(config, values, folder)
    if not groups:
        return print('no case: %s holds no case' % folder)
    groups = picked(config, groups, args)
    copies = args.copies or config['test'].get('copies') or die('.toolchain/project.toml: write [test] copies')
    own = not (args.broken or args.toolchain or args.k or args.part or args.group)
    rounds = args.rounds or 1
    failing = None
    if any(use.get('fail') or use.get('sweep') for use in ((made_false(case) if args.broken else case)
                                                            for _, cases in groups for case in cases)):
        _, mine, _, outputs, _ = changed(args, 'fail', fail.failed, (config, values))
        failing = mine, outputs
    for done in range(rounds):
        if rounds > 1:
            print('round %d of %d' % (done + 1, rounds))
        wrong = test(config, values, {name: paths[0] for name, paths in made}, groups, args.broken, own, copies,
                     failing=failing, workers=workers_of(args, config))
        if wrong:
            die(('still as written with their claims made false: ' if args.broken else 'not as written: ')
                + ', '.join(wrong))
    if own:
        checked(Path(values['root']), Path(values['out']))


if __name__ == '__main__':
    main()
