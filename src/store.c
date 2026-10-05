/* Storage: the shared block, flecs over it, the entity index mirror, and the barrier.
 * docs/bridge/memory/memory.md, docs/bridge/flecs.md, docs/bridge/data/query.md and
 * reference.md, docs/bridge/diagnostics/log.md. */
#include "inside.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* flecs_confine is a global, as ecs_os_api is, because the functions that allocate a column
 * have no world in scope. So which bridge a column is for is whichever this thread is working
 * for: every way into Bridge-ASM700 says so before it reaches flecs. Two bridges in a process
 * are then two worlds that never see each other's block. */
static thread_local bridge *live;

void store_enter(bridge *b)
{
    live = b;
}

uint32_t store_offset(bridge *b, const void *p)
{
    const uint8_t *c = p;
    if (!c || c < b->shared || c >= b->shared + b->shared_len) return 0;
    return (uint32_t)(c - b->shared);
}

/* ---- columns: flecs' allocator, carving the column region ---- */

/* What flecs asks for is what it gets, to sixteen bytes: a freed range is a gap, kept in
 * offset order and joined to its neighbours, and a request takes the smallest gap that holds
 * it before it takes fresh space. The gaps are kept outside the block: a module's stray write
 * must not be able to reach them. */
#define GRAIN 16u

static uint32_t grain(ecs_size_t size)
{
    return ((uint32_t)size + GRAIN - 1) & ~(GRAIN - 1);
}

static int in_columns(bridge *b, const void *p)
{
    const uint8_t *c = p, *base = b->shared + b->columns;
    return c >= base && c < base + b->columns_len;
}

static void gap_drop(bridge *b, uint32_t i)
{
    memmove(&b->gaps[i], &b->gaps[i + 1], (b->gap_count - i - 1) * sizeof *b->gaps);
    b->gap_count --;
}

/* [at, at + len) is free again */
static void give(bridge *b, uint32_t at, uint32_t len)
{
    uint32_t i = 0;
    while (i < b->gap_count && b->gaps[i].at < at) i ++;
    if (i && b->gaps[i - 1].at + b->gaps[i - 1].len == at) {
        b->gaps[-- i].len += len;
    } else {
        if (b->gap_count == b->gap_room) {
            b->gap_room = b->gap_room ? b->gap_room * 2 : 64;
            b->gaps = realloc(b->gaps, b->gap_room * sizeof *b->gaps);
        }
        memmove(&b->gaps[i + 1], &b->gaps[i], (b->gap_count - i) * sizeof *b->gaps);
        b->gaps[i].at = at;
        b->gaps[i].len = len;
        b->gap_count ++;
    }
    if (i + 1 < b->gap_count && b->gaps[i].at + b->gaps[i].len == b->gaps[i + 1].at) {
        b->gaps[i].len += b->gaps[i + 1].len;
        gap_drop(b, i + 1);
    }
    /* a gap at the top is not a gap: it is space never used */
    if (b->gaps[i].at + b->gaps[i].len == b->columns_used) {
        b->columns_used = b->gaps[i].at;
        gap_drop(b, i);
    }
}

/* `len` bytes: from the smallest gap that holds them, or from the unused end. 0 if neither. */
static int take(bridge *b, uint32_t len, uint32_t *at)
{
    uint32_t i, best = UINT32_MAX;
    for (i = 0; i < b->gap_count; i ++) {
        if (b->gaps[i].len >= len && (best == UINT32_MAX || b->gaps[i].len < b->gaps[best].len)) {
            best = i;
        }
    }
    if (best != UINT32_MAX) {
        *at = b->gaps[best].at;
        b->gaps[best].at += len;
        b->gaps[best].len -= len;
        if (!b->gaps[best].len) gap_drop(b, best);
        return 1;
    }
    if (len > b->columns_len - b->columns_used) return 0;
    *at = b->columns_used;
    b->columns_used += len;
    return 1;
}

static void *column_alloc(ecs_size_t size)
{
    bridge *b = live;
    uint32_t at;
    if (size <= 0) return nullptr;
    b->layout_moved = 1;
    if (take(b, grain(size), &at)) return b->shared + b->columns + at;
    /* flecs cannot be unwound from the middle of a change, so it is given what it asked for
     * from outside the block, and the bridge stops when the barrier returns - budgets.md,
     * *What running out does*. No module runs again to read this. */
    b->columns_out = 1;
    return malloc((size_t)size);
}

static void column_free(void *ptr, ecs_size_t size)
{
    bridge *b = live;
    if (!ptr) return;
    b->layout_moved = 1;
    if (!in_columns(b, ptr)) {
        free(ptr);
        return;
    }
    give(b, (uint32_t)((uint8_t *)ptr - b->shared) - b->columns, grain(size));
}

