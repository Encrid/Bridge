/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_STORAGE_INSIDE_H
    #define BRIDGE_SRC_STORAGE_INSIDE_H

    #include "inside.h"

constexpr uint32_t DEAD = 0xffffffffu;

constexpr uint32_t HELD = 0xffffffffu;

constexpr uint64_t COLUMN = 16;

enum folded : uint8_t {
    TOUCHED = 1,
    DESTROYED = 2,
    DROPPED = 4,
    SHORT = 8,
    MOVES = 16
};

struct storage_type {
    uint32_t id;
    uint32_t size;
    uint32_t align;
    enum bridge_storage kept;
    uint32_t slot;
    uint32_t cells;
    uint32_t cells_len;
    bool set;
};

struct storage_terms {
    uint32_t count;
    uint32_t *type;
    enum bridge_oper *oper;
    uint32_t *list;
    uint32_t *pos;
    uint32_t matched;
    bool changed;
    bool used;
    enum bridge_event events;
    storage_heard_t heard;
    void *ctx;
};

typedef struct {
    uint32_t table;
    uint32_t row;
    uint32_t generation;
} place_t;

typedef struct {
    uint32_t rows;
    uint32_t room;
    uint32_t block;
    uint32_t block_len;
    uint32_t incoming;
    bool live;
    bool touched;
    bool sized;
    bool named;
    uint32_t set;
} table_t;

struct storage {
    bridge_world_t *w;
    place_t *places;
    uint32_t high;
    uint32_t free_head;
    table_t *tables;
    uint32_t table_room;
    uint32_t table_free;
    uint32_t table_high;
    uint32_t *table_next;
    uint32_t *hash;
    uint32_t hash_room;
    uint64_t *sets;
    uint32_t *columns;
    uint64_t *col_hash;
    uint32_t words;
    uint32_t type_room;
    uint32_t terms_room;
    uint64_t *who;
    uint32_t *dirty;
    uint32_t dirty_count;
    storage_type_t **types;
    storage_type_t *type_pool;
    storage_terms_t *terms_pool;
    uint32_t *lists;
    uint32_t *term_types;
    enum bridge_oper *term_opers;
    storage_terms_t **systems;
    storage_terms_t **observers;
    uint32_t system_count;
    uint32_t observer_count;
    uint64_t *work;
    uint32_t *changed;
    uint32_t changed_count;
    uint8_t *folded;
    uint32_t *dest;
    uint64_t *at;
    uint64_t *attached;
    uint64_t *scratch;
    uint32_t set_room;
    storage_type_t **set_type;
    uint32_t *set_table;
    uint32_t *where;
    uint64_t *in_table;
    uint64_t *before;
    uint64_t *masked;
    uint32_t table_count;
    uint32_t tags;
    uint32_t apart;
    uint32_t direct_room;
    uint32_t bit_words;
    storage_type_t **direct_type;
    uint64_t *given;
};

typedef struct {
    uint32_t index;
    uint32_t from;
    uint32_t row;
    uint32_t to;
    uint32_t at;
} row_move_t;

typedef struct {
    uint64_t id;
    const uint64_t *before;
    const uint64_t *after;
    const uint64_t *attached;
} seen_t;

typedef struct {
    storage_read_fn_t read;
    void *ctx;
} value_reader_t;

typedef struct {
    storage_t *r;
    const storage_move_t *moves;
    uint32_t count;
    uint8_t *was;
    uint32_t *had;
} convert_t;

#endif
