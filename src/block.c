/* The shared block, the column region carved out of it, the log ring, and the barrier. Who
 * keeps the rows is behind storage.h. docs/bridge/memory/memory.md, docs/bridge/diagnostics/log.md. */
#include "inside.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint32_t block_offset(bridge *b, const void *p)
{
    const uint8_t *c = p;
    if (!c || c < b->shared || c >= b->shared + b->shared_len) return 0;
    return (uint32_t)(c - b->shared);
}

/* ---- a region, carved ---- */

/* A freed range is a gap, kept in offset order and joined to its neighbours, and a request
 * takes the smallest gap that holds it before it takes fresh space. Nothing is ever taken from
 * outside the region: a request it cannot meet is refused - budgets.md, *What running out
 * does*. */
static void gap_drop(region *r, uint32_t i)
{
    memmove(&r->gaps[i], &r->gaps[i + 1], (r->count - i - 1) * sizeof *r->gaps);
    r->count --;
}

/* Room in the list for the gaps that `ranges` ranges handed out can leave: one after each,
 * and one before the first. false if there is not the memory. */
bool region_room(region *r, uint32_t ranges)
{
    range *gaps;
    if (ranges + 1 <= r->room) return true;
    gaps = realloc(r->gaps, ((size_t)ranges + 1) * sizeof *gaps);
    if (!gaps) return false;
    r->gaps = gaps;
    r->room = ranges + 1;
    return true;
}

void region_give(region *r, uint32_t at, uint32_t len)
{
    uint32_t i = 0;
    while (i < r->count && r->gaps[i].at < at) i ++;
    if (i && r->gaps[i - 1].at + r->gaps[i - 1].len == at) {
        r->gaps[-- i].len += len;
    } else {
        /* the column region's list is as long as there may be tables, and the arena's as long
         * as the ranges modules hold, plus one: region_room saw to it when each was taken. So
         * there is room; were there not, the gap would go unrecorded and its bytes unused */
        if (r->count == r->room) return;
        memmove(&r->gaps[i + 1], &r->gaps[i], (r->count - i) * sizeof *r->gaps);
        r->gaps[i] = (range){ at, len };
        r->count ++;
    }
    if (i + 1 < r->count && r->gaps[i].at + r->gaps[i].len == r->gaps[i + 1].at) {
        r->gaps[i].len += r->gaps[i + 1].len;
        gap_drop(r, i + 1);
    }
    /* a gap at the top is not a gap: it is space never used */
    if (r->gaps[i].at + r->gaps[i].len == r->used) {
        r->used = r->gaps[i].at;
        gap_drop(r, i);
    }
}

int region_take(region *r, uint32_t len, uint32_t *at)
{
    uint32_t i, best = UINT32_MAX;
    for (i = 0; i < r->count; i ++) {
        if (r->gaps[i].len >= len && (best == UINT32_MAX || r->gaps[i].len < r->gaps[best].len)) {
            best = i;
        }
    }
    if (best != UINT32_MAX) {
        *at = r->gaps[best].at;
        r->gaps[best].at += len;
        r->gaps[best].len -= len;
        if (!r->gaps[best].len) gap_drop(r, best);
        return 1;
    }
    if (len > r->len - r->used) return 0;
    *at = r->used;
    r->used += len;
    return 1;
}

bool region_grow(region *r, uint32_t at, uint32_t had, uint32_t want)
{
    /* mostly it is the last piece handed out, and what follows it has never been used */
    if (at + had == r->used && want - had <= r->len - r->used) {
        r->used += want - had;
        return true;
    }
    for (uint32_t i = 0; i < r->count && r->gaps[i].at <= at + had; i ++) {
        if (r->gaps[i].at != at + had || r->gaps[i].len < want - had) continue;
        r->gaps[i].at += want - had;
        r->gaps[i].len -= want - had;
        if (!r->gaps[i].len) gap_drop(r, i);
        return true;
    }
    return false;
}

uint32_t region_most(const region *r)
{
    uint32_t most = r->len - r->used;
    for (uint32_t i = 0; i < r->count; i ++) if (r->gaps[i].len > most) most = r->gaps[i].len;
    return most;
}

/* ---- columns: what storage asks for is what it gets, to sixteen bytes ---- */

/* sixteen bytes, which is where a column has to begin to be read at full speed: storage.md,
 * *A table*. A block begins with its entity ids, and they begin where a column may */
constexpr uint32_t GRAIN = 16;

static uint32_t grain(uint32_t size)
{
    return (size + GRAIN - 1) & ~(GRAIN - 1);
}

void block_column_free(bridge *b, void *ptr, uint32_t size)
{
    region_give(&b->column_space, (uint32_t)((uint8_t *)ptr - b->shared) - b->columns, grain(size));
}

void *block_column_regrow(bridge *b, void *ptr, uint32_t old_size, uint32_t new_size)
{
    uint8_t *base = b->shared + b->columns;
    uint32_t had = grain(old_size), want = grain(new_size), at = 0, to;
    if (ptr) {
        at = (uint32_t)((uint8_t *)ptr - base);
        if (want <= had) {
            if (want < had) region_give(&b->column_space, at + want, had - want);
            return ptr;
        }
        /* where it is, when what follows it is free: a block mostly grows */
        if (region_grow(&b->column_space, at, had, want)) return ptr;
    }
    if (!region_take(&b->column_space, want, &to)) return nullptr;
    if (ptr) {
        memcpy(base + to, ptr, had);
        region_give(&b->column_space, at, had);
    }
    return base + to;
}

