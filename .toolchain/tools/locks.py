# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import contextlib
import errno
import os
import sys
import tempfile
import threading
import time
from pathlib import Path

if os.name == 'nt':
    import msvcrt
else:
    import fcntl

WAIT = 0.2
BUSY = (errno.EACCES, errno.EDEADLOCK) if os.name == 'nt' else (errno.EACCES, errno.EAGAIN, errno.EWOULDBLOCK)


def folder():
    return Path(os.environ.get('MC23T_LOCKS') or Path(tempfile.gettempdir()) / 'mc23t-locks')


def took(handle):
    try:
        if os.name == 'nt':
            os.lseek(handle, 0, os.SEEK_SET)
            msvcrt.locking(handle, msvcrt.LK_NBLCK, 1)
        else:
            fcntl.flock(handle, fcntl.LOCK_EX | fcntl.LOCK_NB)
    except OSError as error:
        if error.errno in BUSY:
            return False
        raise
    return True


def gave(handle):
    if os.name == 'nt':
        os.lseek(handle, 0, os.SEEK_SET)
        msvcrt.locking(handle, msvcrt.LK_UNLCK, 1)
    os.close(handle)


def opened(name):
    place = folder()
    place.mkdir(parents=True, exist_ok=True)
    with contextlib.suppress(PermissionError):
        place.chmod(0o1777)
    path = place / (name + '.lock')
    handle = os.open(path, os.O_RDWR | os.O_CREAT, 0o666)
    with contextlib.suppress(PermissionError):
        path.chmod(0o666)
    return handle


MINE, GUARD = {}, threading.Lock()


def mine(name):
    with GUARD:
        return MINE.setdefault(name, threading.Lock())


@contextlib.contextmanager
def held(names, group):
    taken = []
    try:
        for name in sorted(names):
            inner = mine(name)
            inner.acquire()
            taken.append((inner, None))
            handle = opened(name)
            try:
                if not took(handle):
                    print('%s: waits for the use %s, which another run holds' % (group, name), flush=True)
                    while not took(handle):
                        time.sleep(WAIT)
            except BaseException:
                os.close(handle)
                raise
            taken[-1] = (inner, handle)
        yield
    finally:
        for inner, handle in reversed(taken):
            if handle is not None:
                gave(handle)
            inner.release()


def main():
    if len(sys.argv) < 4:
        sys.exit('usage: python locks.py <folder> <seconds> <use> ...')
    os.environ['MC23T_LOCKS'] = sys.argv[1]
    names = sys.argv[3:]
    with held(names, 'locks'):
        print('held ' + ' '.join(sorted(names)), flush=True)
        done = threading.Event()
        threading.Thread(target=lambda: (sys.stdin.readline(), done.set()), daemon=True).start()
        done.wait(float(sys.argv[2]))


if __name__ == '__main__':
    main()
