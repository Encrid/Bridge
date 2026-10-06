/* storage.h, kept by Bridge-ASM700 itself. docs/plan/STAGE10.md.
 *
 * It is made for what the bridge promises and a general store cannot assume: rows are touched
 * only at a barrier, by one thread, with every change of that barrier already queued; systems
 * are fixed at load; nobody iterates through it; and no order of rows is anybody's to keep.
 *
 *   - A barrier is worked an entity at a time. The queues are folded, in their stated order,
 *     into what each changed entity will be, and it is then placed once. No table is made that
 *     nothing stays in.
 *   - A table is one set of types, found by that set, and lives while it has rows. Its rows
 *     are one block of the column region: the entity ids, then a column for each type.
 *   - Nothing in the shared block is read back. Where an entity is, and which entity is in
 *     which row, are kept here; what the block holds of either is a copy for modules to read.
 *   - Everything it holds is sized by a budget and allocated when the bridge is created:
 *     entities, tables, columns, types, systems. Nothing is allocated after.
 *   - What a module reads is rewritten where a table was touched, and nowhere else. */
#include "inside.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

constexpr uint32_t DEAD = 0xffffffffu;          /* a place whose entity is gone */

/* what the fold has decided of an entity, for the length of one barrier */
enum folded : uint8_t {
    TOUCHED = 1, DESTROYED = 2, DROPPED = 4,
    SHORT = 8                       /* dropped for want of columns, and not of a table */
};

struct storage_type {
    uint32_t id;                    /* its bit in a set, and its column in the layout */
    uint32_t size;
    uint32_t align;                 /* where a column of it may begin: a power of two */
};

struct storage_terms {
    uint32_t          count;
    uint32_t         *type;         /* [budget] terms of each, of which `count` are its own */
    enum bridge_oper *oper;
    /* a system's: the tables it matches, and where each is in that list, less one */
    uint32_t         *list, *pos;
    uint32_t          matched;
    bool              changed;      /* which tables it matches is not what was last written */
    bool              used;         /* of the [budget] systems there are, this one is taken */
    /* an observer's */
    enum bridge_event events;
    storage_heard        heard;
    void             *ctx;
};

typedef struct { uint32_t table, row, generation; } place;      /* or `row` is the next free index */

typedef struct {
    uint32_t rows, room;
    uint32_t block, block_len;      /* its block of the column region, from the shared base */
    uint32_t incoming;              /* rows this barrier will add: room is found for them first */
    bool     live, touched;
    bool     sized;                 /* and it has been: `incoming` is how many it has room for */
    bool     named;                 /* it is the table its set is found as: lifetime.md, *What
                                     * unload leaves behind*, for when one is not */
} table;

struct storage {
    bridge    *b;
    /* entities: where each is. Never the copy in the block */
    place     *places;
    uint32_t   high, free_head;     /* indices never used begin at `high`; freed ones are a list */
    /* tables: 0 is no table */
    table     *tables;
    uint32_t   table_room, table_free, table_high;
    uint32_t  *table_next;          /* the free list of table ids */
    uint32_t  *hash;                /* a set to its table: open addressing, twice the room */
    uint32_t   hash_room;
    uint64_t  *sets;                /* [table][words]: which types a table's entities carry */
    uint32_t  *columns;             /* [table][types]: where a type's column is, 0 if absent */
    uint32_t   words, type_room;    /* how wide a set is, in 64-bit words and in types */
    uint32_t   terms_room;
    uint64_t  *who;                 /* which entity is in which row: the column region's twin */
    uint32_t  *dirty;               /* tables touched since what modules read was last written */
    uint32_t   dirty_count;
    /* types and terms: [budget] types and [budget] systems of them, and no more */
    storage_type **types;              /* by id: nullptr where none is declared */
    storage_type  *type_pool;
    storage_terms *terms_pool;
    uint32_t   *lists;              /* every system's `list` and `pos`, one after another */
    uint32_t   *term_types;         /* and every one's terms */
    enum bridge_oper *term_opers;
    storage_terms **systems, **observers;
    uint32_t   system_count, observer_count;
    /* the fold */
    uint64_t  *work;                /* [entity][words]: what a changed entity is becoming */
    uint32_t  *changed;             /* the entities this barrier changes, in the order met */
    uint32_t   changed_count;
    uint8_t   *folded;
    uint32_t  *dest;                /* the table each will be in */
    uint64_t  *at;                  /* where laid_out last put each type's column: a type each */
    uint64_t  *attached;            /* and of that, what an attach wrote a value of: kept
                                     * only while there is an observer to be told */
    uint64_t  *scratch;             /* one set */
};

/* ---- sets ---- */

static bool has(const uint64_t *set, uint32_t type) { return set[type >> 6] >> (type & 63) & 1; }
static void put(uint64_t *set, uint32_t type) { set[type >> 6] |= (uint64_t)1 << (type & 63); }
static void take(uint64_t *set, uint32_t type) { set[type >> 6] &= ~((uint64_t)1 << (type & 63)); }

static const uint64_t *set_of(const storage *r, uint32_t t) { return r->sets + (size_t)t * r->words; }
static uint32_t *columns_of(const storage *r, uint32_t t) { return r->columns + (size_t)t * r->type_room; }

static bool empty(const storage *r, const uint64_t *set)
{
    for (uint32_t w = 0; w < r->words; w ++) if (set[w]) return false;
    return true;
}

static uint32_t hashed(const storage *r, const uint64_t *set)
{
    uint64_t h = 1'469'598'103'934'665'603ull;
    for (uint32_t w = 0; w < r->words; w ++) h = (h ^ set[w]) * 1'099'511'628'211ull;
    return (uint32_t)(h ^ h >> 32) & (r->hash_room - 1);
}

