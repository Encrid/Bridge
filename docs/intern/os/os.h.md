# src/os/os.h

This header gives the operating system as Bridge and its backends need it.
That is address space, shared libraries, threads, the clock, a mutex and an event.

## The layer

This layer is the only code that differs by platform, and it depends on nothing of ours.
It includes `<stddef.h>` and `<stdint.h>` and no other header.
Bridge reaches memory mapping, shared libraries, threads and the clock through this layer and nowhere else.
The library compiles it, and so does each backend, which is a binary of its own.
A backend that is a project of its own keeps a copy of the layer.

`src/os/os.h` declares each function once.
`src/os/posix` holds the functions for Linux and macOS, and `src/os/windows` holds the same functions for Windows.
The build compiles `src/os/{system}/*.c`, where `system` is the key of the target in `.toolchain/toolchain.toml`.
So no file of the layer asks which platform it is compiled for.

## The imports

The library imports only these functions from the system.
The first word is where the import is: `every` system, `posix` (Linux and macOS), one system, or one system and one processor, for example `macos_x86_64`.
`tools/deps.py --check` compares this list with the imports of the built library.

```text
every            malloc
every            calloc
every            realloc
every            free
every            strcmp
every            strncmp
every            strchr
every            strrchr
every            strpbrk
every            strtoll
every            strtoull
posix            strlen
posix            memcpy
posix            memmove
posix            snprintf
posix            vsnprintf
linux            memset
linux            bcmp
macos            memcmp
macos_aarch64    bzero
macos_x86_64     __bzero
macos            __memcpy_chk
linux            __errno_location
macos            __error
windows          _errno
windows          __stdio_common_vsprintf
windows          VirtualAlloc
windows          VirtualFree
windows          LoadLibraryA
windows          LoadLibraryExA
windows          GetFullPathNameA
windows          GetProcAddress
windows          FreeLibrary
windows          CreateThread
windows          WaitForSingleObject
windows          CloseHandle
windows          GetCurrentThreadId
windows          QueryPerformanceCounter
windows          QueryPerformanceFrequency
windows          InitializeCriticalSection
windows          EnterCriticalSection
windows          LeaveCriticalSection
windows          DeleteCriticalSection
windows          CreateEventA
windows          SetEvent
posix            mmap
posix            mprotect
posix            munmap
posix            dlopen
posix            dlsym
posix            dlclose
posix            pthread_create
posix            pthread_join
posix            pthread_self
posix            clock_gettime
posix            pthread_mutex_init
posix            pthread_mutex_lock
posix            pthread_mutex_unlock
posix            pthread_mutex_destroy
posix            pthread_cond_init
posix            pthread_cond_signal
posix            pthread_cond_wait
posix            pthread_cond_timedwait
posix            pthread_cond_destroy
windows          _initterm
windows          _initterm_e
windows          _initialize_onexit_table
windows          _register_onexit_function
windows          _execute_onexit_table
windows          _exit
windows          abort
windows          __acrt_iob_func
windows          __stdio_common_vfprintf
windows          fwrite
windows          VirtualProtect
windows          VirtualQuery
windows          GetLastError
windows          TlsGetValue
windows          Sleep
windows_aarch64  _fpreset
macos            dyld_stub_binder
```

## Why the library imports them

The list holds the imports of the optimized build, which is the build that ships.
A function of the runtime of the compiler is linked into the library, and is not an import.
`tools/deps.py --check` refuses an import that is not in the list, and a listed import that nothing imports.
It does not check a debug build: zig links its checks and its stack traces into that build, and they import more functions.

The C library is the one of the system, which the executable of the host chose.
On Windows, the build links the C runtime that zig carries.
Nothing of ours calls the imports of the startup code.
zig links its startup code into each shared library that it makes, and on Windows that code imports them.
They are in the list because they are imports all the same: a new one is a dependency that changed.

