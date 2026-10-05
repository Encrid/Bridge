/* Storage: the shared block, flecs over it, the entity index mirror, and the barrier.
 * docs/bridge/memory/memory.md, docs/bridge/flecs.md, docs/bridge/data/query.md and
 * reference.md, docs/bridge/diagnostics/log.md. */
#include "inside.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ponytail: one bridge per process. flecs_confine is a global, as ecs_os_api is, because the
 * functions that allocate a column have no world in scope. A context argument through the
 * patch is what a second bridge would need. */
static bridge *live;

uint32_t store_offset(bridge *b, const void *p)
{
    const uint8_t *c = p;
    if (!c || c < b->shared || c >= b->shared + b->shared_len) return 0;
    return (uint32_t)(c - b->shared);
}

/* ---- columns: flecs' allocator, carving the column region ---- */

/* ponytail: power-of-two classes, never split or joined, so up to half a column is slack and
 * a freed class is only reused by its own size. A buddy allocator if a deployment's columns
 * outgrow the budget on slack alone. The free lists are kept outside the block: a module's
 * stray write must not be able to reach them. */
static int class_of(size_t size)
{
    int c = 4;
    while (c < 32 && ((size_t)1 << c) < size) c ++;
    return c;
}

static int in_columns(bridge *b, const void *p)
{
    const uint8_t *c = p, *base = b->shared + b->columns;
    return c >= base && c < base + b->columns_len;
}

static void *column_alloc(ecs_size_t size)
{
    bridge *b = live;
    int c;
    if (size <= 0) return NULL;
    b->layout_moved = 1;
    c = class_of((size_t)size);
    if (c < 32) {
        free_list *f = &b->classes[c];
        size_t n = (size_t)1 << c;
        if (f->count) return b->shared + f->at[-- f->count];
        if (n <= b->columns_len - b->columns_used) {
            void *p = b->shared + b->columns + b->columns_used;
            b->columns_used += (uint32_t)n;
            return p;
        }
    }
    /* flecs cannot be unwound from the middle of a change, so it is given what it asked for
     * from outside the block, and the bridge stops when the barrier returns - budgets.md,
     * *What running out does*. No module runs again to read this. */
    b->columns_out = 1;
    return malloc((size_t)size);
}

static void column_free(void *ptr, ecs_size_t size)
{
    bridge *b = live;
    free_list *f;
    if (!ptr) return;
    b->layout_moved = 1;
    if (!in_columns(b, ptr)) {
        free(ptr);
        return;
    }
    f = &b->classes[class_of((size_t)size)];
    if (f->count == f->room) {
        f->room = f->room ? f->room * 2 : 16;
        f->at = realloc(f->at, f->room * sizeof *f->at);
    }
    f->at[f->count ++] = store_offset(b, ptr);
}

static void *column_realloc(void *ptr, ecs_size_t old_size, ecs_size_t new_size)
{
    void *p;
    if (!ptr) return column_alloc(new_size);
    if (new_size <= 0) {
        column_free(ptr, old_size);
        return NULL;
    }
    if (in_columns(live, ptr) && class_of((size_t)old_size) == class_of((size_t)new_size)) return ptr;
    p = column_alloc(new_size);
    memcpy(p, ptr, (size_t)(old_size < new_size ? old_size : new_size));
    column_free(ptr, old_size);
    return p;
}

/* ---- table ids: dense, ours, given the first time a table is seen holding something ---- */

static uint32_t table_id(bridge *b, ecs_table_t *t)
{
    uint32_t mask, i;
    if (!t) return 0;
    if (b->table_count * 2 >= b->table_hash_room) {
        uint32_t room = b->table_hash_room ? b->table_hash_room * 2 : 256, k;
        free(b->table_hash);
        b->table_hash = calloc(room, sizeof *b->table_hash);
        b->table_hash_room = room;
        for (k = 1; k < b->table_count; k ++) {
            i = (uint32_t)(((uintptr_t)b->tables[k].table >> 4) * 2654435761u) & (room - 1);
            while (b->table_hash[i]) i = (i + 1) & (room - 1);
            b->table_hash[i] = k;
        }
    }
    mask = b->table_hash_room - 1;
    i = (uint32_t)(((uintptr_t)t >> 4) * 2654435761u) & mask;
    while (b->table_hash[i]) {
        if (b->tables[b->table_hash[i]].table == t) return b->table_hash[i];
        i = (i + 1) & mask;
    }
    if (b->table_count >= b->table_room) {
        b->table_room = b->table_room ? b->table_room * 2 : 64;
        b->tables = realloc(b->tables, b->table_room * sizeof *b->tables);
    }
    b->tables[b->table_count].table = t;
    b->tables[b->table_count].rows = 0;
    b->table_hash[i] = b->table_count;
    b->layout_moved = 1;
    return b->table_count ++;
}

