/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

typedef int (*make_fn_t)(void);
typedef void (*drop_fn_t)(void);

#ifdef _WIN32

static int run(const char *path, bool drop)
{
    HMODULE lib = LoadLibraryA(path);
    FARPROC made = lib ? GetProcAddress(lib, "chunk_make") : nullptr;
    FARPROC dropped = lib ? GetProcAddress(lib, "chunk_drop") : nullptr;
    make_fn_t make = nullptr;
    drop_fn_t gone = nullptr;

    if (!made || !dropped)
        return (printf("no chunk\n"), 1);
    memcpy(&make, &made, sizeof(make));
    memcpy(&gone, &dropped, sizeof(gone));
    printf("made %d\n", make());
    if (drop)
        gone();
    FreeLibrary(lib);
    printf("%s\n", drop ? "dropped" : "kept");
    return (0);
}

#else

static int run(const char *path, bool drop)
{
    void *lib = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    void *made = lib ? dlsym(lib, "chunk_make") : nullptr;
    void *dropped = lib ? dlsym(lib, "chunk_drop") : nullptr;
    make_fn_t make = nullptr;
    drop_fn_t gone = nullptr;

    if (!made || !dropped)
        return (printf("no chunk\n"), 1);
    memcpy(&make, &made, sizeof(make));
    memcpy(&gone, &dropped, sizeof(gone));
    printf("made %d\n", make());
    if (drop)
        gone();
    dlclose(lib);
    printf("%s\n", drop ? "dropped" : "kept");
    return (0);
}

#endif

int main(int argc, [[maybe_unused]] char **argv)
{
    return (run(CHUNK_PATH, argc < 2));
}