/* ---- a table's block: its entity ids, then a column a type, each where its type aligns ---- */

static uint64_t padded(uint64_t bytes) { return (bytes + 7u) & ~7ull; }

/* How long a block of `room` rows of `set` is, and into r->at where each of its columns
 * begins in it: the ids, then every type in the order of its number, each at the next
 * multiple of its own alignment. The whole is a multiple of eight, for the ids of the next. */
static uint64_t laid_out(const storage *r, const uint64_t *set, uint32_t room)
{
    uint64_t at = 8ull * room;
    for (uint32_t w = 0; w < r->words; w ++) {
        for (uint64_t bits = set[w]; bits; bits &= bits - 1) {
            const storage_type *type = r->types[w * 64 + (uint32_t)__builtin_ctzll(bits)];
            at = (at + type->align - 1) & ~(uint64_t)(type->align - 1);
            r->at[type->id] = at;
            at += (uint64_t)type->size * room;
        }
    }
    return padded(at);
}

static void touch(storage *r, uint32_t t)
{
    if (r->tables[t].touched) return;
    r->tables[t].touched = true;
    r->dirty[r->dirty_count ++] = t;
}

static uint64_t *who_of(const storage *r, const table *tab)
{
    return r->who + (tab->block - r->b->columns) / 8;
}

/* room for `room` rows, where they are and in the layout that goes with it. false if the
 * column region cannot give it, and then nothing has changed. */
static bool grow(storage *r, uint32_t t, uint32_t room)
{
    table *tab = &r->tables[t];
    bridge *b = r->b;
    const uint64_t *set = set_of(r, t);
    uint32_t *columns = columns_of(r, t), was = tab->block, asked = room;
    uint64_t len, to;
    uint8_t *block;

    if (room <= tab->room) return true;
    /* half as much again as it had, so that a table that grows a row at a time is not moved
     * each time, and what was asked and no more where that is not to be had. Measured against
     * twice: as fast, with half the idle room - storage.md, *A table* */
    if (room < tab->room + tab->room / 2) room = tab->room + tab->room / 2;
    for (;; room = asked) {
        len = laid_out(r, set, room);
        block = len > INT32_MAX ? nullptr
              : block_column_regrow(b, was ? b->shared + was : nullptr, tab->block_len, (uint32_t)len);
        if (block) break;
        if (room == asked) return false;
    }
    tab->block = (uint32_t)(block - b->shared);
    if (was && tab->block != was) {
        memcpy(who_of(r, tab), r->who + (was - b->columns) / 8, 8ull * tab->rows);
    }
    tab->block_len = (uint32_t)len;
    tab->room = room;
    /* the columns are where the old room put them, in the new block: each moves out to where
     * the new room puts it, which laid_out has just said, the last first, so that none is
     * written over before it is read */
    for (uint32_t w = r->words; w --;) {
        for (uint64_t bits = set[w]; bits;) {
            uint32_t id = w * 64 + 63 - (uint32_t)__builtin_clzll(bits);
            const storage_type *type = r->types[id];
            bits &= ~((uint64_t)1 << (id & 63));
            to = r->at[id];
            if (was) memmove(block + to, block + (columns[id] - was), (size_t)type->size * tab->rows);
            columns[id] = tab->block + (uint32_t)to;
        }
    }
    touch(r, t);
    return true;
}

/* ---- tables, by their set ---- */

static uint32_t table_of(const storage *r, const uint64_t *set)
{
    uint32_t i = hashed(r, set);
    while (r->hash[i]) {
        if (!memcmp(set_of(r, r->hash[i]), set, 8u * r->words)) return r->hash[i];
        i = (i + 1) & (r->hash_room - 1);
    }
    return 0;
}

static bool matches(const storage_terms *t, const uint64_t *set)
{
    for (uint32_t i = 0; i < t->count; i ++) {
        if (t->oper[i] == BRIDGE_AND && !has(set, t->type[i])) return false;
        if (t->oper[i] == BRIDGE_NOT && has(set, t->type[i])) return false;
    }
    return true;
}

/* The table a set is found as, and no longer. What followed it in its run of the hash is put
 * back where it is still found. */
static void named(storage *r, uint32_t t)
{
    uint32_t i;
    for (i = hashed(r, set_of(r, t)); r->hash[i]; i = (i + 1) & (r->hash_room - 1)) {}
    r->hash[i] = t;
    r->tables[t].named = true;
}

static void unnamed(storage *r, uint32_t t)
{
    uint32_t i = hashed(r, set_of(r, t)), k;
    while (r->hash[i] != t) i = (i + 1) & (r->hash_room - 1);
    r->hash[i] = 0;
    for (k = (i + 1) & (r->hash_room - 1); r->hash[k]; k = (k + 1) & (r->hash_room - 1)) {
        uint32_t other = r->hash[k], home = hashed(r, set_of(r, other));
        r->hash[k] = 0;
        while (r->hash[home]) home = (home + 1) & (r->hash_room - 1);
        r->hash[home] = other;
    }
    r->tables[t].named = false;
}

/* Systems are fixed at load, so a table is matched against them when it is made, and again only
 * if its set changes: in each list it belongs in, and out of every one. */
static void listed(storage *r, uint32_t t)
{
    for (uint32_t i = 0; i < r->system_count; i ++) {
        storage_terms *s = r->systems[i];
        if (!matches(s, set_of(r, t))) continue;
        s->list[s->matched] = t;
        s->pos[t] = ++ s->matched;
        s->changed = true;
    }
}