/* ---- the entity index mirror - flecs.md, *flecs will not tell you which* ---- */

void store_touch(bridge *b, ecs_entity_t e)
{
    uint32_t index = (uint32_t)e;
    if (index >= b->entities || b->is_dirty[index]) return;
    b->is_dirty[index] = 1;
    if (b->dirty_count == b->dirty_room) {
        b->dirty_room = b->dirty_room ? b->dirty_room * 2 : 1024;
        b->dirty = realloc(b->dirty, b->dirty_room * sizeof *b->dirty);
    }
    b->dirty[b->dirty_count ++] = e;
}

/* an event names an entity whose components changed: where it ended up is read at the barrier */
static void on_change(ecs_iter_t *it)
{
    int32_t i;
    for (i = 0; i < it->count; i ++) store_touch(it->ctx, it->entities[i]);
}

/* and the hook names the one no event does: the entity a delete moved into the hole */
static void on_row_moved(ecs_world_t *world, ecs_entity_t e, ecs_table_t *table, int32_t row)
{
    bridge *b = live;
    uint32_t index = (uint32_t)e;
    (void)world;
    if (index >= b->entities) return;
    if (b->index[index].table == table_id(b, table)) b->index[index].row = (uint32_t)row;
    else store_touch(b, e);
}

static void sync(bridge *b)
{
    uint32_t i;
    for (i = 0; i < b->dirty_count; i ++) {
        uint32_t index = (uint32_t)b->dirty[i];
        /* by index, not by the id that was named: the index may have been recycled since */
        ecs_entity_t e = ecs_get_alive(b->world, index);
        bridge_entity *record = &b->index[index], now = { 0, 0, 0xffffffffu };
        if (e) {
            const ecs_record_t *r = ecs_record_find(b->world, e);
            now.table = table_id(b, r->table);
            now.row = (uint32_t)ECS_RECORD_TO_ROW(r->row);
            now.generation = (uint32_t)(e >> 32);
        }
        if (record->table) b->tables[record->table].rows --;
        if (now.table) b->tables[now.table].rows ++;
        *record = now;
        b->is_dirty[index] = 0;
    }
    b->dirty_count = 0;
}

/* ---- publishing: the header, column_base, and one span per matched table ---- */

void store_header(bridge *b, module *m)
{
    bridge_header *h = (bridge_header *)m->priv;
    h->abi = BRIDGE_ABI;
    h->scratch = m->scratch;
    h->scratch_len = m->scratch_len;
    h->ring = m->ring;
    h->ring_len = m->ring_len;
    h->ring_tail = m->ring_tail;
    h->queries = m->spans;
    h->column_base = m->spans + (uint32_t)sizeof(bridge_query) * m->system_count;
    h->terms = m->declared_count;
    h->tables = b->table_count;
    h->entity_index = store_offset(b, b->index);
    h->entities = b->entities;
}

