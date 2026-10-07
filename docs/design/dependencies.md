# Dependencies

**Every thing Bridge-ASM700 is built with or linked against is listed here, with why it is
there.** Third-party code is not refused. A dependency nobody wrote down is: `tools/deps.py` reads
this document and fails the build's tests on a tool `build.py` calls that is not listed, on a
function a built artifact imports that is not listed, and on one that is listed and that nothing
imports any more, so that the list is neither short nor long.

## What a build needs installed

A line is `tool`, the name `build.py` looks for, and why.

    tool      zig          the C compiler, the linker and the archiver, for every target
    tool      wasm-tools   assembles the test modules and the SDK's accessors, which are written
                           as wasm text, and prints a module for the case that reads one
    tool      wasm-merge   Binaryen: merges the SDK's accessors into a compiled wasm module
    tool      wasm-opt     Binaryen: puts the merged accessors inline - `wasm.md`, *Memories*

Python runs `build.py` and the scripts of `tools/`, with nothing outside its standard library.

## What is in the tree

**wasmtime**, the engine the wasm backend is linked against: its C headers, its import library
and its shared library, for each target, with where each came from and its hash beside it -
`wasmtime.md`, *The pin*. The backend imports the functions of it that it calls, and nothing else
imports any:

    every     wasm_*       the wasm C API, as wasmtime implements it
    every     wasmtime_*   wasmtime's own C API: linear memories it does not allocate, fuel,
                           epochs, linkers

## What the built artifacts import

The library, `bridge`, and the two backends, `bridge_wasm` and `bridge_native`. A line is the
system it is imported on, the function, and why. `every` is all three systems, and `posix` is
Linux and macOS. A function the compiler's own runtime provides is linked into the artifact and
is not an import.

**The list is of what ships: the optimised build.** A debug build is linked with zig's own
checks and its stack traces, which import some sixty functions more on Windows, and none of that
is in a library a host is handed. The case says so when it is run on one, and holds nothing.

### The C library

    every     malloc       what is allocated outside a phase: records, tables of them, copies of
    every     calloc       names. Nothing is allocated in a phase - `budgets.md`, *Nothing is
    every     realloc      allocated in a phase*
    every     free
    every     strcmp       names compared: of modules, phases, components, settings
    every     strncmp      a name compared up to its namespace
    every     strchr       where a namespace or a version ends in a name
    every     memchr       the wasm backend: where the first line of wasmtime's message ends
    every     strtoll      a constant of a descriptor, read - `schema.md`, *The type set*
    every     strtoull
    posix     strlen       how long a name is. Inline on Windows
    posix     memcpy       values and names copied. Inline on Windows
    posix     memmove      a run of a table closed up. Inline on Windows
    posix     snprintf     what a record says, written. On Windows these two are one function
    posix     vsnprintf    of the C runtime, below
    linux     memset       a block zeroed. Inline elsewhere
    linux     bcmp         what the compiler makes of a comparison of bytes for equality
    macos     memcmp       the same comparison, as it is on macOS
    macos     bzero        what the compiler makes of a block zeroed
    macos     __memcpy_chk what the system's headers make of memcpy
    linux     __errno_location      errno, which strtoll sets
    macos     __error               the same
    windows   _errno                the same
    windows   __stdio_common_vsprintf
                           snprintf and vsnprintf, as the C runtime has them
    windows   strpbrk      whether a library's path has a directory in it

### The operating system, through `utils/os`

One layer, compiled into the library and into each backend - `build.md`, *The C library*.

    windows   VirtualAlloc              memory reserved and committed
    windows   VirtualFree               and released
    windows   LoadLibraryA              a backend's or a native module's library opened
    windows   LoadLibraryExA            with its own folder searched first
    windows   GetFullPathNameA          for which its path is made whole
    windows   GetProcAddress            a symbol of it found
    windows   FreeLibrary               and the library closed
    windows   CreateThread              a worker thread started
    windows   WaitForSingleObject       a thread joined, and an event waited for
    windows   CloseHandle               a thread's or an event's handle closed
    windows   GetCurrentThreadId        which thread this is, for a pinned module
    windows   QueryPerformanceCounter   the clock a deadline is kept by
    windows   QueryPerformanceFrequency and its unit
    windows   InitializeCriticalSection a mutex made
    windows   EnterCriticalSection      locked
    windows   LeaveCriticalSection      unlocked
    windows   DeleteCriticalSection     and freed
    windows   CreateEventA              an event made
    windows   SetEvent                  and set
    posix     mmap                      memory reserved
    posix     mprotect                  committed
    posix     munmap                    and released
    posix     dlopen                    a backend's or a native module's library opened
    posix     dlsym                     a symbol of it found
    posix     dlclose                   and the library closed
    posix     pthread_create            a worker thread started
    posix     pthread_join              and joined
    posix     pthread_self              which thread this is, for a pinned module
    posix     clock_gettime             the clock a deadline is kept by
    posix     pthread_mutex_init        a mutex made
    posix     pthread_mutex_lock        locked
    posix     pthread_mutex_unlock      unlocked
    posix     pthread_mutex_destroy     and freed
    posix     pthread_cond_init         an event made
    posix     pthread_cond_signal       set
    posix     pthread_cond_wait         waited for
    posix     pthread_cond_timedwait    waited for until a deadline
    posix     pthread_cond_destroy      and freed

### What the compiler's startup code imports

Nothing of ours calls these. zig links its startup code into every shared library it makes, and
on Windows that code imports them: it runs the library's initialisers, applies its relocations,
and says what went wrong if it cannot. They are here because they are imports all the same, and a
new one is a dependency that changed.

    windows   _initterm                 the initialisers run
    windows   _initterm_e
    windows   _initialize_onexit_table  the table of what runs at exit
    windows   _register_onexit_function
    windows   _execute_onexit_table
    windows   _exit                     the process ended, if the startup fails
    windows   abort
    windows   __acrt_iob_func           where it says so
    windows   __stdio_common_vfprintf
    windows   fwrite
    windows   VirtualProtect            the library's relocations applied
    windows   VirtualQuery
    windows   GetLastError
    windows   TlsGetValue               its thread-local storage
    windows   Sleep                     its start, waited for
    macos     dyld_stub_binder          the loader's own, which binds an import at its first call