static void unlisted(storage *r, uint32_t t)
{
    for (uint32_t i = 0; i < r->system_count; i ++) {
        storage_terms *s = r->systems[i];
        uint32_t at = s->pos[t];
        if (!at) continue;
        s->list[at - 1] = s->list[-- s->matched];
        s->pos[s->list[at - 1]] = at;
        s->pos[t] = 0;
        s->changed = true;
    }
}

/* the table of this set, made if there is none. 0 if there is no room for another. */
static uint32_t table_for(storage *r, const uint64_t *set)
{
    uint32_t t = table_of(r, set);
    if (t) return t;
    if (r->table_free) {
        t = r->table_free;
        r->table_free = r->table_next[t];
    } else if (r->table_high <= r->table_room) {
        t = r->table_high ++;
    } else {
        return 0;
    }
    /* touched or not is kept: an id given back in this barrier is already on the list */
    r->tables[t] = (table){ .live = true, .touched = r->tables[t].touched };
    memcpy(r->sets + (size_t)t * r->words, set, 8u * r->words);
    memset(columns_of(r, t), 0, 4u * r->type_room);
    named(r, t);
    listed(r, t);
    touch(r, t);
    return t;
}

/* its last row has left: its block, its id and its place in every system's list are given back */
static void table_gone(storage *r, uint32_t t)
{
    table *tab = &r->tables[t];
    if (tab->block) block_column_free(r->b, r->b->shared + tab->block, tab->block_len);
    if (tab->named) unnamed(r, t);
    unlisted(r, t);
    memset(columns_of(r, t), 0, 4u * r->type_room);
    touch(r, t);
    tab->live = false;
    tab->rows = tab->room = tab->block = tab->block_len = 0;
    r->table_next[t] = r->table_free;
    r->table_free = t;
}

/* A table stops carrying `type`, where it stands: the column goes, the ones after it close up,
 * and what the block no longer needs is given back. No row moves and nothing is asked of the
 * column region, so this cannot fail. `name` if no other table is found as what is left. */
static void stripped(storage *r, uint32_t t, const storage_type *type, bool name)
{
    table *tab = &r->tables[t];
    bridge *b = r->b;
    uint64_t *set = r->sets + (size_t)t * r->words;
    uint32_t *columns = columns_of(r, t);
    uint64_t len;

    if (tab->named) unnamed(r, t);
    unlisted(r, t);
    columns[type->id] = 0;
    take(set, type->id);
    /* what is left, laid out again: the columns before the one that went are where they were,
     * and each after it moves down to its new place, the first first */
    len = laid_out(r, set, tab->room);
    for (uint32_t w = type->id >> 6; w < r->words; w ++) {
        for (uint64_t bits = set[w]; bits; bits &= bits - 1) {
            const storage_type *after = r->types[w * 64 + (uint32_t)__builtin_ctzll(bits)];
            if (after->id < type->id) continue;
            memmove(b->shared + tab->block + r->at[after->id], b->shared + columns[after->id],
                (size_t)after->size * tab->rows);
            columns[after->id] = tab->block + (uint32_t)r->at[after->id];
        }
    }
    /* the same block, shorter: what a smaller size is answered with is where it already is */
    if (block_column_regrow(b, b->shared + tab->block, tab->block_len, (uint32_t)len)
        != b->shared + tab->block) unreachable();
    tab->block_len = (uint32_t)len;
    if (name) named(r, t);
    listed(r, t);
    touch(r, t);
}

/* ---- rows ---- */

static void placed(storage *r, uint32_t index, uint32_t t, uint32_t row)
{
    place *p = &r->places[index];
    p->table = t;
    p->row = row;
    r->b->index[index] = (bridge_entity){ t, row, p->generation };
}

/* a row at the end of `t`, which has room for it, for the entity at `index` */
static uint32_t appended(storage *r, uint32_t t, uint32_t index)
{
    table *tab = &r->tables[t];
    uint64_t id = (uint64_t)r->places[index].generation << 32 | index;
    uint32_t row = tab->rows ++;
    who_of(r, tab)[row] = id;
    memcpy(r->b->shared + tab->block + 8ull * row, &id, 8);
    touch(r, t);
    return row;
}

/* a row leaves `t`: the last one is moved into its place, and the entity that was in it is told */
static void removed(storage *r, uint32_t t, uint32_t row)
{
    table *tab = &r->tables[t];
    uint32_t last = -- tab->rows, *columns = columns_of(r, t);
    touch(r, t);
    if (row == last) return;
    for (uint32_t w = 0; w < r->words; w ++) {
        for (uint64_t bits = set_of(r, t)[w]; bits; bits &= bits - 1) {
            const storage_type *type = r->types[w * 64 + (uint32_t)__builtin_ctzll(bits)];
            uint8_t *column = r->b->shared + columns[type->id];
            memcpy(column + (size_t)row * type->size, column + (size_t)last * type->size, type->size);
        }
    }
    who_of(r, tab)[row] = who_of(r, tab)[last];
    memcpy(r->b->shared + tab->block + 8ull * row, &who_of(r, tab)[row], 8);
    placed(r, (uint32_t)who_of(r, tab)[row], t, row);
}

/* the entity at `index` goes to `to`, which has room: what it carried that `to` also holds
 * goes with it, and what `to` holds that it did not is zero */