/* 1 if the module's span region cannot hold what its queries match */
static int publish(bridge *b, module *m)
{
    bridge_header *h = (bridge_header *)m->priv;
    uint64_t end = (uint64_t)m->spans + m->spans_len, at;
    uint32_t q, t, s;

    store_header(b, m);
    at = ((uint64_t)h->column_base + 4ull * m->declared_count * b->table_count + 7u) & ~7ull;
    if (at > end) return 1;

    if (b->layout_moved) {
        uint32_t *cell = (uint32_t *)(m->priv + h->column_base);
        memset(cell, 0, 4u * m->declared_count);        /* table 0 is no table */
        for (t = 1; t < b->table_count; t ++) {
            for (s = 0; s < m->declared_count; s ++) {
                ecs_table_t *table = b->tables[t].table;
                int32_t column = ecs_table_get_column_index(b->world, table, m->declared[s]->id);
                cell[t * m->declared_count + s] =
                    column < 0 ? 0 : store_offset(b, ecs_table_get_column(table, column, 0));
            }
        }
    }

    for (q = 0; q < m->system_count; q ++) {
        bridge_system *sys = m->systems[q];
        bridge_query *slot = (bridge_query *)(m->priv + m->spans) + q;
        uint32_t size = BRIDGE_SPAN_SIZE(sys->terms), spans = 0;
        ecs_iter_t it;
        slot->spans = 0;
        slot->at = (uint32_t)at;
        if (!sys->query) continue;
        it = ecs_query_iter(b->world, sys->query);
        while (ecs_query_next(&it)) {
            uint8_t *span = m->priv + at;
            uint32_t id = table_id(b, it.table), value;
            if (at + size > end) {
                ecs_iter_fini(&it);
                return 1;
            }
            /* validation.md, *Always on*: what this catches is our own patch failing */
            if (b->tables[id].rows != it.count) {
                say(b, NULL, NULL, BRIDGE_ERR, "the entity index is out of step with flecs: "
                    "table %u has %d rows and the index counts %d", id, it.count, b->tables[id].rows);
            }
            memset(span, 0, size);
            value = (uint32_t)it.count;
            memcpy(span, &value, 4);
            value = store_offset(b, it.entities);
            memcpy(span + 4, &value, 4);
            for (t = 0; t < sys->terms; t ++) {
                uint64_t source = it.sources[t];
                value = ecs_field_is_set(&it, (int8_t)t)
                    ? store_offset(b, ecs_field_w_size(&it, sys->sizes[t], (int8_t)t)) : 0;
                memcpy(span + BRIDGE_SPAN_BASE + 4u * t, &value, 4);
                memcpy(span + BRIDGE_SPAN_SOURCE(sys->terms) + 8u * t, &source, 8);
            }
            at += size;
            spans ++;
        }
        slot->spans = spans;
    }
    return 0;
}

/* ---- the log ring ---- */

static void drain(bridge *b, module *m)
{
    static const char outside[] = "(a log line whose text lies outside the scratch arena)";
    bridge_header *h = (bridge_header *)m->priv;
    uint32_t head = h->ring_head, written = head - m->ring_tail;
    uint32_t kept = written < m->ring_len ? written : m->ring_len, i;

    for (i = 0; i < kept; i ++) {
        bridge_log_record rec;
        bridge_record r = {0};
        memcpy(&rec, m->priv + m->ring + ((m->ring_tail + i) % m->ring_len) * sizeof rec, sizeof rec);
        r.identity = m->identity;
        r.system = rec.system < m->system_count ? m->systems[rec.system]->name : NULL;
        if (rec.text >= m->scratch && rec.len <= m->scratch_len
            && rec.text - m->scratch <= m->scratch_len - rec.len) {
            r.text = (const char *)m->priv + rec.text;
            r.len = rec.len;
        } else {
            r.text = outside;
            r.len = sizeof outside - 1;
        }
        r.when = rec.when;
        r.arg[0] = rec.arg[0];
        r.arg[1] = rec.arg[1];
        r.level = rec.level;
        b->sink(b->ud, &r);
    }
    if (written > kept) {
        say(b, m, NULL, BRIDGE_WARN, "%u log lines were dropped: its ring holds %u",
            written - kept, m->ring_len);
    }
    m->ring_tail = head;
    h->ring_tail = head;
    h->scratch_used = 0;        /* after the drain: a line's text lives in the scratch arena */
}

/* ---- the barrier - docs/bridge/run/schedule.md, *Phases* ---- */

void store_barrier(bridge *b)
{
    module *m;
    uint32_t tables;

    ecs_defer_end(b->world);            /* queued changes applied */
    sync(b);
    do {                                /* spans republished */
        tables = b->table_count;
        for (m = b->modules; m; m = m->next) {
            if (m->stopped || !publish(b, m)) continue;
            m->stopped = 1;
            say(b, m, NULL, BRIDGE_ERR, "module '%s' is stopped: what its queries match no "
                "longer fits in its spans of %u bytes", m->identity, m->spans_len);
        }
    } while (tables != b->table_count);
    b->layout_moved = 0;
    for (m = b->modules; m; m = m->next) drain(b, m);
    ecs_defer_begin(b->world);
    b->barriers ++;

    if (b->columns_out && !b->stopped_by) {
        say(b, NULL, NULL, BRIDGE_ERR, "the bridge has stopped: [budget] columns of %u bytes "
            "ran out", b->columns_len);
        b->stopped_by = copy("[budget] columns ran out", 24);
    }
}

/* ---- create and destroy ---- */

static void flecs_says(int32_t level, const char *file, int32_t line, const char *msg)
{
    (void)file; (void)line;
    if (live && level <= BRIDGE_WARN) say(live, NULL, NULL, (int8_t)level, "flecs: %s", msg);
}

