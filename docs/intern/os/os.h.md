# src/os/os.h

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