static void moved(storage *r, uint32_t index, uint32_t to)
{
    place *p = &r->places[index];
    uint32_t from = p->table, row = p->row, at = 0;
    if (from == to) return;
    if (to) {
        uint32_t *into = columns_of(r, to), *out = from ? columns_of(r, from) : nullptr;
        at = appended(r, to, index);
        for (uint32_t w = 0; w < r->words; w ++) {
            for (uint64_t bits = set_of(r, to)[w]; bits; bits &= bits - 1) {
                const storage_type *type = r->types[w * 64 + (uint32_t)__builtin_ctzll(bits)];
                uint8_t *cell = r->b->shared + into[type->id] + (size_t)at * type->size;
                if (out && out[type->id]) {
                    memcpy(cell, r->b->shared + out[type->id] + (size_t)row * type->size, type->size);
                } else {
                    memset(cell, 0, type->size);
                }
            }
        }
    }
    if (from) removed(r, from, row);
    placed(r, index, to, at);
}

static bool alive(const storage *r, uint64_t id)
{
    uint32_t index = (uint32_t)id;
    return index && index < r->high && r->places[index].table != DEAD
        && r->places[index].generation == (uint32_t)(id >> 32);
}

/* gone: its row, and its index, which comes back under another generation */
static void killed(storage *r, uint32_t index)
{
    place *p = &r->places[index];
    if (p->table) removed(r, p->table, p->row);
    r->b->index[index] = (bridge_entity){ 0, 0, DEAD };
    p->table = DEAD;
    p->generation = p->generation + 1 >= DEAD ? 1 : p->generation + 1;
    p->row = r->free_head;
    r->free_head = index;
}

/* ---- observers: what each is owed for an entity, from what it carried and what it carries ---- */

/* data.md, *Observers*. `before` and `after` are null for an entity that carried nothing, or is
 * gone; `attached` is what an attach wrote a value of, and is still carried. */
static void told(const storage *r, uint64_t id, const uint64_t *before, const uint64_t *after,
                 const uint64_t *attached)
{
    for (uint32_t i = 0; i < r->observer_count; i ++) {
        const storage_terms *o = r->observers[i];
        /* an entity that carries nothing is in no table, and matches nothing */
        bool was = before && !empty(r, before) && matches(o, before);
        bool is = after && !empty(r, after) && matches(o, after), given = false;
        uint32_t owed;
        if (!was && !is) continue;
        for (uint32_t k = 0; attached && k < o->count; k ++) {
            given |= o->oper[k] != BRIDGE_NOT && has(attached, o->type[k]);
        }
        owed = !is ? BRIDGE_ON_LEAVE : (was ? 0 : BRIDGE_ON_ENTER) | (given ? BRIDGE_ON_ATTACH : 0);
        owed &= o->events;
        if (owed) o->heard(o->ctx, id, (enum bridge_event)owed);
    }
}

/* ---- at a barrier ---- */

static uint64_t *work_of(const storage *r, uint32_t index) { return r->work + (size_t)index * r->words; }
static uint64_t *attached_of(const storage *r, uint32_t index) { return r->attached + (size_t)index * r->words; }

static void start_from(storage *r, uint32_t index)
{
    uint32_t t = r->places[index].table;
    if (t) memcpy(work_of(r, index), set_of(r, t), 8u * r->words);
    else memset(work_of(r, index), 0, 8u * r->words);
}

/* budgets.md, *What running out does*: the entity is left as it was, and it is a failure of
 * the module that asked the last change of it, toward that module's own budget */
static void dropped(storage *r, uint32_t index, bool for_columns)
{
    r->folded[index] |= for_columns ? DROPPED | SHORT : DROPPED;
}

void storage_writes(storage *r, module *m, storage_type *type)
{
    if (!m->writable) m->writable = calloc(r->words, sizeof *m->writable);
    put(m->writable, type->id);
}

/* grants.md, *What a module may destroy*: an entity that carries something it may write, or
 * that carries nothing. Read from where the entity is, before this barrier moves it. */
static bool stake(const storage *r, const module *m, uint32_t index)
{
    uint32_t t = r->places[index].table;
    if (!t) return true;
    for (uint32_t w = 0; m->writable && w < r->words; w ++) {
        if (set_of(r, t)[w] & m->writable[w]) return true;
    }
    return false;
}

/* threads.md: the queues are applied in the order the calls that filled them ran, and not a
 * module at a time. The next run of changes: one module's, of one turn. A module's own queue
 * is in that order already, so this takes the earliest of what each has next. */
static module *next_run(bridge *b, uint32_t *from, uint32_t *to)
{
    module *m, *first = nullptr;
    for (m = b->modules; m; m = m->next) {
        if (m->mark_at == m->mark_count) continue;
        if (!first || m->marks[m->mark_at].turn < first->marks[first->mark_at].turn) first = m;
    }
    if (!first) return nullptr;
    *from = first->marks[first->mark_at ++].at;
    *to = first->mark_at < first->mark_count ? first->marks[first->mark_at].at : first->queue_len;
    return first;
}

