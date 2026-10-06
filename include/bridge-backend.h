/* The backend seam: what Bridge-ASM700 asks of the thing that runs a module.
 * docs/seam/backend/backend.md is the contract; this header is its C form.
 *
 * A backend is a shared library exporting the seventeen functions below by name.
 * Only a backend and Bridge-ASM700 compile this header. */
#ifndef BRIDGE_BACKEND_H
#define BRIDGE_BACKEND_H

#include <stddef.h>
#include <stdint.h>

#ifdef _WIN32
#define BACKEND_EXPORT [[gnu::dllexport]]
#else
#define BACKEND_EXPORT [[gnu::visibility("default")]]
#endif

typedef struct backend          backend;
typedef struct backend_memory   backend_memory;
typedef struct backend_module   backend_module;
typedef struct backend_instance backend_instance;
typedef struct backend_func     backend_func;

/* Why an entry failed. The caller owns the buffer, so there is no lifetime to get wrong. */
typedef struct { char text[256]; } backend_err;

/* What a backend can actually do - capabilities.md. */
enum backend_capability : uint32_t {
    BACKEND_SANDBOX  = 1 << 0,
    BACKEND_METER    = 1 << 1,
    BACKEND_DEADLINE = 1 << 2,
    BACKEND_LIMIT    = 1 << 3,
    BACKEND_RELOAD   = 1 << 4,
    /* two instances of one module can exist at once: which order a reload takes */
    BACKEND_SIDE_BY_SIDE = 1 << 5,
    /* its library can be taken out of the process once backend_destroy has returned. A backend
     * that brought in a runtime with threads of its own does not say so, and stays loaded */
    BACKEND_UNLOAD = 1 << 6
};

/* A setting is the backend's own: Bridge-ASM700 passes each through unread. A backend refuses a
 * key it does not know, by name, as Bridge-ASM700 refuses one of its own it does not.
 *
 * It is the host seam's bridge_setting again, on purpose: a seam's header is whole without the
 * other's. Bridge-ASM700 holds the two to one shape, where it copies one to the other. */
enum backend_setting_kind : uint8_t {
    BACKEND_BOOL = 1, BACKEND_INT = 2, BACKEND_FLOAT = 3, BACKEND_STRING = 4
};
typedef struct {
    const char *key;
    enum backend_setting_kind kind;
    union { bool b; int64_t i; double f; const char *s; } v;
} backend_setting;

/* A signature is declared going in and checked coming out - signatures.md. */
enum backend_type : uint8_t { BACKEND_I32 = 1, BACKEND_I64 = 2 };
typedef struct {
    const enum backend_type *params;  uint32_t param_count;
    const enum backend_type *results; uint32_t result_count;
} backend_sig;

/* A function Bridge-ASM700 provides to a module. Every value travels as 64 bits. Non-zero says
 * the call that reached it must not go on: a backend that can unwind a module does so there. */
typedef int (*backend_fn)(void *ud, const uint64_t *args, uint64_t *results);

enum backend_import_kind : uint8_t {
    BACKEND_IMPORT_FUNC = 1, BACKEND_IMPORT_MEMORY = 2, BACKEND_IMPORT_OTHER = 3
};
typedef struct {
    const char *ns;    size_t ns_len;       /* not NUL-terminated */
    const char *field; size_t field_len;
    enum backend_import_kind kind;
} backend_import;

/* Which adopted memory answers which of a module's imports. */
typedef struct {
    const char     *ns, *field;
    backend_memory *memory;
} backend_memory_import;

typedef struct backend_api {
    backend *(*backend_create)(const backend_setting *settings, uint32_t count, backend_err *err);
    void     (*backend_destroy)(backend *b);
    enum backend_capability (*backend_capabilities)(backend *b);

    /* The address space this backend needs around a block: `reservation` bytes from its
     * start, then `guard` bytes nothing is mapped into. Zero for both if it needs none. */
    void            (*memory_shape)(backend *b, uint64_t *reservation, uint64_t *guard);
    backend_memory *(*memory_adopt)(backend *b, void *ptr, uint64_t len, backend_err *err);
    /* After the last instance the memory was handed to is destroyed, never before. */
    void            (*memory_release)(backend_memory *m);

    /* `demands` is the capability flags this module's config asks for: a backend need not
     * compile in a guarantee nobody demanded, and may never enforce less than was. */
    backend_module *(*module_compile)(backend *b, const void *bytes, size_t len,
                                      const char *path, enum backend_capability demands,
                                      backend_err *err);
    uint32_t        (*module_imports)(backend_module *m, backend_import *out, uint32_t max);
    void            (*module_destroy)(backend_module *m);

    backend_instance *(*instance_create)(backend *b, backend_module *m,
                                         const backend_memory_import *memories, uint32_t count,
                                         backend_err *err);
    void (*instance_destroy)(backend_instance *i);
    void (*instance_set_fuel)(backend_instance *i, uint64_t units);
    void (*instance_set_deadline)(backend_instance *i, uint64_t nanos);     /* the next call only */
    void (*instance_set_memory_limit)(backend_instance *i, uint64_t bytes);

    int (*define_func)(backend_instance *i, const char *ns, const char *field,
                       backend_fn fn, void *ud, const backend_sig *sig, backend_err *err);
    /* An export that `call` can run: one that takes nothing of Bridge-ASM700's and answers a
     * status. nullptr, and why, if there is none of that name, or if the backend can see that
     * its shape is another. */
    backend_func *(*instance_func)(backend_instance *i, const char *name, backend_err *err);
    /* 0 if the function returned, and then `status` is what it returned. */
    int (*call)(backend_func *f, int32_t *status, backend_err *err);
} backend_api;

/* The names a backend exports, in the order of backend_api's members. */
#define BACKEND_ENTRIES \
    "backend_create", "backend_destroy", "backend_capabilities", \
    "memory_shape", "memory_adopt", "memory_release", \
    "module_compile", "module_imports", "module_destroy", \
    "instance_create", "instance_destroy", \
    "instance_set_fuel", "instance_set_deadline", "instance_set_memory_limit", \
    "define_func", "instance_func", "call"
constexpr uint32_t BACKEND_ENTRY_COUNT = 17;
static_assert(sizeof(backend_api) == BACKEND_ENTRY_COUNT * sizeof(void (*)()));

#endif
