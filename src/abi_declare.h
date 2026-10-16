/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_ABI_DECLARE_H
    #define BRIDGE_SRC_ABI_DECLARE_H

    #include "abi.h"

typedef struct {
    char text[160];
} abi_others_t;

typedef struct {
    module_t *m;
    bridge_declaration_t d;
    const char *desc;
    char name[NAME_MOST + 1];
    char *bare;
    uint64_t print;
    declared_type_t *c;
    declared_type_t *was;
    schema_t s;
} declaring_t;

typedef struct {
    module_t *m;
    uint32_t rows;
    storage_run_t *runs;
    uint32_t run_count;
    char lost[240];
    size_t lost_len;
    bool short_of;
} relaying_t;

typedef struct {
    const schema_t *was;
    const schema_t *is;
    uint32_t from;
    uint32_t to;
    const char *path;
} relay_at_t;

static const char *const ABI_STORAGE_NAMES[] = {
    "",
    "a table",
    "a sparse set",
    "a toggle",
    "direct-indexed",
    "a singleton"
};

uint64_t abi_hash_text(const char *s, size_t n);

bool abi_is_listed(char **list, uint32_t count, const char *name);

bool abi_is_foreign(const module_t *m, const char *name);

bool abi_is_writable(const module_t *m, const declared_type_t *c);

bool abi_find_type(void *ctx, const char *bare, schema_named_t *named);

void abi_say_others(const module_t *m, const declared_type_t *c,
    abi_others_t *others);

bool abi_accepts(module_t *m, const char *name, uint32_t rows);

uint32_t abi_get_element_size(const schema_field_t *f);

void abi_add_run(relaying_t *l, uint32_t from, uint32_t to, uint32_t len);

void abi_say_lost(relaying_t *l, const char *name);

bool abi_names_superseded(const declared_type_t *c);

const declared_type_t *abi_find_holder(const module_t *m,
    const declared_type_t *c, const module_t **by);

int abi_check_move(module_t *m, const declared_type_t *c,
    enum bridge_storage kept);

int abi_relay_fields(relaying_t *l, const relay_at_t *at);

int abi_relay(module_t *m, declared_type_t *was, enum bridge_kind kind,
    const schema_t *is);

int abi_check_stated(declaring_t *g);

int abi_read_layout(declaring_t *g);

int abi_check_size(declaring_t *g, uint32_t size, uint32_t align);

int abi_replace_type(declaring_t *g);

int abi_make_type(declaring_t *g);

#endif