void storage_apply(storage *r)
{
    bridge *b = r->b;
    module *m;
    uint32_t i, at, end;

    /* 1. the fold: what each changed entity will carry, and nothing moved */
    for (m = b->modules; m; m = m->next) m->mark_at = 0;
    while ((m = next_run(b, &at, &end))) {
        while (at < end) {
            change ch;
            uint32_t index;
            memcpy(&ch, m->queue + at, sizeof ch);
            at += (uint32_t)sizeof ch + ((ch.size + 7u) & ~7u);
            index = (uint32_t)ch.id;
            if (!alive(r, ch.id) || r->folded[index] & DESTROYED) continue;
            if (!(r->folded[index] & TOUCHED)) {
                r->folded[index] = TOUCHED;
                r->changed[r->changed_count ++] = index;
                start_from(r, index);
                if (r->observer_count) memset(attached_of(r, index), 0, 8u * r->words);
            }
            if (ch.op == CHANGE_DESTROY) {
                if (stake(r, m, index)) r->folded[index] |= DESTROYED;
                else m->barred ++;
            } else if (ch.op == CHANGE_ATTACH) {
                put(work_of(r, index), ch.c->type->id);
                if (r->observer_count) put(attached_of(r, index), ch.c->type->id);
            } else {
                take(work_of(r, index), ch.c->type->id);
                if (r->observer_count) take(attached_of(r, index), ch.c->type->id);
            }
        }
    }

    /* 2. a table for each, and room in it, before anything moves: what cannot be had is known
     * here, while every entity is still where it was */
    for (i = 0; i < r->changed_count; i ++) {
        uint32_t index = r->changed[i], t = 0;
        if (r->folded[index] & DESTROYED) continue;
        /* what it carries already, it carries where it is: an attach of what it has is a value
         * and no move, in whichever table of that set it happens to be */
        uint32_t from = r->places[index].table;
        if (from && !memcmp(work_of(r, index), set_of(r, from), 8u * r->words)) {
            t = from;
        } else if (!empty(r, work_of(r, index))) {
            t = table_for(r, work_of(r, index));
            if (!t) {
                dropped(r, index, false);
                continue;
            }
        }
        r->dest[index] = t;
        if (t && t != r->places[index].table) r->tables[t].incoming ++;
    }
    for (i = 0; i < r->changed_count; i ++) {
        uint32_t index = r->changed[i], t = r->dest[index];
        table *tab = &r->tables[t];
        if (r->folded[index] & (DESTROYED | DROPPED) || !t || t == r->places[index].table) continue;
        if (!tab->sized) {
            /* all of them, or as many as the column region is found to hold: by halves down
             * to a number that fits, and from there back up between it and the last that did
             * not. budgets.md, *What running out does* */
            uint32_t most = tab->incoming;
            while (tab->incoming && !grow(r, t, tab->rows + tab->incoming)) tab->incoming /= 2;
            while (most - tab->incoming > 1) {
                uint32_t between = tab->incoming + (most - tab->incoming) / 2;
                if (grow(r, t, tab->rows + between)) tab->incoming = between;
                else most = between;
            }
            tab->sized = true;
        }
        if (tab->incoming) tab->incoming --;
        else dropped(r, index, true);
    }

    /* 3. what observers are owed: for each entity, what it carried against what it will */
    for (i = 0; r->observer_count && i < r->changed_count; i ++) {
        uint32_t index = r->changed[i], from = r->places[index].table;
        if (r->folded[index] & DROPPED) continue;
        told(r, (uint64_t)r->places[index].generation << 32 | index, from ? set_of(r, from) : nullptr,
            r->folded[index] & DESTROYED ? nullptr : work_of(r, index), attached_of(r, index));
    }

    /* 4. each entity placed, once */
    for (i = 0; i < r->changed_count; i ++) {
        uint32_t index = r->changed[i];
        if (r->folded[index] & DROPPED) continue;
        if (r->folded[index] & DESTROYED) killed(r, index);
        else moved(r, index, r->dest[index]);
    }

    /* 5. the values, in the order they were asked: the last one written is the one that stays */
    for (m = b->modules; m; m = m->next) m->mark_at = 0;
    while ((m = next_run(b, &at, &end))) {
        while (at < end) {
            change ch;
            const void *value = m->queue + at + sizeof ch;
            uint32_t index, column;
            memcpy(&ch, m->queue + at, sizeof ch);
            at += (uint32_t)sizeof ch + ((ch.size + 7u) & ~7u);
            index = (uint32_t)ch.id;
            /* budgets.md, *What running out does*: a change of an entity that was left as it
             * was is a change that was not applied, and it is told to who asked it */
            if (index < r->high && r->folded[index] & DROPPED
                && r->places[index].generation == (uint32_t)(ch.id >> 32)) {
                m->left[(r->folded[index] & SHORT) != 0] ++;
            }
            if (ch.op != CHANGE_ATTACH || index >= r->high || !(r->folded[index] & TOUCHED)
                || r->folded[index] & (DESTROYED | DROPPED)
                || r->places[index].generation != (uint32_t)(ch.id >> 32)) continue;
            column = r->places[index].table ? columns_of(r, r->places[index].table)[ch.c->type->id] : 0;
            if (column) {
                memcpy(b->shared + column + (size_t)r->places[index].row * ch.c->size, value, ch.size);
            }
        }
    }
    for (m = b->modules; m; m = m->next) {
        m->changed_now = m->queue_len != 0;
        m->queue_len = m->mark_count = 0;
    }

    /* 6. a table nothing is left in is gone, and the fold is forgotten */
    for (i = 0; i < r->changed_count; i ++) {
        uint32_t index = r->changed[i], t = r->dest[index];
        if (t) r->tables[t].incoming = 0, r->tables[t].sized = false;
        r->folded[index] = 0;
        r->dest[index] = 0;
    }
    r->changed_count = 0;
    for (i = 0; i < r->dirty_count; i ++) {
        uint32_t t = r->dirty[i];
        if (r->tables[t].live && !r->tables[t].rows) table_gone(r, t);
    }
    for (m = b->modules; m; m = m->next) {
        static const char *const text[2] = {
            "module '%s': [budget] tables of %u is used up. Not applied, of the changes it "
            "asked: %u",
            "module '%s': [budget] columns of %u bytes is used up. Not applied, of the changes "
            "it asked: %u" };
        for (i = 0; i < 2; i ++) {
            if (!m->left[i]) continue;
            say(b, m, nullptr, BRIDGE_ERR, text[i], m->identity, i ? b->columns_len : r->table_room,
                m->left[i]);
            m->left[i] = 0;
            m->unplaced_now = 1;                /* failure.md, *The budget* */
        }
        if (m->barred) {
            say(b, m, nullptr, BRIDGE_ERR, "module '%s' asked to destroy entities that carry "
                "nothing it may write. Left as they were: %u", m->identity, m->barred);
            m->barred = 0;
            m->unplaced_now = 1;
        }
    }
}

