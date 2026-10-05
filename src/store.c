/* The shared block, the column region carved out of it, the log ring, and the barrier. Who
 * keeps the rows is behind rows.h. docs/bridge/memory/memory.md, docs/bridge/diagnostics/log.md. */
#include "inside.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint32_t store_offset(bridge *b, const void *p)
{
    const uint8_t *c = p;
    if (!c || c < b->shared || c >= b->shared + b->shared_len) return 0;
    return (uint32_t)(c - b->shared);
}

/* ---- columns: the column region, carved ---- */

/* What storage asks for is what it gets, to sixteen bytes: a freed range is a gap, kept in
 * offset order and joined to its neighbours, and a request takes the smallest gap that holds
 * it before it takes fresh space. The gaps are kept outside the block: a module's stray write
 * must not be able to reach them. Nothing is ever taken from outside the region: a request it
 * cannot meet is refused - budgets.md, *What running out does*. */
constexpr uint32_t GRAIN = 16;

static uint32_t grain(uint32_t size)
{
    return (size + GRAIN - 1) & ~(GRAIN - 1);
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

void store_column_free(bridge *b, void *ptr, uint32_t size)
{
    give(b, (uint32_t)((uint8_t *)ptr - b->shared) - b->columns, grain(size));
}

void *store_column_regrow(bridge *b, void *ptr, uint32_t old_size, uint32_t new_size)
{
    uint8_t *base = b->shared + b->columns;
    uint32_t had = grain(old_size), want = grain(new_size), at = 0, to, i;
    if (ptr) {
        at = (uint32_t)((uint8_t *)ptr - base);
        if (want <= had) {
            if (want < had) give(b, at + want, had - want);
            return ptr;
        }
        /* where it is, when what follows it is free: a block mostly grows, and mostly it is
         * the last one */
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
    if (!take(b, want, &to)) return nullptr;
    if (ptr) {
        memcpy(base + to, ptr, had);
        give(b, at, had);
    }
    return base + to;
}

/* ---- what a module is given to hand out ---- */

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
    m->id_count += rows_reserve(b->rows, m->ids + kept, want - kept);
    /* the index is twelve bytes times the highest id, and that is what was budgeted */
    m->ids_short = m->id_count < want;
    return m->ids_short;
}

/* ---- publishing: the header, column_base, and one span per matched table ---- */

void store_header(bridge *b, module *m)
{
    auto h = (bridge_header *)m->priv;
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
    h->tables = rows_tables(b->rows);
    h->entity_index = store_offset(b, b->index);
    h->entities = b->entities;
}

/* ---- the log ring ---- */

void store_drain(bridge *b, module *m)
{
    static const char outside[] = "(a log line whose text lies outside the scratch arena)";
    auto h = (bridge_header *)m->priv;
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

    rows_apply(b->rows);                /* queued changes applied */
    for (m = b->modules; m; m = m->next) store_refill(b, m);
    do {                                /* spans republished */
        tables = rows_tables(b->rows);
        for (m = b->modules; m; m = m->next) {
            if (m->stopped) m->published = false;       /* and all of it again, if it returns */
            if (m->stopped || !rows_publish(b->rows, m)) continue;
            m->stopped = 1;
            say(b, m, nullptr, BRIDGE_ERR, "module '%s' is stopped: what its queries match no "
                "longer fits in its spans of %u bytes", m->identity, m->spans_len);
        }
    } while (tables != rows_tables(b->rows));
    rows_written(b->rows);
    b->layout_moved = 0;
    /* then the observers of what changed. What they ask for goes on their queues, and so
     * is the next barrier's - schedule.md, *An observer runs at the barrier, one hop per
     * barrier* */
    schedule_run(b, b->phase_count);
    for (i = 0; i < b->system_count; i ++) b->systems[i]->pending_count = 0;
    schedule_account(b);                /* failure.md, *The budget* */
    for (m = b->modules; m; m = m->next) store_drain(b, m);
    b->barriers ++;
}

/* ---- create and destroy ---- */

static int unset(bridge *b, const char *key, uint64_t value)
{
    if (value) return 0;
    say(b, nullptr, nullptr, BRIDGE_ERR, "[budget] %s is unset", key);
    return 1;
}

/* an offset is 32 bits: a size it cannot reach is refused by name, before any sum is made of it */
static int beyond(bridge *b, const char *key, uint64_t value)
{
    if (value <= UINT32_MAX) return 0;
    say(b, nullptr, nullptr, BRIDGE_ERR, "[budget] %s of %llu is more than an offset of 32 bits can "
        "reach", key, (unsigned long long)value);
    return 1;
}

int store_create(bridge *b, const bridge_budget *budget)
{
    uint64_t index = PAGE, arena, columns, len, reserve;
    uint32_t i;

    if (unset(b, "entities", budget->entities) || unset(b, "persistent", budget->persistent)
        || unset(b, "columns", budget->columns) || unset(b, "modules", budget->modules)
        || unset(b, "tables", budget->tables) || unset(b, "types", budget->types)
        || unset(b, "systems", budget->systems) || unset(b, "terms", budget->terms)) {
        return 1;
    }
    if (beyond(b, "entities", budget->entities) || beyond(b, "persistent", budget->persistent)
        || beyond(b, "columns", budget->columns)) {
        return 1;
    }
    /* each is under 2^32 now, so none of these sums can leave 64 bits */
    arena = (index + sizeof(bridge_entity) * budget->entities + 15u) & ~15ull;
    columns = (arena + budget->persistent + 15u) & ~15ull;
    len = (columns + budget->columns + PAGE - 1) & ~(uint64_t)(PAGE - 1);
    /* and together they are never more than an offset can reach */
    if (len > (uint64_t)1 << 32) {
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
    b->max_modules = budget->modules;
    b->tables = budget->tables;
    b->max_types = budget->types;
    b->max_systems = budget->systems;
    b->max_terms = budget->terms;
    /* a gap lies between two blocks, and a table has one block: never more gaps than tables */
    b->gap_room = b->tables + 1;
    b->gaps = malloc(b->gap_room * sizeof *b->gaps);
    b->rows = rows_create(b);
    if (!b->rows) return 1;

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
    if (b->rows) rows_destroy(b->rows);
    for (i = 0; i < b->backend_count; i ++) {
        open_backend *o = &b->backends[i];
        if (o->shared) o->api.memory_release(o->shared);
    }
    if (b->shared) os_release(b->shared, b->shared_reserve);
    free(b->gaps);
    for (i = 0; i < b->component_count; i ++) {
        schema_free(&b->components[i]->s);
        free(b->components[i]->deps);
        free(b->components[i]->name);
        free(b->components[i]->zero);
        free(b->components[i]);
    }
    for (i = 0; i < b->kept_count; i ++) free(b->kept[i].stem);
    free(b->kept);
    free(b->components);
}
