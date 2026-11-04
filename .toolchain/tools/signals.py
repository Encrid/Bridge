# Copyright (C) 2026 Enkrid
#
# Under the GNU Affero General Public License, version 3 only: see license.
import ctypes
import os
import signal
import subprocess
import sys
import threading

from shared import die, known

POSIX = {'interrupt': 'SIGINT', 'termination': 'SIGTERM', 'hang-up': 'SIGHUP'}
IGNORED = 'trap "" INT; exec "$0" "$@"'
BREAK = ("import ctypes, sys; k = ctypes.WinDLL('kernel32', use_last_error=True); pid = int(sys.argv[1]); "
         "sys.exit(0 if k.AttachConsole(pid) and k.GenerateConsoleCtrlEvent(1, pid) else ctypes.get_last_error() or 1)")


def allowed(use, path):
    name, sent, closed, hung = use.get('name'), set(), False, False
    if use.get('ignore', []) not in ([], ['interrupt']):
        die('%s: the case %s ignores %s, and only interrupt can be ignored' % (path, name, use['ignore']))
    for step in use.get('script', []):
        known(step, 'script', path)
        if 'wait' in step and closed:
            die('%s: the case %s waits for a line after it closes the output' % (path, name))
        if step.get('close', 'output') != 'output':
            die('%s: the case %s closes %s, and only the output can close' % (path, name, step['close']))
        if 'signal' in step and step['signal'] not in POSIX:
            die('%s: the case %s sends %s, and the signals are interrupt, termination and hang-up'
                % (path, name, step['signal']))
        if 'signal' in step and hung:
            die('%s: the case %s sends a signal after a hang-up, and on Windows a hang-up ends the console'
                % (path, name))
        pause = step.get('pause', 1)
        if isinstance(pause, bool) or not isinstance(pause, (int, float)) or pause <= 0:
            die('%s: the case %s pauses %s, and a pause is a number of seconds above 0' % (path, name, pause))
        closed = closed or 'close' in step
        hung = hung or step.get('signal') == 'hang-up'
        sent.add(step.get('signal'))
    if {'interrupt', 'termination'} <= sent:
        die('%s: the case %s sends an interrupt and a termination, and on Windows a termination turns off '
            'the interrupt' % (path, name))


def started(argv, root, env, use):
    ignored = bool(use.get('ignore'))
    names = {step['signal'] for step in use.get('script', []) if 'signal' in step}
    if os.name == 'nt' and (names or ignored):
        return Console(argv, root, env, ignored or 'termination' in names)
    if ignored:
        argv = ['sh', '-c', IGNORED] + list(argv)
    return subprocess.Popen(argv, cwd=root, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, env=env)


def sent(proc, name):
    if os.name == 'nt':
        proc.sent(name)
    else:
        proc.send_signal(getattr(signal, POSIX[name]))


def drained(screen):
    while screen.read(1 << 16):
        pass