uint32_t storage_reserve(storage *r, uint64_t *ids, uint32_t want)
{
    uint32_t count = 0;
    while (count < want) {
        uint32_t index = r->free_head;
        if (index) r->free_head = r->places[index].row;
        else if (r->high < r->b->entities) index = r->high ++, r->places[index].generation = 1;
        else break;
        r->places[index].table = 0;
        r->places[index].row = 0;
        r->b->index[index] = (bridge_entity){ 0, 0, r->places[index].generation };
        ids[count ++] = (uint64_t)r->places[index].generation << 32 | index;
    }
    return count;
}

uint32_t storage_tables(storage *r)
{
    return r->table_room + 1;
}

/* one span: how many rows, where their ids are, and a base for each term */
static void span(const storage *r, const storage_terms *s, uint32_t t, uint8_t *at)
{
    const table *tab = &r->tables[t];
    const uint32_t *columns = columns_of(r, t);
    uint32_t value = tab->rows;
    memset(at, 0, BRIDGE_SPAN_SIZE(s->count));
    memcpy(at, &value, 4);
    memcpy(at + 4, &tab->block, 4);
    for (uint32_t i = 0; i < s->count; i ++) {
        value = s->oper[i] == BRIDGE_NOT ? 0 : columns[s->type[i]];
        memcpy(at + BRIDGE_SPAN_BASE + 4u * i, &value, 4);
    }
}

int storage_publish(storage *r, module *m)
{
    bridge *b = r->b;
    auto h = (bridge_header *)m->priv;
    auto cell = (uint32_t *)(m->priv + m->spans + (uint32_t)sizeof(bridge_query) * m->system_count
                             + (uint32_t)sizeof(bridge_list) * m->observer_count);
    uint64_t end = (uint64_t)m->spans + m->spans_len, at;
    uint32_t tables = r->table_room + 1, q, i;
    bool all = b->layout_moved || !m->published, again = all;

    at = ((uint64_t)((uint8_t *)cell - m->priv) + 4ull * m->declared_count * tables + 7u) & ~7ull;
    if (at > end) return 1;

    /* column_base: every table when a declaration changed, and otherwise the ones touched */
    if (all) {
        block_header(b, m);
        memset(cell, 0, 4u * m->declared_count * tables);
        for (uint32_t t = 1; t < r->table_high; t ++) {
            if (!r->tables[t].live) continue;
            for (uint32_t s = 0; s < m->declared_count; s ++) {
                cell[t * m->declared_count + s] = columns_of(r, t)[m->declared[s]->type->id];
            }
        }
    } else {
        for (i = 0; i < r->dirty_count; i ++) {
            uint32_t t = r->dirty[i];
            for (uint32_t s = 0; s < m->declared_count; s ++) {
                cell[t * m->declared_count + s] = columns_of(r, t)[m->declared[s]->type->id];
            }
        }
    }

    /* spans: all of this module's again if any of its systems matches other tables than it did,
     * since they sit one after another; and otherwise the ones whose table was touched */
    for (q = 0; q < m->system_count; q ++) {
        again |= m->systems[q]->matched && m->systems[q]->matched->changed;
    }
    if (again) {
        for (q = 0; q < m->system_count; q ++) {
            storage_terms *s = m->systems[q]->matched;
            auto slot = (bridge_query *)(m->priv + m->spans) + q;
            slot->spans = 0;
            slot->at = (uint32_t)at;
            if (!s) continue;
            if (at + (uint64_t)s->matched * BRIDGE_SPAN_SIZE(s->count) > end) return 1;
            for (i = 0; i < s->matched; i ++, at += BRIDGE_SPAN_SIZE(s->count)) {
                span(r, s, s->list[i], m->priv + at);
            }
            slot->spans = s->matched;
            s->changed = false;
        }
        /* a span is a multiple of four bytes, and what follows the spans holds an id */
        at = (at + 7u) & ~7ull;
        if (at > end) return 1;
        m->spans_end = (uint32_t)at;
    } else {
        for (q = 0; q < m->system_count; q ++) {
            const storage_terms *s = m->systems[q]->matched;
            auto slot = (const bridge_query *)(m->priv + m->spans) + q;
            if (!s) continue;
            for (i = 0; i < r->dirty_count; i ++) {
                uint32_t where = s->pos[r->dirty[i]];
                if (where) span(r, s, r->dirty[i], m->priv + slot->at + (where - 1) * BRIDGE_SPAN_SIZE(s->count));
            }
        }
        at = m->spans_end;
    }

    /* and each observer's list of who triggered it, when there is one or there was */
    for (q = 0, i = 0; q < m->observer_count; q ++) i |= m->observers[q]->pending_count;
    if (i || m->had_triggers || again) {
        for (q = 0; q < m->observer_count; q ++) {
            bridge_system *sys = m->observers[q];
            auto slot = (bridge_list *)(m->priv + h->observers) + q;
            if (at + (uint64_t)sys->pending_count * sizeof(bridge_trigger) > end) return 1;
            slot->count = sys->pending_count;
            slot->at = (uint32_t)at;
            for (uint32_t k = 0; k < sys->pending_count; k ++) {
                bridge_trigger trigger = { sys->pending[k].id, sys->pending[k].events, 0 };
                memcpy(m->priv + at, &trigger, sizeof trigger);
                at += sizeof trigger;
            }
        }
        m->had_triggers = i != 0;
    }
    m->published = true;
    return 0;
}