/* ---- what a module is given to hand out ---- */

/* A module's ids for the time until the next barrier: what it did not use, topped up to what
 * its config stated. They are real entities with nothing on them, which no query matches. */
int block_refill(bridge *b, module *m)
{
    uint32_t want = m->spawns > 0 ? (uint32_t)m->spawns : 0, kept = m->id_count - m->id_next;
    if (!want) return 0;
    /* nothing used since the last barrier is nothing to move: the block is as long as its
     * `spawns`, and moving it onto itself was most of a quiet barrier's cost */
    if (m->id_next) memmove(m->ids, m->ids + m->id_next, kept * sizeof *m->ids);
    m->id_next = 0;
    m->id_count = kept;
    m->ids_short = 0;
    m->id_count += storage_reserve(b->storage, m->ids + kept, want - kept);
    /* the index is twelve bytes times the highest id, and that is what was budgeted */
    m->ids_short = m->id_count < want;
    return m->ids_short;
}

/* ---- publishing: the header, column_base, and one span per matched table ---- */

void block_header(bridge *b, module *m)
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
    h->types = m->declared_count;
    h->tables = storage_tables(b->storage);
    h->entity_index = block_offset(b, b->index);
    h->entities = b->entities;
}

/* ---- the log ring ---- */

void block_drain(bridge *b, module *m)
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
        /* a module's four names for a level and a host's are two seams' names for the same
         * numbers, each header being whole without the other: held equal here, where one
         * becomes the other */
        static_assert((int)BRIDGE_LOG_ERR == (int)BRIDGE_ERR && (int)BRIDGE_LOG_WARN == (int)BRIDGE_WARN
            && (int)BRIDGE_LOG_INFO == (int)BRIDGE_INFO && (int)BRIDGE_LOG_DEBUG == (int)BRIDGE_DEBUG);
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

void block_barrier(bridge *b)
{
    module *m;
    uint32_t tables, i;

    storage_apply(b->storage);                /* queued changes applied */
    for (m = b->modules; m; m = m->next) block_refill(b, m);
    do {                                /* spans republished */
        tables = storage_tables(b->storage);
        for (m = b->modules; m; m = m->next) {
            if (m->stopped) m->published = false;       /* and all of it again, if it returns */
            if (m->stopped || !storage_publish(b->storage, m)) continue;
            m->stopped = 1;
            say(b, m, nullptr, BRIDGE_ERR, "module '%s' is stopped: what its queries match no "
                "longer fits in its spans of %u bytes", m->identity, m->spans_len);
        }
    } while (tables != storage_tables(b->storage));
    storage_written(b->storage);
    b->layout_moved = 0;
    /* then the observers of what changed. What they ask for goes on their queues, and so
     * is the next barrier's - schedule.md, *An observer runs at the barrier, one hop per
     * barrier* */
    schedule_run(b, b->phase_count);
    b->turns = 0;
    for (i = 0; i < b->system_count; i ++) b->systems[i]->pending_count = 0;
    schedule_account(b);                /* failure.md, *The budget* */
    for (m = b->modules; m; m = m->next) block_drain(b, m);
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

int block_create(bridge *b, const bridge_budget *budget)
{
    /* query.md: offset 0 is none, so nothing is there. Eight bytes keep the index aligned */
    uint64_t index = 8, arena, columns, len, reserve;
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
    arena = (index + sizeof(bridge_entity) * budget->entities + 7u) & ~7ull;
    columns = (arena + budget->persistent + 15u) & ~15ull;      /* storage.md, *A table* */
    /* a whole number of what every backend that maps it needs - backend.md */
    len = (columns + budget->columns + b->granule - 1) / b->granule * b->granule;
    /* and together they are never more than an offset can reach */
    if (len > (uint64_t)1 << 32) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "[budget] cannot be satisfied: the shared memory would "
            "be %llu bytes and an offset into it is 32 bits", (unsigned long long)len);
        return 1;
    }
    reserve = (len > b->reservation ? len : b->reservation) + b->guard;
    b->shared = os_reserve((size_t)reserve, (size_t)len);
    if (!b->shared) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "[budget] cannot be satisfied: the operating system would not "
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
    b->column_space = (region){ .room = b->tables + 1, .len = b->columns_len };
    b->column_space.gaps = malloc(b->column_space.room * sizeof(range));
    b->arena_space = (region){ .len = b->arena_len };
    if (bridge_short(b, b->column_space.gaps, "the list of what is free in [budget] columns")) return 1;
    b->storage = storage_create(b);
    if (!b->storage) return 1;

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

void block_destroy(bridge *b)
{
    uint32_t i;
    if (b->storage) storage_destroy(b->storage);
    for (i = 0; i < b->backend_count; i ++) {
        open_backend *o = &b->backends[i];
        if (o->shared) o->api.memory_release(o->shared);
    }
    if (b->shared) os_release(b->shared, b->shared_reserve);
    free(b->column_space.gaps);
    free(b->arena_space.gaps);
    for (i = 0; i < b->type_count; i ++) {
        schema_free(&b->types[i]->s);
        free(b->types[i]->deps);
        free(b->types[i]->name);
        free(b->types[i]->zero);
        free(b->types[i]);
    }
    for (i = 0; i < b->kept_count; i ++) free(b->kept[i].stem);
    free(b->kept);
    free(b->types);
}
