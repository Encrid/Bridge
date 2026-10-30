/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_BACKEND_H
    #define BRIDGE_BACKEND_H

    #include <stddef.h>
    #include <stdint.h>

    #ifdef __cplusplus
extern "C" {
    #endif

    #ifdef _WIN32
        #define BACKEND_EXPORT [[gnu::dllexport]]
    #else
        #define BACKEND_EXPORT [[gnu::visibility("default")]]
    #endif

typedef struct backend backend_t;
typedef struct backend_memory backend_memory_t;
typedef struct backend_module backend_module_t;
typedef struct backend_instance backend_instance_t;
typedef struct backend_func backend_func_t;

typedef struct {
    char text[256];
} backend_err_t;
static_assert(sizeof(backend_err_t) == 256);
static_assert(alignof(backend_err_t) == 1);

enum backend_capability : uint32_t {
    BACKEND_WORK_LIMIT = 1 << 1,
    BACKEND_TIMEOUT = 1 << 2,
    BACKEND_MEMORY_LIMIT = 1 << 3,
    BACKEND_RELOAD = 1 << 4,
    BACKEND_SIDE_BY_SIDE = 1 << 5,
    BACKEND_UNLOAD = 1 << 6,
    BACKEND_ELEMENT_LIMIT = 1 << 7
};

enum backend_setting_kind : uint8_t {
    BACKEND_BOOL = 1,
    BACKEND_INT = 2,
    BACKEND_FLOAT = 3,
    BACKEND_STRING = 4
};

typedef union {
    bool b;
    int64_t i;
    double f;
    const char *s;
} backend_value_t;

typedef struct {
    const char *key;
    enum backend_setting_kind kind;
    uint8_t pad[7];
    backend_value_t v;
} backend_setting_t;
static_assert(sizeof(void *) == 8);
static_assert(sizeof(size_t) == 8);
static_assert(sizeof(backend_setting_t) == 24);
static_assert(alignof(backend_setting_t) == 8);
static_assert(offsetof(backend_setting_t, key) == 0);
static_assert(offsetof(backend_setting_t, kind) == 8);
static_assert(offsetof(backend_setting_t, pad) == 9);
static_assert(offsetof(backend_setting_t, v) == 16);

enum backend_type : uint8_t {
    BACKEND_I32 = 1,
    BACKEND_I64 = 2
};

typedef struct {
    const enum backend_type *params;
    uint32_t param_count;
    uint32_t pad0;
    const enum backend_type *results;
    uint32_t result_count;
    uint32_t pad1;
} backend_sig_t;
static_assert(sizeof(backend_sig_t) == 32);
static_assert(alignof(backend_sig_t) == 8);
static_assert(offsetof(backend_sig_t, params) == 0);
static_assert(offsetof(backend_sig_t, param_count) == 8);
static_assert(offsetof(backend_sig_t, pad0) == 12);
static_assert(offsetof(backend_sig_t, results) == 16);
static_assert(offsetof(backend_sig_t, result_count) == 24);
static_assert(offsetof(backend_sig_t, pad1) == 28);

typedef int (*backend_fn_t)(void *ud, const uint64_t *args, uint64_t *results);

enum backend_import_kind : uint8_t {
    BACKEND_IMPORT_FUNC = 1,
    BACKEND_IMPORT_MEMORY = 2,
    BACKEND_IMPORT_OTHER = 3
};

typedef struct {
    const char *ns;
    size_t ns_len;
    const char *field;
    size_t field_len;
    enum backend_import_kind kind;
    uint8_t pad[7];
} backend_import_t;
static_assert(sizeof(backend_import_t) == 40);
static_assert(alignof(backend_import_t) == 8);
static_assert(offsetof(backend_import_t, ns) == 0);
static_assert(offsetof(backend_import_t, ns_len) == 8);
static_assert(offsetof(backend_import_t, field) == 16);
static_assert(offsetof(backend_import_t, field_len) == 24);
static_assert(offsetof(backend_import_t, kind) == 32);
static_assert(offsetof(backend_import_t, pad) == 33);

typedef struct {
    const char *ns;
    const char *field;
    backend_memory_t *memory;
} backend_memory_import_t;
static_assert(sizeof(backend_memory_import_t) == 24);
static_assert(alignof(backend_memory_import_t) == 8);
static_assert(offsetof(backend_memory_import_t, ns) == 0);
static_assert(offsetof(backend_memory_import_t, field) == 8);
static_assert(offsetof(backend_memory_import_t, memory) == 16);

constexpr uint32_t BACKEND_ABI = 3;

typedef struct {
    const void *bytes;
    size_t len;
    const char *path;
    enum backend_capability demands;
    uint32_t pad;
} backend_source_t;
static_assert(sizeof(backend_source_t) == 32);
static_assert(alignof(backend_source_t) == 8);
static_assert(offsetof(backend_source_t, bytes) == 0);
static_assert(offsetof(backend_source_t, len) == 8);
static_assert(offsetof(backend_source_t, path) == 16);
static_assert(offsetof(backend_source_t, demands) == 24);
static_assert(offsetof(backend_source_t, pad) == 28);

typedef struct {
    const char *ns;
    const char *field;
    backend_fn_t fn;
    void *ud;
    const backend_sig_t *sig;
} backend_entry_t;
static_assert(sizeof(backend_entry_t) == 40);
static_assert(alignof(backend_entry_t) == 8);
static_assert(offsetof(backend_entry_t, ns) == 0);
static_assert(offsetof(backend_entry_t, field) == 8);
static_assert(offsetof(backend_entry_t, fn) == 16);
static_assert(offsetof(backend_entry_t, ud) == 24);
static_assert(offsetof(backend_entry_t, sig) == 32);

typedef struct backend_api {
    uint32_t (*backend_get_abi)(void);
    backend_t *(*backend_create)(const backend_setting_t *settings,
        uint32_t count, backend_err_t *err);
    void (*backend_destroy)(backend_t *b);
    enum backend_capability (*backend_get_capabilities)(backend_t *b);
    void (*memory_get_shape)(backend_t *b, uint64_t *reservation,
        uint64_t *guard, uint64_t *granule);
    backend_memory_t *(*memory_adopt)(backend_t *b, void *ptr, uint64_t len,
        backend_err_t *err);
    void (*memory_release)(backend_memory_t *m);
    backend_module_t *(*module_compile)(backend_t *b,
        const backend_source_t *source, backend_err_t *err);
    uint32_t (*module_list_imports)(backend_module_t *m,
        backend_import_t *out, uint32_t max);
    void (*module_destroy)(backend_module_t *m);
    backend_instance_t *(*instance_create)(backend_module_t *m,
        const backend_memory_import_t *memories, uint32_t count,
        backend_err_t *err);
    void (*instance_destroy)(backend_instance_t *i);
    void (*instance_set_work_limit)(backend_instance_t *i, uint64_t units);
    void (*instance_set_timeout)(backend_instance_t *i, uint64_t nanos);
    void (*instance_set_memory_limit)(backend_instance_t *i, uint64_t bytes);
    void (*instance_set_element_limit)(backend_instance_t *i,
        uint64_t elements);
    int (*define_func)(backend_instance_t *i, const backend_entry_t *entry,
        backend_err_t *err);
    backend_func_t *(*instance_get_func)(backend_instance_t *i,
        const char *name, backend_err_t *err);
    int (*call)(backend_func_t *f, int32_t *status, backend_err_t *err);
} backend_api_t;

constexpr uint32_t BACKEND_ENTRY_COUNT = 19;
static const char *const BACKEND_ENTRY_NAMES[BACKEND_ENTRY_COUNT] = {
    "backend_get_abi",
    "backend_create",
    "backend_destroy",
    "backend_get_capabilities",
    "memory_get_shape",
    "memory_adopt",
    "memory_release",
    "module_compile",
    "module_list_imports",
    "module_destroy",
    "instance_create",
    "instance_destroy",
    "instance_set_work_limit",
    "instance_set_timeout",
    "instance_set_memory_limit",
    "instance_set_element_limit",
    "define_func",
    "instance_get_func",
    "call"
};
static_assert(sizeof(backend_api_t) == BACKEND_ENTRY_COUNT * sizeof(void *));

    #ifdef __cplusplus
}
    #endif

#endif