/* Once every module has been written for: what was touched is touched no longer. */
void storage_written(storage *r)
{
    for (uint32_t i = 0; i < r->dirty_count; i ++) r->tables[r->dirty[i]].touched = false;
    r->dirty_count = 0;
}

/* ---- at load: a type, and terms ---- */

storage_type *storage_declare(storage *r, uint32_t size, uint32_t align)
{
    uint32_t id;
    for (id = 0; r->types[id]; id ++) {}        /* its declarer counted: there is one */
    r->types[id] = &r->type_pool[id];
    *r->types[id] = (storage_type){ id, size, align };
    return r->types[id];
}

void storage_undeclare(storage *r, storage_type *type)
{
    r->types[type->id] = nullptr;
}

static storage_terms *terms_of(storage *r, const storage_term *terms, uint32_t count)
{
    storage_terms *t;
    uint32_t k;
    for (k = 0; r->terms_pool[k].used; k ++) {}     /* whoever registers counted: there is one */
    t = &r->terms_pool[k];
    *t = (storage_terms){ .count = count, .used = true };
    t->type = r->term_types + (size_t)k * r->b->max_terms;
    t->oper = r->term_opers + (size_t)k * r->b->max_terms;
    t->list = r->lists + (size_t)k * 2 * (r->table_room + 2);
    t->pos = t->list + r->table_room + 2;
    memset(t->list, 0, 8u * (r->table_room + 2));
    for (uint32_t i = 0; i < count; i ++) {
        t->type[i] = terms[i].type->id;
        t->oper[i] = terms[i].oper;
    }
    return t;
}

storage_terms *storage_system(storage *r, const storage_term *terms, uint32_t count)
{
    storage_terms *s = terms_of(r, terms, count);
    for (uint32_t t = 1; t < r->table_high; t ++) {
        if (!r->tables[t].live || !matches(s, set_of(r, t))) continue;
        s->list[s->matched] = t;
        s->pos[t] = ++ s->matched;
    }
    s->changed = true;
    r->systems[r->system_count ++] = s;
    return s;
}

storage_terms *storage_observer(storage *r, const storage_term *terms, uint32_t count,
                          enum bridge_event events, storage_heard heard, void *ctx)
{
    storage_terms *o = terms_of(r, terms, count);
    o->events = events;
    o->heard = heard;
    o->ctx = ctx;
    r->observers[r->observer_count ++] = o;
    return o;
}

void storage_forget(storage *r, storage_terms *terms)
{
    uint32_t i;
    for (i = 0; i < r->system_count && r->systems[i] != terms; i ++) {}
    if (i < r->system_count) r->systems[i] = r->systems[-- r->system_count];
    for (i = 0; i < r->observer_count && r->observers[i] != terms; i ++) {}
    if (i < r->observer_count) {
        /* in order, not by the last: the order observers are told in is the order they came */
        memmove(&r->observers[i], &r->observers[i + 1], (r->observer_count - i - 1) * sizeof *r->observers);
        r->observer_count --;
    }
    terms->used = false;
}

/* ---- at unload, and for settings: one entity at a time, outside any barrier ---- */

/* the entity at `index` comes to carry `set`, told to whoever observes it. false, with a
 * record, if there is no table or no room for it: it is then as it was. */
static bool became(storage *r, uint32_t index, const uint64_t *set)
{
    uint64_t id = (uint64_t)r->places[index].generation << 32 | index;
    uint32_t from = r->places[index].table, t = 0;
    if (!empty(r, set)) {
        t = table_for(r, set);
        if (!t) {
            say(r->b, nullptr, nullptr, BRIDGE_ERR, "[budget] tables of %u is used up", r->table_room);
            return false;
        }
        if (t != from && !grow(r, t, r->tables[t].rows + 1)) {
            say(r->b, nullptr, nullptr, BRIDGE_ERR, "[budget] columns of %u bytes is used up", r->b->columns_len);
            if (!r->tables[t].rows) table_gone(r, t);
            return false;
        }
    }
    told(r, id, from ? set_of(r, from) : nullptr, set, set);
    moved(r, index, t);
    if (from && r->tables[from].live && !r->tables[from].rows) table_gone(r, from);
    return true;
}

static void destroyed(storage *r, uint32_t index)
{
    uint64_t id = (uint64_t)r->places[index].generation << 32 | index;
    uint32_t from = r->places[index].table;
    told(r, id, from ? set_of(r, from) : nullptr, nullptr, nullptr);
    killed(r, index);
    if (from && r->tables[from].live && !r->tables[from].rows) table_gone(r, from);
}

/* lifetime.md, *What unload leaves behind*. A table at a time. */
void storage_drop(storage *r, storage_type *type, enum bridge_on_unload policy)
{
    uint64_t *set = r->scratch;
    uint32_t apart = 0;
    for (uint32_t t = 1; t < r->table_high; t ++) {
        table *tab = &r->tables[t];
        uint32_t to;
        bool none;
        if (!tab->live || !has(set_of(r, t), type->id)) continue;
        memcpy(set, set_of(r, t), 8u * r->words);
        take(set, type->id);
        none = empty(r, set);
        if (policy == BRIDGE_DELETE || (policy == BRIDGE_SWEEP && none)) {
            /* from the last row, so that none is moved into the place of one already seen */
            while (tab->live) destroyed(r, (uint32_t)who_of(r, tab)[tab->rows - 1]);
            continue;
        }
        for (uint32_t row = tab->rows; row --;) {
            told(r, who_of(r, tab)[row], set_of(r, t), set, nullptr);
        }
        to = none ? 0 : table_of(r, set);
        if (none || (to && grow(r, to, r->tables[to].rows + tab->rows))) {
            /* nothing left to carry, or a table of what is left with room for them all */
            while (tab->rows) moved(r, (uint32_t)who_of(r, tab)[tab->rows - 1], to);
            table_gone(r, t);
        } else {
            /* no table of what is left, and this one becomes it; or one that has no room,
             * and until either empties there are two */
            if (to) apart += tab->rows;
            stripped(r, t, type, !to);
        }
    }
    /* lifetime.md, *What unload leaves behind*: the one case of a strip with a trace */
    for (uint32_t i = 0; apart && i < r->b->type_count; i ++) {
        if (r->b->types[i]->type != type) continue;
        say(r->b, nullptr, nullptr, BRIDGE_WARN, "'%s' is stripped, and %u rows stay in a second "
            "table of their archetype: [budget] columns of %u bytes has no room to join them to "
            "the first", r->b->types[i]->name, apart, r->b->columns_len);
    }
}