class Console:
    def __init__(self, argv, root, env, grouped):
        import msvcrt
        from ctypes import wintypes as w

        class Coord(ctypes.Structure):
            _fields_ = [('X', w.SHORT), ('Y', w.SHORT)]

        class Start(ctypes.Structure):
            _fields_ = [('cb', w.DWORD), ('reserved', w.LPWSTR), ('desktop', w.LPWSTR), ('title', w.LPWSTR)]
            _fields_ += [(name, w.DWORD) for name in ('x', 'y', 'width', 'height', 'columns', 'rows', 'fill', 'flags')]
            _fields_ += [('show', w.WORD), ('reserved2', w.WORD), ('reserved3', ctypes.c_void_p),
                         ('input', w.HANDLE), ('output', w.HANDLE), ('errors', w.HANDLE), ('attributes', ctypes.c_void_p)]

        class Process(ctypes.Structure):
            _fields_ = [('process', w.HANDLE), ('thread', w.HANDLE), ('pid', w.DWORD), ('tid', w.DWORD)]

        self.k = k = ctypes.WinDLL('kernel32', use_last_error=True)
        k.CreatePipe.argtypes = [ctypes.POINTER(w.HANDLE), ctypes.POINTER(w.HANDLE), ctypes.c_void_p, w.DWORD]
        k.CreatePseudoConsole.argtypes = [Coord, w.HANDLE, w.HANDLE, w.DWORD, ctypes.POINTER(w.HANDLE)]
        k.ClosePseudoConsole.argtypes = [w.HANDLE]
        k.ClosePseudoConsole.restype = None
        k.SetHandleInformation.argtypes = [w.HANDLE, w.DWORD, w.DWORD]
        k.InitializeProcThreadAttributeList.argtypes = [ctypes.c_void_p, w.DWORD, w.DWORD,
                                                        ctypes.POINTER(ctypes.c_size_t)]
        k.UpdateProcThreadAttribute.argtypes = [ctypes.c_void_p, w.DWORD, ctypes.c_size_t, ctypes.c_void_p,
                                                ctypes.c_size_t, ctypes.c_void_p, ctypes.c_void_p]
        k.CreateProcessW.argtypes = [w.LPCWSTR, w.LPWSTR, ctypes.c_void_p, ctypes.c_void_p, w.BOOL, w.DWORD,
                                     ctypes.c_void_p, w.LPCWSTR, ctypes.c_void_p, ctypes.c_void_p]
        k.CloseHandle.argtypes = [w.HANDLE]
        k.TerminateProcess.argtypes = [w.HANDLE, w.UINT]
        k.WaitForSingleObject.argtypes = [w.HANDLE, w.DWORD]
        k.WaitForSingleObject.restype = w.DWORD
        k.GetExitCodeProcess.argtypes = [w.HANDLE, ctypes.POINTER(w.DWORD)]
        k.SetConsoleCtrlHandler.argtypes = [ctypes.c_void_p, w.BOOL]
        k.SetConsoleCtrlHandler(None, False)
        self.closing = None
        tty_read, tty_write = self.pipe(w)
        screen_read, screen_write = self.pipe(w)
        self.console = w.HANDLE()
        self.check(k.CreatePseudoConsole(Coord(80, 25), tty_read, screen_write, 0, ctypes.byref(self.console)) == 0,
                   'CreatePseudoConsole')
        in_read, in_write = self.pipe(w)
        out_read, out_write = self.pipe(w)
        for handle in (in_read, out_write):
            self.check(k.SetHandleInformation(handle, 1, 1), 'SetHandleInformation')
        size = ctypes.c_size_t()
        k.InitializeProcThreadAttributeList(None, 2, 0, ctypes.byref(size))
        attributes = ctypes.create_string_buffer(size.value)
        self.check(k.InitializeProcThreadAttributeList(attributes, 2, 0, ctypes.byref(size)),
                   'InitializeProcThreadAttributeList')
        self.check(k.UpdateProcThreadAttribute(attributes, 0, 0x00020016, self.console, ctypes.sizeof(w.HANDLE),
                                               None, None), 'UpdateProcThreadAttribute')
        inherited = (w.HANDLE * 2)(in_read, out_write)
        self.check(k.UpdateProcThreadAttribute(attributes, 0, 0x00020002, ctypes.cast(inherited, ctypes.c_void_p),
                                               ctypes.sizeof(inherited), None, None), 'UpdateProcThreadAttribute')
        start = Start(cb=ctypes.sizeof(Start), flags=0x100, input=in_read, output=out_write, errors=out_write,
                      attributes=ctypes.cast(attributes, ctypes.c_void_p))
        text = None if env is None else ''.join('%s=%s\0' % pair for pair in sorted(env.items(),
                                                key=lambda pair: pair[0].upper())) + '\0'
        block = None if text is None else (ctypes.c_wchar * len(text))(*text)
        info = Process()
        line = ctypes.create_unicode_buffer(subprocess.list2cmdline(argv))
        self.check(k.CreateProcessW(None, line, None, None, True, 0x80000 | 0x400 | (0x200 if grouped else 0), block,
                                    str(root), ctypes.byref(start), ctypes.byref(info)), 'CreateProcessW')
        for handle in (tty_read, screen_write, in_read, out_write, info.thread):
            k.CloseHandle(handle)
        self.pid, self.process = info.pid, info.process
        threading.Thread(target=drained, args=(self.opened(msvcrt, screen_read, 'rb'),), daemon=True).start()
        self.tty = self.opened(msvcrt, tty_write, 'wb', 0)
        self.stdin = self.opened(msvcrt, in_write, 'wb')
        self.stdout = self.opened(msvcrt, out_read, 'rb')

    def pipe(self, w):
        read, write = w.HANDLE(), w.HANDLE()
        self.check(self.k.CreatePipe(ctypes.byref(read), ctypes.byref(write), None, 0), 'CreatePipe')
        return read, write

    @staticmethod
    def opened(msvcrt, handle, mode, buffering=-1):
        return os.fdopen(msvcrt.open_osfhandle(handle.value, 0), mode, buffering)

    @staticmethod
    def check(done, call):
        if not done:
            die('%s failed: error %d' % (call, ctypes.get_last_error()))

    def sent(self, name):
        if name == 'interrupt':
            self.tty.write(b'\x03')
        elif name == 'termination':
            done = subprocess.run([sys.executable, '-c', BREAK, str(self.pid)], creationflags=0x8,
                                  stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            if done.returncode:
                die('GenerateConsoleCtrlEvent failed: error %d' % done.returncode)
        else:
            self.close()

    def close(self):
        if not self.closing:
            self.closing = threading.Thread(target=self.k.ClosePseudoConsole, args=(self.console,), daemon=True)
            self.closing.start()

    def kill(self):
        self.k.TerminateProcess(self.process, 1)

    def wait(self):
        code = ctypes.c_ulong()
        self.k.WaitForSingleObject(self.process, 0xFFFFFFFF)
        self.k.GetExitCodeProcess(self.process, ctypes.byref(code))
        self.close()
        return code.value
