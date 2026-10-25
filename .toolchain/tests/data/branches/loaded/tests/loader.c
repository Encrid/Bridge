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

typedef int (*choose_fn_t)(int x);

#ifdef _WIN32

static int run(const char *path, int x)
{
    HMODULE lib = LoadLibraryA(path);
    FARPROC found = lib ? GetProcAddress(lib, "choose") : nullptr;
    choose_fn_t choose = nullptr;
    int got;

    if (!found)
        return (printf("no chooser\n"), 1);
    memcpy(&choose, &found, sizeof(choose));
    got = choose(x);
    FreeLibrary(lib);
    printf("chose %d\n", got);
    return (0);
}

#else

static int run(const char *path, int x)
{
    void *lib = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    void *found = lib ? dlsym(lib, "choose") : nullptr;
    choose_fn_t choose = nullptr;
    int got;

    if (!found)
        return (printf("no chooser\n"), 1);
    memcpy(&choose, &found, sizeof(choose));
    got = choose(x);
    dlclose(lib);
    printf("chose %d\n", got);
    return (0);
}

#endif

int main(int argc, [[maybe_unused]] char **argv)
{
    int status = run(CHOOSER_PATH, 1);

    if (argc > 1)
        return (status);
    return (status + run(CHOOSER_PATH, 0));
}
