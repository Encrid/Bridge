/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_ABI_H
    #define BRIDGE_ABI_H

    #include <stddef.h>
    #include <stdint.h>

    #ifdef __cplusplus
extern "C" {
    #endif

constexpr uint32_t BRIDGE_ABI = 3;

constexpr int32_t BRIDGE_END = INT32_MAX;

constexpr char BRIDGE_NAMESPACE[] = "bridge";
constexpr char BRIDGE_SHARED[] = "shared";
constexpr char BRIDGE_PRIVATE[] = "private";

enum bridge_kind : uint32_t {
    BRIDGE_STRUCT = 1,
    BRIDGE_ENUM = 2,
    BRIDGE_BITMASK = 3,
    BRIDGE_TAG = 4
};

constexpr uint32_t BRIDGE_CARRIED = 1;

enum bridge_on_unload : uint32_t {
    BRIDGE_STRIP = 1,
    BRIDGE_SWEEP = 2,
    BRIDGE_DELETE = 3
};

enum bridge_on_destroy : uint32_t {
    BRIDGE_ON_DESTROY_REMOVE = 1,
    BRIDGE_ON_DESTROY_KEEP = 2
};

enum bridge_storage : uint32_t {
    BRIDGE_TABLE = 1,
    BRIDGE_SPARSE_SET = 2,
    BRIDGE_TOGGLE = 3,
    BRIDGE_DIRECT_INDEXED = 4,
    BRIDGE_SINGLETON = 5
};

enum bridge_access : uint8_t {
    BRIDGE_READ = 1,
    BRIDGE_WRITE = 2
};

enum bridge_oper : uint8_t {
    BRIDGE_AND = 1,
    BRIDGE_NOT = 2,
    BRIDGE_OPTIONAL = 3
};

typedef struct {
    uint32_t slot;
    enum bridge_access access;
    enum bridge_oper oper;
    uint16_t pad;
} bridge_term_t;
static_assert(sizeof(bridge_term_t) == 8);
static_assert(alignof(bridge_term_t) == 4);
static_assert(offsetof(bridge_term_t, slot) == 0);
static_assert(offsetof(bridge_term_t, access) == 4);
static_assert(offsetof(bridge_term_t, oper) == 5);
static_assert(offsetof(bridge_term_t, pad) == 6);

enum bridge_event : uint32_t {
    BRIDGE_ON_ENTER = 1,
    BRIDGE_ON_ATTACH = 2,
    BRIDGE_ON_LEAVE = 4
};

typedef struct {
    uint64_t id;
    enum bridge_event events;
    uint32_t pad;
} bridge_trigger_t;
static_assert(sizeof(bridge_trigger_t) == 16);
static_assert(alignof(bridge_trigger_t) == 8);
static_assert(offsetof(bridge_trigger_t, id) == 0);
static_assert(offsetof(bridge_trigger_t, events) == 8);
static_assert(offsetof(bridge_trigger_t, pad) == 12);

typedef struct {
    uint32_t name;
    uint32_t name_len;
    enum bridge_kind kind;
    uint32_t size;
    uint32_t align;
    uint32_t desc;
    uint32_t desc_len;
    enum bridge_on_unload on_unload;
    enum bridge_storage storage;
    enum bridge_on_destroy on_destroy;
} bridge_declaration_t;
static_assert(sizeof(bridge_declaration_t) == 40);
static_assert(alignof(bridge_declaration_t) == 4);
static_assert(offsetof(bridge_declaration_t, name) == 0);
static_assert(offsetof(bridge_declaration_t, name_len) == 4);
static_assert(offsetof(bridge_declaration_t, kind) == 8);
static_assert(offsetof(bridge_declaration_t, size) == 12);
static_assert(offsetof(bridge_declaration_t, align) == 16);
static_assert(offsetof(bridge_declaration_t, desc) == 20);
static_assert(offsetof(bridge_declaration_t, desc_len) == 24);
static_assert(offsetof(bridge_declaration_t, on_unload) == 28);
static_assert(offsetof(bridge_declaration_t, storage) == 32);
static_assert(offsetof(bridge_declaration_t, on_destroy) == 36);

typedef struct {
    uint32_t name;
    uint32_t name_len;
    uint32_t phase;
    uint32_t phase_len;
    uint32_t terms;
    uint32_t term_count;
    uint32_t changes;
    uint32_t change_count;
    uint32_t before;
    uint32_t before_count;
} bridge_system_statement_t;
static_assert(sizeof(bridge_system_statement_t) == 40);
static_assert(alignof(bridge_system_statement_t) == 4);
static_assert(offsetof(bridge_system_statement_t, name) == 0);
static_assert(offsetof(bridge_system_statement_t, name_len) == 4);
static_assert(offsetof(bridge_system_statement_t, phase) == 8);
static_assert(offsetof(bridge_system_statement_t, phase_len) == 12);
static_assert(offsetof(bridge_system_statement_t, terms) == 16);
static_assert(offsetof(bridge_system_statement_t, term_count) == 20);
static_assert(offsetof(bridge_system_statement_t, changes) == 24);
static_assert(offsetof(bridge_system_statement_t, change_count) == 28);
static_assert(offsetof(bridge_system_statement_t, before) == 32);
static_assert(offsetof(bridge_system_statement_t, before_count) == 36);

typedef struct {
    uint32_t name;
    uint32_t name_len;
    enum bridge_event events;
    uint32_t terms;
    uint32_t term_count;
    uint32_t changes;
    uint32_t change_count;
    uint32_t before;
    uint32_t before_count;
} bridge_observer_statement_t;
static_assert(sizeof(bridge_observer_statement_t) == 36);
static_assert(alignof(bridge_observer_statement_t) == 4);
static_assert(offsetof(bridge_observer_statement_t, name) == 0);
static_assert(offsetof(bridge_observer_statement_t, name_len) == 4);
static_assert(offsetof(bridge_observer_statement_t, events) == 8);
static_assert(offsetof(bridge_observer_statement_t, terms) == 12);
static_assert(offsetof(bridge_observer_statement_t, term_count) == 16);
static_assert(offsetof(bridge_observer_statement_t, changes) == 20);
static_assert(offsetof(bridge_observer_statement_t, change_count) == 24);
static_assert(offsetof(bridge_observer_statement_t, before) == 28);
static_assert(offsetof(bridge_observer_statement_t, before_count) == 32);

constexpr uint32_t BRIDGE_ZEROED = UINT32_MAX;

typedef struct {
    uint32_t abi;
    uint32_t scratch;
    uint32_t scratch_len;
    uint32_t scratch_used;
    uint32_t ring;
    uint32_t ring_len;
    uint32_t ring_head;
    uint32_t ring_tail;
    uint32_t queries;
    uint32_t column_base;
    uint32_t types;
    uint32_t tables;
    uint32_t entity_index;
    uint32_t entities;
    uint32_t barriers;
    uint32_t place;
    uint32_t system;
    uint32_t observer;
    uint32_t log_level;
    uint32_t observers;
    uint32_t kept;
    uint32_t rows;
    uint32_t blocks;
    uint32_t stride;
    uint32_t write;
    uint32_t kind;
    uint32_t owned;
    uint32_t arena_lo;
    uint32_t arena_hi;
    uint32_t changes_left;
} bridge_header_t;
static_assert(sizeof(bridge_header_t) == 120);
static_assert(alignof(bridge_header_t) == 4);
static_assert(offsetof(bridge_header_t, abi) == 0);
static_assert(offsetof(bridge_header_t, scratch) == 4);
static_assert(offsetof(bridge_header_t, scratch_len) == 8);
static_assert(offsetof(bridge_header_t, scratch_used) == 12);
static_assert(offsetof(bridge_header_t, ring) == 16);
static_assert(offsetof(bridge_header_t, ring_len) == 20);
static_assert(offsetof(bridge_header_t, ring_head) == 24);
static_assert(offsetof(bridge_header_t, ring_tail) == 28);
static_assert(offsetof(bridge_header_t, queries) == 32);
static_assert(offsetof(bridge_header_t, column_base) == 36);
static_assert(offsetof(bridge_header_t, types) == 40);
static_assert(offsetof(bridge_header_t, tables) == 44);
static_assert(offsetof(bridge_header_t, entity_index) == 48);
static_assert(offsetof(bridge_header_t, entities) == 52);
static_assert(offsetof(bridge_header_t, barriers) == 56);
static_assert(offsetof(bridge_header_t, place) == 60);
static_assert(offsetof(bridge_header_t, system) == 64);
static_assert(offsetof(bridge_header_t, observer) == 68);
static_assert(offsetof(bridge_header_t, log_level) == 72);
static_assert(offsetof(bridge_header_t, observers) == 76);
static_assert(offsetof(bridge_header_t, kept) == 80);
static_assert(offsetof(bridge_header_t, rows) == 84);
static_assert(offsetof(bridge_header_t, blocks) == 88);
static_assert(offsetof(bridge_header_t, stride) == 92);
static_assert(offsetof(bridge_header_t, write) == 96);
static_assert(offsetof(bridge_header_t, kind) == 100);
static_assert(offsetof(bridge_header_t, owned) == 104);
static_assert(offsetof(bridge_header_t, arena_lo) == 108);
static_assert(offsetof(bridge_header_t, arena_hi) == 112);
static_assert(offsetof(bridge_header_t, changes_left) == 116);

typedef struct {
    uint32_t spans;
    uint32_t at;
} bridge_query_t;
static_assert(sizeof(bridge_query_t) == 8);
static_assert(alignof(bridge_query_t) == 4);
static_assert(offsetof(bridge_query_t, spans) == 0);
static_assert(offsetof(bridge_query_t, at) == 4);

typedef struct {
    uint32_t at;
    uint32_t len;
} bridge_text_t;
static_assert(sizeof(bridge_text_t) == 8);
static_assert(alignof(bridge_text_t) == 4);
static_assert(offsetof(bridge_text_t, at) == 0);
static_assert(offsetof(bridge_text_t, len) == 4);

typedef struct {
    uint32_t count;
    uint32_t at;
} bridge_list_t;
static_assert(sizeof(bridge_list_t) == 8);
static_assert(alignof(bridge_list_t) == 4);
static_assert(offsetof(bridge_list_t, count) == 0);
static_assert(offsetof(bridge_list_t, at) == 4);

constexpr uint32_t BRIDGE_OWNED_CAP = 8;

constexpr uint32_t BRIDGE_SPAN_SIZE = 4;

typedef struct {
    uint32_t table;
    uint32_t row;
    uint32_t generation;
} bridge_entity_t;
static_assert(sizeof(bridge_entity_t) == 12);
static_assert(alignof(bridge_entity_t) == 4);
static_assert(offsetof(bridge_entity_t, table) == 0);
static_assert(offsetof(bridge_entity_t, row) == 4);
static_assert(offsetof(bridge_entity_t, generation) == 8);

enum bridge_log : uint8_t {
    BRIDGE_LOG_ERR = 3,
    BRIDGE_LOG_WARN = 4,
    BRIDGE_LOG_NOTICE = 5,
    BRIDGE_LOG_INFO = 6,
    BRIDGE_LOG_DEBUG = 7
};

typedef struct {
    uint32_t barriers;
    uint32_t place;
    uint32_t system;
    uint8_t observer;
    uint8_t level;
    uint8_t pad[2];
    uint32_t text;
    uint32_t len;
    uint32_t arg[2];
} bridge_log_record_t;
static_assert(sizeof(bridge_log_record_t) == 32);
static_assert(alignof(bridge_log_record_t) == 4);
static_assert(offsetof(bridge_log_record_t, barriers) == 0);
static_assert(offsetof(bridge_log_record_t, place) == 4);
static_assert(offsetof(bridge_log_record_t, system) == 8);
static_assert(offsetof(bridge_log_record_t, observer) == 12);
static_assert(offsetof(bridge_log_record_t, level) == 13);
static_assert(offsetof(bridge_log_record_t, pad) == 14);
static_assert(offsetof(bridge_log_record_t, text) == 16);
static_assert(offsetof(bridge_log_record_t, len) == 20);
static_assert(offsetof(bridge_log_record_t, arg) == 24);

typedef struct {
    uint32_t (*declare)(uint32_t at);
    uint32_t (*add_system)(uint32_t at);
    uint32_t (*observe)(uint32_t at);
    uint64_t (*spawn)(void);
    void (*destroy)(uint64_t id);
    void (*attach)(uint64_t id, uint32_t slot, uint32_t ptr);
    void (*detach)(uint64_t id, uint32_t slot);
    uint32_t (*alloc)(uint32_t len);
} bridge_entries_t;

    #ifdef __cplusplus
}
    #endif

#endif