void storage_release(storage *r, const uint64_t *ids, uint32_t count)
{
    for (uint32_t i = 0; i < count; i ++) if (alive(r, ids[i])) destroyed(r, (uint32_t)ids[i]);
}

uint64_t storage_row(storage *r, storage_type *type, const void *bytes, uint32_t size)
{
    uint64_t id, *set = r->scratch;
    uint32_t index;
    if (!storage_reserve(r, &id, 1)) return 0;
    index = (uint32_t)id;
    memset(set, 0, 8u * r->words);
    put(set, type->id);
    if (!became(r, index, set)) {
        killed(r, index);
        return 0;
    }
    memcpy(r->b->shared + columns_of(r, r->places[index].table)[type->id]
           + (size_t)r->places[index].row * type->size, bytes, size);
    return id;
}

bool storage_alive(storage *r, uint64_t id)
{
    return alive(r, id);
}

bool storage_any(storage *r, storage_type *type, bool (*is)(void *ctx, const uint8_t *row), void *ctx)
{
    for (uint32_t t = 1; t < r->table_high; t ++) {
        const uint8_t *column;
        if (!r->tables[t].live || !has(set_of(r, t), type->id)) continue;
        column = r->b->shared + columns_of(r, t)[type->id];
        for (uint32_t row = 0; row < r->tables[t].rows; row ++) {
            if (is(ctx, column + (size_t)row * type->size)) return true;
        }
    }
    return false;
}

/* ---- create and destroy ---- */

storage *storage_create(bridge *b)
{
    storage *r = calloc(1, sizeof *r);
    size_t tables = (size_t)b->tables + 1;
    r->b = b;
    r->table_room = b->tables;
    r->table_high = 1;                  /* table 0 is no table */
    r->high = 1;                        /* and index 0 is no entity */
    r->words = (b->max_types + 63) / 64;
    r->type_room = r->words * 64;
    r->terms_room = b->max_systems;
    r->places = calloc(b->entities, sizeof *r->places);
    r->tables = calloc(tables, sizeof *r->tables);
    r->table_next = calloc(tables, sizeof *r->table_next);
    r->dirty = calloc(tables, sizeof *r->dirty);
    for (r->hash_room = 16; r->hash_room < tables * 2; r->hash_room *= 2) {}
    r->hash = calloc(r->hash_room, sizeof *r->hash);
    r->sets = calloc(tables * r->words, 8);
    r->columns = calloc(tables * r->type_room, 4);
    /* a slot for each eight bytes of the column region: a table's ids sit in it, so its twin
     * sits at the same place here. What is never touched is never backed by memory. */
    r->who = os_reserve(b->columns_len, b->columns_len);
    r->work = calloc((size_t)b->entities * r->words, 8);
    r->changed = calloc(b->entities, sizeof *r->changed);
    r->folded = calloc(b->entities, 1);
    r->dest = calloc(b->entities, sizeof *r->dest);
    r->attached = calloc((size_t)b->entities * r->words, 8);
    r->at = calloc(r->type_room ? r->type_room : 1, sizeof *r->at);
    r->scratch = calloc(r->words, 8);
    /* one more than there may be, so that looking for a free one always ends */
    r->types = calloc((size_t)r->type_room + 1, sizeof *r->types);
    r->type_pool = calloc(r->type_room, sizeof *r->type_pool);
    r->terms_pool = calloc((size_t)r->terms_room + 1, sizeof *r->terms_pool);
    r->lists = calloc((size_t)r->terms_room * 2 * (tables + 1), sizeof *r->lists);
    r->term_types = calloc((size_t)r->terms_room * b->max_terms, sizeof *r->term_types);
    r->term_opers = calloc((size_t)r->terms_room * b->max_terms, sizeof *r->term_opers);
    r->systems = calloc(r->terms_room, sizeof *r->systems);
    r->observers = calloc(r->terms_room, sizeof *r->observers);
    if (!r->who) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "[budget] cannot be satisfied: the operating system would not "
            "give storage %u bytes of its own for the columns it keeps", b->columns_len);
        storage_destroy(r);
        return nullptr;
    }
    return r;
}

void storage_destroy(storage *r)
{
    if (r->who) os_release(r->who, r->b->columns_len);
    free(r->types);
    free(r->type_pool);
    free(r->terms_pool);
    free(r->lists);
    free(r->term_types);
    free(r->term_opers);
    free(r->systems);
    free(r->observers);
    free(r->places);
    free(r->tables);
    free(r->table_next);
    free(r->dirty);
    free(r->hash);
    free(r->sets);
    free(r->columns);
    free(r->work);
    free(r->changed);
    free(r->folded);
    free(r->dest);
    free(r->attached);
    free(r->at);
    free(r->scratch);
    free(r);
}