| Imports | Why |
| --- | --- |
| `malloc`, `calloc`, `realloc`, `free` | The library allocates records, their tables and copies of names outside a phase. It allocates nothing in a phase. |
| `strcmp`, `strncmp`, `strchr`, `strrchr`, `strpbrk` | The library compares names of modules, phases, components and settings, and a name up to its namespace. `strchr` finds where a namespace or a version ends, and a folder in the path of a library. `strrchr` finds where the name of a system begins in a full name. `strpbrk` finds an `@` or a `:` in the name of a system. |
| `strtoll`, `strtoull` | The schema reads the constants of a descriptor with them. |
| `__errno_location`, `__error`, `_errno` | They give `errno` on Linux, macOS and Windows. `strtoll` and `strtoull` set it. |
| `strlen`, `memcpy`, `memmove` | The library measures names, copies values and names, and closes up a run of a table. The compiler makes them inline on Windows. |
| `snprintf`, `vsnprintf`, `__stdio_common_vsprintf` | The library writes the text of a record. On Windows, the C runtime gives both functions as `__stdio_common_vsprintf`. |
| `memset` | The library makes a block zero on Linux. The compiler makes it inline on macOS and Windows. |
| `bcmp`, `memcmp` | The compiler makes them of a compare of bytes for equality: `bcmp` on Linux, and `memcmp` on macOS. |
| `bzero`, `__bzero` | The compiler makes them of a block made zero on macOS: `bzero` on aarch64, and `__bzero` on x86_64. |
| `__memcpy_chk` | The headers of macOS make it of `memcpy`. |
| `VirtualAlloc`, `VirtualFree`, `mmap`, `mprotect`, `munmap` | This layer reserves, commits and releases memory. |
| `LoadLibraryA`, `LoadLibraryExA`, `GetFullPathNameA`, `GetProcAddress`, `FreeLibrary`, `dlopen`, `dlsym`, `dlclose` | This layer opens the library of a backend or of a native module, finds a symbol in it, and closes it. |
| `CreateThread`, `WaitForSingleObject`, `CloseHandle`, `pthread_create`, `pthread_join` | This layer starts and joins a worker thread. On Windows, `WaitForSingleObject` also waits for an event, and `CloseHandle` also closes one. |
| `GetCurrentThreadId`, `pthread_self` | This layer gives the thread that runs, for a pinned module. |
| `QueryPerformanceCounter`, `QueryPerformanceFrequency`, `clock_gettime` | This layer reads the clock that keeps a deadline, and its unit. |
| `InitializeCriticalSection`, `EnterCriticalSection`, `LeaveCriticalSection`, `DeleteCriticalSection`, `pthread_mutex_init`, `pthread_mutex_lock`, `pthread_mutex_unlock`, `pthread_mutex_destroy` | This layer makes, locks, unlocks and frees a mutex. |
| `CreateEventA`, `SetEvent`, `pthread_cond_init`, `pthread_cond_signal`, `pthread_cond_wait`, `pthread_cond_timedwait`, `pthread_cond_destroy` | This layer makes, sets, waits for and frees an event. `pthread_cond_timedwait` waits until a deadline. |
| `_initterm`, `_initterm_e`, `_initialize_onexit_table`, `_register_onexit_function`, `_execute_onexit_table` | The startup code runs the initializers of the library, and keeps the table of what runs at exit. |
| `_exit`, `abort`, `__acrt_iob_func`, `__stdio_common_vfprintf`, `fwrite` | The startup code ends the process if the start fails, and writes what went wrong. |
| `VirtualProtect`, `VirtualQuery`, `GetLastError` | The startup code applies the relocations of the library. |
| `TlsGetValue`, `Sleep` | The startup code reads its thread-local storage, and waits for its start. |
| `_fpreset` | The startup code sets the floating-point state on Windows for arm64. |
| `dyld_stub_binder` | The loader of macOS binds an import with it at the first call. |

## Constants

Each constant is a `constexpr uint64_t`.

| Constant | Content |
| --- | --- |
| `OS_SECOND` | `1'000'000'000`: the nanoseconds in a second. |

## Types

### os_thread_t

A thread that `os_thread_start` makes: `struct os_thread`.
Each platform defines the struct in its `os_types.h`.

### os_mutex_t

A mutex: `struct os_mutex`, which each platform defines.

### os_event_t

An event: `struct os_event`, which each platform defines.

### os_thread_fn_t

The function that a thread runs: `void (*)(void *arg)`.

## Functions

Each function that can fail is `[[nodiscard]]`: the caller must use its result.

| Function | Result |
| --- | --- |
| `os_reserve` | Reserves `reserve` bytes of address space, and makes the first `commit` bytes readable, writable and zero. The rest faults when touched, which a backend that enforces bounds by a fault needs after a block. Gives `nullptr` on failure. |
| `os_release` | Gives back all of a reservation. Each caller passes a base that `os_reserve` gave. |
| `os_lib_open` | Opens a shared library, or gives `nullptr`. For a path, the loader looks there for the library and for what it needs, since a backend brings its runtime beside itself. A bare name goes to the search of the operating system. |
| `os_lib_get_symbol` | Gives where a library has what it exports as `name`, or `nullptr`. |
| `os_lib_close` | Closes a library. Each caller passes a library that `os_lib_open` gave. |
| `os_thread_start` | Starts a thread that runs `fn(arg)`. Gives `nullptr` on failure. |
| `os_thread_join` | Waits for a thread to end, and frees what `os_thread_start` gave. |
| `os_get_thread_id` | Gives a number for the calling thread that no other live thread has. It is never 0. |
| `os_get_now` | Gives nanoseconds from a clock that only rises. |
| `os_mutex_create` | Makes a mutex. Gives `nullptr` on failure. |
| `os_mutex_free` | Frees a mutex that no thread holds. A null `m` does nothing. |
| `os_mutex_lock` | Takes a mutex, and waits while another thread holds it. |
| `os_mutex_unlock` | Gives a mutex back. |
| `os_event_create` | Makes an event that one thread sets and another waits for. A set before the wait is not lost, and a wait consumes it. Gives `nullptr` on failure. |
| `os_event_free` | Frees an event that no thread waits for. A null `e` does nothing. |
| `os_event_set` | Sets an event: one wait, now or later, returns for it. |
| `os_event_wait` | Waits until the event is set, and consumes the set. A wait has no time limit: no caller needs one. |
