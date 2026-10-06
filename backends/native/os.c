#include "os.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string.h>

/* A module named by a path is looked for there, and so is what it needs beside it. Windows
 * looks beside a library only when asked, and only for a full path written its own way. A bare
 * name is left to the operating system's search, as on every platform. */
void *os_lib_open(const char *path)
{
    char full[MAX_PATH];
    DWORD len;
    if (!strpbrk(path, "/\\")) return (void *)LoadLibraryA(path);
    len = GetFullPathNameA(path, sizeof full, full, nullptr);
    if (!len || len >= sizeof full) return nullptr;
    return (void *)LoadLibraryExA(full, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
}
void *os_lib_sym(void *lib, const char *name) { return (void *)GetProcAddress((HMODULE)lib, name); }
void  os_lib_close(void *lib) { if (lib) FreeLibrary((HMODULE)lib); }
uint64_t os_thread_id() { return GetCurrentThreadId(); }

#else

#include <dlfcn.h>
#include <pthread.h>

void *os_lib_open(const char *path) { return dlopen(path, RTLD_NOW | RTLD_LOCAL); }
void *os_lib_sym(void *lib, const char *name) { return dlsym(lib, name); }
void  os_lib_close(void *lib) { if (lib) dlclose(lib); }
uint64_t os_thread_id() { return (uint64_t)(uintptr_t)pthread_self(); }

#endif
