#include "backend.h"
#include "os.h"

#include <stdio.h>

/* A backend is a library exporting seventeen names. Every member of backend_api is a function
 * pointer, so the struct is filled as an array of them, in the order BACKEND_ENTRIES gives. */
void *backend_open(const char *path, backend_api *api, char *why, size_t why_len)
{
    static const char *const names[BACKEND_ENTRY_COUNT] = { BACKEND_ENTRIES };
    void **slots = (void **)api;
    void *lib = os_lib_open(path);
    int i;
    if (!lib) {
        snprintf(why, why_len, "cannot open backend library '%s'", path);
        return NULL;
    }
    for (i = 0; i < BACKEND_ENTRY_COUNT; i ++) {
        slots[i] = os_lib_sym(lib, names[i]);
        if (!slots[i]) {
            snprintf(why, why_len, "backend library '%s' has no entry '%s'", path, names[i]);
            os_lib_close(lib);
            return NULL;
        }
    }
    return lib;
}