static int unset(bridge *b, const char *key, uint64_t value)
{
    if (value) return 0;
    say(b, NULL, NULL, BRIDGE_ERR, "[budget] %s is unset", key);
    return 1;
}

int store_create(bridge *b, const bridge_budget *budget)
{
    uint64_t index = PAGE, arena, columns, len, reserve;
    ecs_os_api_t os;
    uint32_t i;

    if (live) {
        say(b, NULL, NULL, BRIDGE_ERR, "another bridge exists in this process, and there is "
            "room for one");
        return 1;
    }
    if (unset(b, "entities", budget->entities) || unset(b, "persistent", budget->persistent)
        || unset(b, "columns", budget->columns) || unset(b, "max_modules", budget->max_modules)) {
        return 1;
    }
    arena = (index + sizeof(bridge_entity) * budget->entities + 15u) & ~15ull;
    columns = (arena + budget->persistent + 15u) & ~15ull;
    len = (columns + budget->columns + PAGE - 1) & ~(uint64_t)(PAGE - 1);
    /* an offset is 32 bits, so the shared memory is never more than it can reach */
    if (budget->entities > UINT32_MAX || budget->persistent > UINT32_MAX
        || budget->columns > UINT32_MAX || len > (uint64_t)1 << 32) {
        say(b, NULL, NULL, BRIDGE_ERR, "[budget] cannot be satisfied: the shared memory would "
            "be %llu bytes and an offset into it is 32 bits", (unsigned long long)len);
        return 1;
    }
    reserve = (len > b->reservation ? len : b->reservation) + b->guard;
    b->shared = os_reserve((size_t)reserve, (size_t)len);
    if (!b->shared) {
        say(b, NULL, NULL, BRIDGE_ERR, "[budget] cannot be satisfied: the system would not "
            "give %llu bytes", (unsigned long long)len);
        return 1;
    }
    b->shared_len = (size_t)len;
    b->shared_reserve = (size_t)reserve;
    b->index = (bridge_entity *)(b->shared + index);
    b->entities = (uint32_t)budget->entities;
    b->arena = (uint32_t)arena;
    b->arena_len = (uint32_t)budget->persistent;
    b->columns = (uint32_t)columns;
    b->columns_len = (uint32_t)budget->columns;
    b->max_modules = budget->max_modules;
    b->is_dirty = calloc(b->entities, 1);
    b->table_count = 1;                 /* table 0 is no table */

    live = b;
    flecs_confine = (flecs_confine_t){ column_alloc, column_realloc, column_free, on_row_moved };
    ecs_os_set_api_defaults();
    os = ecs_os_get_api();
    os.log_ = flecs_says;
    ecs_os_set_api(&os);
    b->world = ecs_init();
    ecs_observer(b->world, {
        .query.terms = {{ .id = EcsWildcard }},
        .events = { EcsOnAdd, EcsOnRemove },
        .callback = on_change,
        .ctx = b
    });
    ecs_defer_begin(b->world);          /* and it stays deferred outside a barrier */
    if (b->columns_out) {
        say(b, NULL, NULL, BRIDGE_ERR, "[budget] columns of %u bytes does not hold flecs' own "
            "tables, before any module's", b->columns_len);
        return 1;
    }

    for (i = 0; i < b->backend_count; i ++) {
        open_backend *o = &b->backends[i];
        backend_err err = {0};
        o->shared = o->api.memory_adopt(o->be, b->shared, len, &err);
        if (!o->shared) {
            say(b, NULL, NULL, BRIDGE_ERR, "backend '%s' cannot adopt the shared memory: %s",
                o->name, err.text);
            return 1;
        }
    }
    return 0;
}

void store_destroy(bridge *b)
{
    uint32_t i;
    if (b->world) {
        ecs_defer_end(b->world);
        ecs_fini(b->world);
    }
    for (i = 0; i < b->backend_count; i ++) {
        open_backend *o = &b->backends[i];
        if (o->shared) o->api.memory_release(o->shared);
    }
    if (b->shared) os_release(b->shared, b->shared_reserve);
    for (i = 0; i < 32; i ++) free(b->classes[i].at);
    for (i = 0; i < b->component_count; i ++) {
        free(b->components[i]->name);
        free(b->components[i]->zero);
        free(b->components[i]);
    }
    free(b->components);
    free(b->tables);
    free(b->table_hash);
    free(b->dirty);
    free(b->is_dirty);
    if (live == b) live = NULL;
}