static void *column_realloc(void *ptr, ecs_size_t old_size, ecs_size_t new_size)
{
    bridge *b = live;
    void *p;
    if (!ptr) return column_alloc(new_size);
    if (new_size <= 0) {
        column_free(ptr, old_size);
        return nullptr;
    }
    if (in_columns(b, ptr)) {
        uint32_t at = (uint32_t)((uint8_t *)ptr - b->shared) - b->columns;
        uint32_t had = grain(old_size), want = grain(new_size), i;
        if (want == had) return ptr;
        if (want < had) {
            give(b, at + want, had - want);
            return ptr;
        }
        /* a column mostly grows, and mostly it is the last thing allocated: grow it where
         * it is when what follows it is free */
        if (at + had == b->columns_used && want - had <= b->columns_len - b->columns_used) {
            b->columns_used += want - had;
            return ptr;
        }
        for (i = 0; i < b->gap_count && b->gaps[i].at <= at + had; i ++) {
            if (b->gaps[i].at != at + had || b->gaps[i].len < want - had) continue;
            b->gaps[i].at += want - had;
            b->gaps[i].len -= want - had;
            if (!b->gaps[i].len) gap_drop(b, i);
            return ptr;
        }
    }
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
    if (b->table_count * 2 >= b->table_hash_room || !b->table_hash) {
        uint32_t room = b->table_hash_room ? b->table_hash_room : 256, k;
        while (b->table_count * 2 >= room) room *= 2;
        free(b->table_hash);
        b->table_hash = calloc(room, sizeof *b->table_hash);
        b->table_hash_room = room;
        for (k = 1; k < b->table_count; k ++) {
            if (!b->tables[k].table) continue;      /* retired */
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
    b->layout_moved = 1;
    if (b->free_count) {
        /* an id whose table is gone, and which nothing in the index still says */
        uint32_t id = b->free_tables[-- b->free_count];
        b->tables[id].table = t;
        b->tables[id].rows = 0;
        b->table_hash[i] = id;
        return id;
    }
    if (b->table_count >= b->table_room) {
        b->table_room = b->table_room ? b->table_room * 2 : 64;
        b->tables = realloc(b->tables, b->table_room * sizeof *b->tables);
    }
    b->tables[b->table_count].table = t;
    b->tables[b->table_count].rows = 0;
    b->table_hash[i] = b->table_count;
    return b->table_count ++;
}

/* flecs says when it frees a table, from the one function every path to that goes through:
 * a component deleted, the children of something deleted, its own sweep of empty tables. The
 * id is dead until the next sync has moved every index record off it, then free. A table
 * with several ids in its type is announced once for each, so this has to be safe to repeat. */
static void on_table_gone(ecs_iter_t *it)
{
    bridge *b = it->ctx;
    uint32_t id;
    for (id = 1; id < b->table_count && b->tables[id].table != it->table; id ++) {}
    if (id >= b->table_count) return;
    b->tables[id].table = nullptr;
    b->tables[id].rows = 0;
    if (b->dead_count == b->dead_room) {
        b->dead_room = b->dead_room ? b->dead_room * 2 : 16;
        b->dead_tables = realloc(b->dead_tables, b->dead_room * sizeof *b->dead_tables);
    }
    b->dead_tables[b->dead_count ++] = id;
    free(b->table_hash);            /* rebuilt without it at the next lookup */
    b->table_hash = nullptr;
    b->layout_moved = 1;
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
    /* every entity a retired table held was named by an event and has just been placed */
    for (i = 0; i < b->dead_count; i ++) {
        if (b->free_count == b->free_room) {
            b->free_room = b->free_room ? b->free_room * 2 : 16;
            b->free_tables = realloc(b->free_tables, b->free_room * sizeof *b->free_tables);
        }
        b->free_tables[b->free_count ++] = b->dead_tables[i];
    }
    b->dead_count = 0;
}

/* ---- what modules asked for since the last barrier ---- */

/* Each module's changes, in the order it loaded and then the order it asked: the same every
 * run, whichever thread ran what. An id that died in the meantime is passed over. */
void store_apply(bridge *b, module *m)
{
    uint32_t at = 0;
    while (at < m->queue_len) {
        change ch;
        const void *value = m->queue + at + sizeof ch;
        memcpy(&ch, m->queue + at, sizeof ch);
        at += (uint32_t)sizeof ch + ((ch.size + 7u) & ~7u);
        if (!ch.id || !ecs_is_alive(b->world, ch.id)) continue;
        if (ch.op == CHANGE_DESTROY) ecs_delete(b->world, ch.id);
        else if (ch.op == CHANGE_ATTACH) ecs_set_id(b->world, ch.id, ch.c->id, ch.size, value);
        else ecs_remove_id(b->world, ch.id, ch.c->id);
        store_touch(b, ch.id);
    }
    m->queue_len = 0;
}

static void replay(bridge *b)
{
    module *m;
    for (m = b->modules; m; m = m->next) store_apply(b, m);
}

/* A module's ids for the time until the next barrier: what it did not use, topped up to what
 * its config stated. They are real entities with nothing on them, which no query matches. */
int store_refill(bridge *b, module *m)
{
    uint32_t want = m->spawns > 0 ? (uint32_t)m->spawns : 0, kept = m->id_count - m->id_next;
    if (!want) return 0;
    if (want > m->id_room) {
        m->ids = realloc(m->ids, want * sizeof *m->ids);
        m->id_room = want;
    }
    /* nothing used since the last barrier is nothing to move: the block is as long as its
     * `spawns`, and moving it onto itself was most of a quiet barrier's cost */
    if (m->id_next) memmove(m->ids, m->ids + m->id_next, kept * sizeof *m->ids);
    m->id_next = 0;
    m->id_count = kept;
    m->ids_short = 0;
    while (m->id_count < want) {
        ecs_entity_t e = ecs_new(b->world);
        /* the index is twelve bytes times the highest id, and that is what was budgeted */
        if ((uint32_t)e >= b->entities) {
            ecs_delete(b->world, e);
            m->ids_short = 1;
            break;
        }
        store_touch(b, e);
        m->ids[m->id_count ++] = e;
    }
    return m->ids_short;
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
    h->observers = m->spans + (uint32_t)sizeof(bridge_query) * m->system_count;
    h->column_base = h->observers + (uint32_t)sizeof(bridge_list) * m->observer_count;
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
                int32_t column = !table ? -1
                    : ecs_table_get_column_index(b->world, table, m->declared[s]->id);
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
                say(b, nullptr, nullptr, BRIDGE_ERR, "the entity index is out of step with flecs: "
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

    /* and each observer's list of who triggered it - abi.md, *The ABI* */
    for (q = 0; q < m->observer_count; q ++) {
        bridge_system *sys = m->observers[q];
        bridge_list *slot = (bridge_list *)(m->priv + h->observers) + q;
        if (at + (uint64_t)sys->pending_count * sizeof(bridge_trigger) > end) return 1;
        slot->count = sys->pending_count;
        slot->at = (uint32_t)at;
        for (t = 0; t < sys->pending_count; t ++) {
            bridge_trigger trigger = { sys->pending[t].id, sys->pending[t].events, 0 };
            memcpy(m->priv + at, &trigger, sizeof trigger);
            at += sizeof trigger;
        }
    }
    return 0;
}

/* ---- the log ring ---- */

void store_drain(bridge *b, module *m)
{
    static const char outside[] = "(a log line whose text lies outside the scratch arena)";
    bridge_header *h = (bridge_header *)m->priv;
    uint32_t head = h->ring_head, written = head - m->ring_tail;
    uint32_t kept = written < m->ring_len ? written : m->ring_len, i;

    for (i = 0; i < kept; i ++) {
        bridge_log_record rec;
        bridge_record r = {};
        memcpy(&rec, m->priv + m->ring + ((m->ring_tail + i) % m->ring_len) * sizeof rec, sizeof rec);
        r.identity = m->identity;
        r.system = rec.system < m->system_count ? m->systems[rec.system]->name
                 : rec.system >= 0x8000u && rec.system - 0x8000u < m->observer_count
                 ? m->observers[rec.system - 0x8000u]->name : nullptr;
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
        say(b, m, nullptr, BRIDGE_WARN, "%u log lines were dropped: its ring holds %u",
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
    uint32_t tables, i;

    replay(b);                          /* queued changes applied */
    for (m = b->modules; m; m = m->next) store_refill(b, m);
    sync(b);
    do {                                /* spans republished */
        tables = b->table_count;
        for (m = b->modules; m; m = m->next) {
            if (m->stopped || !publish(b, m)) continue;
            m->stopped = 1;
            say(b, m, nullptr, BRIDGE_ERR, "module '%s' is stopped: what its queries match no "
                "longer fits in its spans of %u bytes", m->identity, m->spans_len);
        }
    } while (tables != b->table_count);
    b->layout_moved = 0;
    /* then the observers of what changed. What they ask for goes on their queues, and so
     * is the next barrier's - schedule.md, *An observer runs at the barrier, one hop per
     * barrier* */
    schedule_run(b, b->phase_count);
    for (i = 0; i < b->system_count; i ++) b->systems[i]->pending_count = 0;
    schedule_account(b);                /* failure.md, *The budget* */
    for (m = b->modules; m; m = m->next) store_drain(b, m);
    b->barriers ++;

    if (b->columns_out && !b->stopped_by) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "the bridge has stopped: [budget] columns of %u bytes "
            "ran out", b->columns_len);
        b->stopped_by = copy("[budget] columns ran out", 24);
    }
}

/* ---- create and destroy ---- */

static void flecs_says(int32_t level, const char *file, int32_t line, const char *msg)
{
    (void)file; (void)line;
    if (live && level <= BRIDGE_WARN) say(live, nullptr, nullptr, (int8_t)level, "flecs: %s", msg);
}

static int unset(bridge *b, const char *key, uint64_t value)
{
    if (value) return 0;
    say(b, nullptr, nullptr, BRIDGE_ERR, "[budget] %s is unset", key);
    return 1;
}

int store_create(bridge *b, const bridge_budget *budget)
{
    uint64_t index = PAGE, arena, columns, len, reserve;
    ecs_os_api_t os;
    uint32_t i;

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
        say(b, nullptr, nullptr, BRIDGE_ERR, "[budget] cannot be satisfied: the shared memory would "
            "be %llu bytes and an offset into it is 32 bits", (unsigned long long)len);
        return 1;
    }
    reserve = (len > b->reservation ? len : b->reservation) + b->guard;
    b->shared = os_reserve((size_t)reserve, (size_t)len);
    if (!b->shared) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "[budget] cannot be satisfied: the system would not "
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
    /* flecs' own globals are set up by its first world, and are nobody's to race for */
    os_global_lock();
    flecs_confine = (flecs_confine_t){ column_alloc, column_realloc, column_free, on_row_moved };
    ecs_os_set_api_defaults();
    os = ecs_os_get_api();
    if (os.log_ != flecs_says) {
        os.log_ = flecs_says;
        ecs_os_set_api(&os);
    }
    b->world = ecs_init();
    os_global_unlock();
    ecs_observer(b->world, {
        .query.terms = {{ .id = EcsWildcard }},
        .events = { EcsOnAdd, EcsOnRemove },
        .callback = on_change,
        .ctx = b
    });
    /* EcsAny, not a wildcard: a wildcard is not told of a table whose type is only pairs,
     * and the tables of an enum's constants are that */
    ecs_observer(b->world, {
        .query.terms = {{ .id = EcsAny }},
        .events = { EcsOnTableDelete },
        .callback = on_table_gone,
        .ctx = b
    });
    b->names = ecs_entity(b->world, { .name = "bridge-asm700" });
    /* the one type that is given and not declared, where every namespace's lookup ends */
    b->text_type = ecs_struct(b->world, {
        .entity = ecs_entity(b->world, { .name = "bridge_text", .parent = b->names }),
        .members = {{ .name = "at", .type = ecs_id(ecs_u32_t) },
                    { .name = "len", .type = ecs_id(ecs_u32_t) }}
    });
    if (b->columns_out) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "[budget] columns of %u bytes does not hold flecs' own "
            "tables, before any module's", b->columns_len);
        return 1;
    }

    for (i = 0; i < b->backend_count; i ++) {
        open_backend *o = &b->backends[i];
        backend_err err = {};
        o->shared = o->api.memory_adopt(o->be, b->shared, len, &err);
        if (!o->shared) {
            say(b, nullptr, nullptr, BRIDGE_ERR, "backend '%s' cannot adopt the shared memory: %s",
                o->name, err.text);
            return 1;
        }
    }
    return 0;
}

void store_destroy(bridge *b)
{
    uint32_t i;
    live = b;
    if (b->world) {
        os_global_lock();
        ecs_fini(b->world);
        os_global_unlock();
    }
    for (i = 0; i < b->backend_count; i ++) {
        open_backend *o = &b->backends[i];
        if (o->shared) o->api.memory_release(o->shared);
    }
    if (b->shared) os_release(b->shared, b->shared_reserve);
    free(b->gaps);
    for (i = 0; i < b->component_count; i ++) {
        free(b->components[i]->deps);
        free(b->components[i]->name);
        free(b->components[i]->zero);
        free(b->components[i]);
    }
    for (i = 0; i < b->kept_count; i ++) free(b->kept[i].stem);
    free(b->kept);
    free(b->dead_tables);
    free(b->free_tables);
    free(b->components);
    free(b->tables);
    free(b->table_hash);
    free(b->dirty);
    free(b->is_dirty);
    live = nullptr;
}
