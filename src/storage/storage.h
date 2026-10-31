/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_STORAGE_H
    #define BRIDGE_SRC_STORAGE_H

    #include "bridge_abi.h"

constexpr uint32_t STORAGE_HASH_MOST = 1u << 31;

typedef struct bridge_world bridge_world_t;
typedef struct module module_t;
typedef struct storage storage_t;
typedef struct storage_type storage_type_t;
typedef struct storage_terms storage_terms_t;

typedef struct {
    storage_type_t *type;
    enum bridge_oper oper;
} storage_term_t;

typedef void (*storage_heard_t)(void *ctx, uint64_t id,
    enum bridge_event event);

typedef struct {
    enum bridge_event events;
    storage_heard_t heard;
    void *ctx;
} storage_listener_t;

typedef struct {
    uint32_t from;
    uint32_t to;
    uint32_t len;
} storage_run_t;

typedef struct {
    storage_type_t *from;
    storage_type_t *to;
    const storage_run_t *runs;
    uint32_t run_count;
} storage_move_t;

typedef struct {
    uint32_t entities;
    uint32_t entities_peak;
    uint32_t tables;
    uint32_t tables_peak;
} storage_usage_t;

typedef void (*storage_read_fn_t)(void *ctx, const uint8_t *value);
typedef void (*storage_visit_fn_t)(storage_t *r, uint32_t index, void *ctx);
typedef void (*storage_changed_fn_t)(void *ctx, const storage_type_t *type);

[[nodiscard]] storage_t *storage_create(bridge_world_t *w);

void storage_destroy(storage_t *r);

[[nodiscard]] storage_type_t *storage_declare(storage_t *r, uint32_t size,
    uint32_t align, enum bridge_storage kept);

uint32_t storage_count_free_sets(const storage_t *r);

uint32_t storage_count_free_direct(const storage_t *r);

void storage_undeclare(storage_t *r, storage_type_t *type);

[[nodiscard]] storage_terms_t *storage_add_system(storage_t *r,
    const storage_term_t *terms, uint32_t count);

[[nodiscard]] storage_terms_t *storage_add_observer(storage_t *r,
    const storage_term_t *terms, uint32_t count,
    const storage_listener_t *listener);

void storage_forget(storage_t *r, storage_terms_t *terms);

void storage_apply(storage_t *r);

void storage_validate_scan(storage_t *r, storage_changed_fn_t fn, void *ctx);

void storage_validate_baseline(storage_t *r);

[[nodiscard]] bool storage_make_writable(storage_t *r, module_t *m);

void storage_let_write(storage_t *r, module_t *m, storage_type_t *type);

void storage_let_keep(storage_t *r, const storage_type_t *type, bool keep);

uint32_t storage_reserve(storage_t *r, uint64_t *ids, uint32_t want);

[[nodiscard]] int storage_publish(storage_t *r, module_t *m,
    const char **list);

void storage_end_publish(storage_t *r);

uint32_t storage_count_tables(storage_t *r);

storage_usage_t storage_usage(const storage_t *r);

bool storage_is_dirty(const storage_t *r);

uint32_t storage_count_carriers(storage_t *r, const storage_type_t *type);

uint32_t storage_count_off(storage_t *r, const storage_type_t *type);

void storage_visit_carriers(storage_t *r, const storage_type_t *type,
    storage_visit_fn_t visit, void *ctx);

enum storage_moved : uint8_t {
    STORAGE_MOVED = 1,
    STORAGE_NO_ROOM = 2,
    STORAGE_NO_MEMORY = 3
};

[[nodiscard]] enum storage_moved storage_convert(storage_t *r,
    const storage_move_t *moves, uint32_t count);

void storage_drop(storage_t *r, storage_type_t *type,
    enum bridge_on_unload policy);

void storage_release(storage_t *r, const uint64_t *ids, uint32_t count);

void storage_hand_out(storage_t *r, const uint64_t *ids, uint32_t count);

void storage_give_single(storage_t *r, storage_type_t *type,
    const void *bytes, uint32_t size);

bool storage_is_alive(storage_t *r, uint64_t id);

void storage_visit_values(storage_t *r, storage_type_t *type,
    storage_read_fn_t read, void *ctx);

#endif
