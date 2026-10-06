/* storage.h, kept by Bridge-ASM700 itself. docs/plan/stage10.md.
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
    SHORT = 8,                      /* dropped for want of columns, and not of a table */
    MOVES = 16                      /* it is to have a row it has not: room is found for it */
};

struct storage_type {
    uint32_t id;                    /* its bit in a set, and its column in the layout */
    uint32_t size;
    uint32_t align;                 /* where a column of it may begin: a power of two */
    enum bridge_storage kept;       /* the storage layout it is kept in */
    uint32_t slot;                  /* which sparse set it is, when it is one; or which of
                                     * the direct-indexed */
    uint32_t cells, cells_len;      /* direct-indexed: its block, the sixteen bytes that say
                                     * where its bits are and then a cell an entity. A
                                     * singleton: its block, which is its one value */
    bool     set;                   /* a singleton: it has a value */
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
    storage_heard_t        heard;
    void             *ctx;
};

typedef struct { uint32_t table, row, generation; } place_t;      /* or `row` is the next free index */

typedef struct {
    uint32_t rows, room;
    uint32_t block, block_len;      /* its block of the column region, from the shared base */
    uint32_t incoming;              /* rows this barrier will add: room is found for them first */
    bool     live, touched;
    bool     sized;                 /* and it has been: `incoming` is how many it has room for */
    bool     named;                 /* it is the table its set is found as: lifetime.md, *What
                                     * unload leaves behind*, for when one is not */
    uint32_t set;                   /* the sparse set this is the table of, plus one: 0 for
                                     * the table of an archetype */
} table_t;

struct storage {
    bridge_t    *b;
    /* entities: where each is. Never the copy in the block */
    place_t     *places;
    uint32_t   high, free_head;     /* indices never used begin at `high`; freed ones are a list */
    /* tables: 0 is no table */
    table_t     *tables;
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
    storage_type_t **types;              /* by id: nullptr where none is declared */
    storage_type_t  *type_pool;
    storage_terms_t *terms_pool;
    uint32_t   *lists;              /* every system's `list` and `pos`, one after another */
    uint32_t   *term_types;         /* and every one's terms */
    enum bridge_oper *term_opers;
    storage_terms_t **systems, **observers;
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
    /* sparse sets: [budget] sparse_sets of them - storage.md, *A sparse set*. Each is kept as
     * a table of its one type, whose rows are found by entity, here, and not by where the
     * entity is */
    uint32_t   set_room;
    storage_type_t **set_type;        /* by slot: nullptr where none is declared */
    uint32_t  *set_table;           /* by slot: its table, 0 while no entity is in it */
    uint32_t  *where;               /* [slot][entity]: its row there plus one, 0 for none */
    uint64_t  *in_table;            /* one set: the types that are kept in tables */
    uint64_t  *before, *masked;     /* two sets more */
    uint32_t   table_count;         /* how many tables of archetypes there are */
    uint32_t   tags;                /* how many declared types have no bytes */
    uint32_t   apart;               /* how many declared types are not kept in a table. While
                                     * there is none, an entity carries what its table holds
                                     * and nothing else, and a barrier looks no further */
    /* direct-indexed: [budget] direct_indexed of them - storage.md, *Direct-indexed*. A cell
     * for every entity, in a block of the column region taken at declare; and a bit for every
     * entity that says it has a value there, kept here, with a copy for modules */
    uint32_t   direct_room, bit_words;
    storage_type_t **direct_type;     /* by slot: nullptr where none is declared */
    uint64_t  *given;               /* [slot][bit_words]: which entities have one */
};

/* ---- sets ---- */

static bool has(const uint64_t *set, uint32_t type) { return set[type >> 6] >> (type & 63) & 1; }
static void put(uint64_t *set, uint32_t type) { set[type >> 6] |= (uint64_t)1 << (type & 63); }
static void take(uint64_t *set, uint32_t type) { set[type >> 6] &= ~((uint64_t)1 << (type & 63)); }

static const uint64_t *set_of(const storage_t *r, uint32_t t) { return r->sets + (size_t)t * r->words; }
static uint32_t *columns_of(const storage_t *r, uint32_t t) { return r->columns + (size_t)t * r->type_room; }

/* A set is a 64-bit word for each 64 types there may be: one, in most deployments. Copied
 * and compared a word at a time here, since memcpy and memcmp of a length the compiler cannot
 * see are calls, and these are made for every entity a barrier changes. */
static inline void set_copy(const storage_t *r, uint64_t *to, const uint64_t *from)
{
    for (uint32_t w = 0; w < r->words; w ++) to[w] = from[w];
}

static inline void set_none(const storage_t *r, uint64_t *set)
{
    for (uint32_t w = 0; w < r->words; w ++) set[w] = 0;
}

static inline bool set_same(const storage_t *r, const uint64_t *a, const uint64_t *b)
{
    for (uint32_t w = 0; w < r->words; w ++) if (a[w] != b[w]) return false;
    return true;
}

/* A value is a word or two more often than not, and a copy or a clearing of a length the
 * compiler cannot see is a call. The two commonest sizes are done here, for every row that
 * moves. */
static inline void bytes_copy(uint8_t *to, const uint8_t *from, uint32_t size)
{
    if (size == 4) memcpy(to, from, 4);
    else if (size == 8) memcpy(to, from, 8);
    else memcpy(to, from, size);
}

static inline void bytes_none(uint8_t *at, uint32_t size)
{
    if (size == 4) memset(at, 0, 4);
    else if (size == 8) memset(at, 0, 8);
    else memset(at, 0, size);
}

static uint32_t *where_of(const storage_t *r, uint32_t slot)
{
    return r->where + (size_t)slot * r->b->entities;
}

/* what a module reads of a sparse set: for each entity where its value is, from the shared
 * base, and 0 where it has none - storage.md, *A sparse set* */
static uint32_t *cells_of(const storage_t *r, uint32_t slot)
{
    return (uint32_t *)(r->b->shared + r->b->sparse) + (size_t)slot * r->b->entities;
}

static uint64_t *given_of(const storage_t *r, uint32_t slot)
{
    return r->given + (size_t)slot * r->bit_words;
}

/* what a module reads of the same: a bit an entity, where the sixteen bytes ahead of the
 * cells say */
static uint64_t *shown_of(const storage_t *r, uint32_t slot)
{
    return (uint64_t *)(r->b->shared + r->b->direct) + (size_t)slot * r->bit_words;
}

/* storage.md, *A tag*: what a module is handed for where a column is. A tag has none, and is
 * handed that it is carried; but a toggled tag has its bits ahead of where its column would
 * be, and is handed that. */
static inline uint32_t handed(const storage_t *r, uint32_t type, uint32_t column)
{
    /* while no tag is declared, a column is where it is, and no type is looked at */
    if (!r->tags || !column) return column;
    return !r->types[type]->size && r->types[type]->kept != BRIDGE_TOGGLE ? BRIDGE_CARRIED : column;
}

/* and for where the value of row `row` of a sparse set is */
static uint32_t cell_of(const storage_type_t *type, uint32_t column, uint32_t row)
{
    return type->size ? column + row * type->size : BRIDGE_CARRIED;
}

/* everything the entity at `index` carries: what its table holds, every sparse set it is in,
 * and every direct-indexed component it has been given */
[[gnu::always_inline]] static inline void carried(const storage_t *r, uint32_t index, uint64_t *set)
{
    uint32_t t = r->places[index].table;
    if (t) set_copy(r, set, set_of(r, t));
    else set_none(r, set);
    if (!r->apart) return;
    for (uint32_t slot = 0; slot < r->set_room; slot ++) {
        if (r->set_type[slot] && where_of(r, slot)[index]) put(set, r->set_type[slot]->id);
    }
    for (uint32_t slot = 0; slot < r->direct_room; slot ++) {
        if (r->direct_type[slot] && has(given_of(r, slot), index)) put(set, r->direct_type[slot]->id);
    }
    /* and index 0, which is no entity, is what a singleton that has a value is on */
    for (uint32_t id = 0; !index && id < r->type_room; id ++) {
        if (r->types[id] && r->types[id]->set) put(set, id);
    }
}

static bool empty(const storage_t *r, const uint64_t *set)
{
    for (uint32_t w = 0; w < r->words; w ++) if (set[w]) return false;
    return true;
}

static uint32_t hashed(const storage_t *r, const uint64_t *set)
{
    uint64_t h = 1'469'598'103'934'665'603ull;
    for (uint32_t w = 0; w < r->words; w ++) h = (h ^ set[w]) * 1'099'511'628'211ull;
    return (uint32_t)(h ^ h >> 32) & (r->hash_room - 1);
}

/* ---- a table's block: its entity ids, then a column a type, each at a multiple of sixteen ---- */

/* storage.md, *A table*: a column that begins at a multiple of sixteen is read by a compiler's
 * loop of 16-byte loads with none of them across two cache lines. Measured: one that begins
 * eight past it takes 8.3 us for 204,800 rows where this takes 5.6 to 6.4. */
constexpr uint64_t COLUMN = 16;

static uint64_t padded(uint64_t bytes) { return (bytes + COLUMN - 1) & ~(COLUMN - 1); }

/* storage.md, *A toggle*: a toggled column has a bit a row, a 64-bit word for each 64 rows,
 * ahead of it in the block; and between the two, sixteen bytes that say where the bits begin,
 * which is where a module that has the column finds them. */
static uint64_t bits_len(uint32_t room) { return padded(8ull * ((room + 63) / 64)); }

/* where the bits of the toggled column at `column` are, in a block of `room` rows */
static uint32_t bits_of(uint32_t column, uint32_t room)
{
    return column - (uint32_t)COLUMN - (uint32_t)bits_len(room);
}

static bool on_at(const storage_t *r, uint32_t column, uint32_t room, uint32_t row)
{
    uint64_t word;
    memcpy(&word, r->b->shared + bits_of(column, room) + 8u * (row >> 6), 8);
    return word >> (row & 63) & 1;
}

static void turned(storage_t *r, uint32_t column, uint32_t room, uint32_t row, bool on)
{
    uint8_t *at = r->b->shared + bits_of(column, room) + 8u * (row >> 6);
    uint64_t word;
    memcpy(&word, at, 8);
    word = on ? word | (uint64_t)1 << (row & 63) : word & ~((uint64_t)1 << (row & 63));
    memcpy(at, &word, 8);
}

/* How long a block of `room` rows of `set` is, and into r->at where each of its columns
 * begins in it: the ids, then every type in the order of its number, each at the next
 * multiple of sixteen. The whole is a multiple of sixteen too, for the block after it. */
static uint64_t laid_out(const storage_t *r, const uint64_t *set, uint32_t room)
{
    uint64_t at = 8ull * room;
    for (uint32_t w = 0; w < r->words; w ++) {
        for (uint64_t bits = set[w]; bits; bits &= bits - 1) {
            const storage_type_t *type = r->types[w * 64 + (uint32_t)__builtin_ctzll(bits)];
            /* a tag has no column to begin anywhere */
            if (type->size || type->kept == BRIDGE_TOGGLE) at = padded(at);
            if (type->kept == BRIDGE_TOGGLE) at += bits_len(room) + COLUMN;
            r->at[type->id] = at;
            at += (uint64_t)type->size * room;
        }
    }
    return padded(at);
}

static void touch(storage_t *r, uint32_t t)
{
    if (r->tables[t].touched) return;
    r->tables[t].touched = true;
    r->dirty[r->dirty_count ++] = t;
}

static uint64_t *who_of(const storage_t *r, const table_t *tab)
{
    return r->who + (tab->block - r->b->columns) / 8;
}

/* room for `room` rows, where they are and in the layout that goes with it. false if the
 * column region cannot give it, and then nothing has changed. */
static bool grow(storage_t *r, uint32_t t, uint32_t room)
{
    table_t *tab = &r->tables[t];
    bridge_t *b = r->b;
    const uint64_t *set = set_of(r, t);
    uint32_t *columns = columns_of(r, t), was = tab->block, asked = room, had = tab->room;
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
            const storage_type_t *type = r->types[id];
            bits &= ~((uint64_t)1 << (id & 63));
            to = r->at[id];
            if (was) memmove(block + to, block + (columns[id] - was), (size_t)type->size * tab->rows);
            if (type->kept == BRIDGE_TOGGLE) {
                /* its bits after its values, since both move out; and where they are now */
                uint32_t bits = tab->block + bits_of((uint32_t)to, room);
                if (was) {
                    memmove(b->shared + bits, block + (bits_of(columns[id], had) - was),
                        8u * ((tab->rows + 63) / 64));
                }
                memcpy(block + to - COLUMN, &bits, 4);
            }
            columns[id] = tab->block + (uint32_t)to;
        }
    }
    /* a sparse set's values have moved, and each entity's cell says where its own is */
    if (tab->set) {
        const storage_type_t *type = r->set_type[tab->set - 1];
        uint32_t *cells = cells_of(r, tab->set - 1);
        for (uint32_t row = 0; row < tab->rows; row ++) {
            cells[(uint32_t)who_of(r, tab)[row]] = cell_of(type, columns[type->id], row);
        }
    }
    touch(r, t);
    return true;
}

/* ---- tables, by their set ---- */

static uint32_t table_of(const storage_t *r, const uint64_t *set)
{
    uint32_t i = hashed(r, set);
    while (r->hash[i]) {
        if (set_same(r, set_of(r, r->hash[i]), set)) return r->hash[i];
        i = (i + 1) & (r->hash_room - 1);
    }
    return 0;
}

static bool matches(const storage_terms_t *t, const uint64_t *set)
{
    for (uint32_t i = 0; i < t->count; i ++) {
        if (t->oper[i] == BRIDGE_AND && !has(set, t->type[i])) return false;
        if (t->oper[i] == BRIDGE_NOT && has(set, t->type[i])) return false;
    }
    return true;
}

/* Whether a system's spans include table `t`: its set matches, and a sparse set's table is
 * walked only by a system that must have its type - storage.md, *What a query walks* */
static bool reads(const storage_t *r, const storage_terms_t *s, uint32_t t)
{
    if (!matches(s, set_of(r, t))) return false;
    if (!r->tables[t].set) return true;
    for (uint32_t i = 0; i < s->count; i ++) {
        if (s->oper[i] == BRIDGE_AND && s->type[i] == r->set_type[r->tables[t].set - 1]->id) return true;
    }
    return false;
}

/* The table a set is found as, and no longer. What followed it in its run of the hash is put
 * back where it is still found. */
static void named(storage_t *r, uint32_t t)
{
    uint32_t i;
    for (i = hashed(r, set_of(r, t)); r->hash[i]; i = (i + 1) & (r->hash_room - 1)) {}
    r->hash[i] = t;
    r->tables[t].named = true;
}

static void unnamed(storage_t *r, uint32_t t)
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
static void listed(storage_t *r, uint32_t t)
{
    for (uint32_t i = 0; i < r->system_count; i ++) {
        storage_terms_t *s = r->systems[i];
        if (!reads(r, s, t)) continue;
        s->list[s->matched] = t;
        s->pos[t] = ++ s->matched;
        s->changed = true;
    }
}

static void unlisted(storage_t *r, uint32_t t)
{
    for (uint32_t i = 0; i < r->system_count; i ++) {
        storage_terms_t *s = r->systems[i];
        uint32_t at = s->pos[t];
        if (!at) continue;
        s->list[at - 1] = s->list[-- s->matched];
        s->pos[s->list[at - 1]] = at;
        s->pos[t] = 0;
        s->changed = true;
    }
}

/* the table of this set, made if there is none. 0 if there is no room for another. */
static uint32_t table_for(storage_t *r, const uint64_t *set)
{
    uint32_t t = table_of(r, set), slot;
    if (t) return t;
    /* a set that holds a type kept in a sparse set is that sparse set's table: there is
     * room for one each beside [budget] tables, so it is never refused */
    for (slot = 0; slot < r->set_room; slot ++) {
        if (r->set_type[slot] && has(set, r->set_type[slot]->id)) break;
    }
    if (slot == r->set_room && r->table_count >= r->b->tables) return 0;
    if (r->table_free) {
        t = r->table_free;
        r->table_free = r->table_next[t];
    } else if (r->table_high <= r->table_room) {
        t = r->table_high ++;
    } else {
        return 0;
    }
    /* touched or not is kept: an id given back in this barrier is already on the list */
    r->tables[t] = (table_t){ .live = true, .touched = r->tables[t].touched };
    if (slot < r->set_room) {
        r->tables[t].set = slot + 1;
        r->set_table[slot] = t;
    } else {
        r->table_count ++;
    }
    memcpy(r->sets + (size_t)t * r->words, set, 8u * r->words);
    memset(columns_of(r, t), 0, 4u * r->type_room);
    named(r, t);
    listed(r, t);
    touch(r, t);
    return t;
}

/* its last row has left: its block, its id and its place in every system's list are given back */
static void table_gone(storage_t *r, uint32_t t)
{
    table_t *tab = &r->tables[t];
    if (tab->block) block_column_free(r->b, r->b->shared + tab->block, tab->block_len);
    if (tab->named) unnamed(r, t);
    unlisted(r, t);
    memset(columns_of(r, t), 0, 4u * r->type_room);
    touch(r, t);
    if (tab->set) r->set_table[tab->set - 1] = 0;
    else r->table_count --;
    tab->live = false;
    tab->rows = tab->room = tab->block = tab->block_len = tab->set = 0;
    r->table_next[t] = r->table_free;
    r->table_free = t;
}

/* A table stops carrying `type`, where it stands: the column goes, the ones after it close up,
 * and what the block no longer needs is given back. No row moves and nothing is asked of the
 * column region, so this cannot fail. `name` if no other table is found as what is left. */
static void stripped(storage_t *r, uint32_t t, const storage_type_t *type, bool name)
{
    table_t *tab = &r->tables[t];
    bridge_t *b = r->b;
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
            const storage_type_t *after = r->types[w * 64 + (uint32_t)__builtin_ctzll(bits)];
            if (after->id < type->id) continue;
            if (after->kept == BRIDGE_TOGGLE) {
                /* its bits before its values, since both move down */
                uint32_t bits = bits_of(tab->block + (uint32_t)r->at[after->id], tab->room);
                memmove(b->shared + bits, b->shared + bits_of(columns[after->id], tab->room),
                    8u * ((tab->rows + 63) / 64));
                memcpy(b->shared + tab->block + r->at[after->id] - COLUMN, &bits, 4);
            }
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

static void placed(storage_t *r, uint32_t index, uint32_t t, uint32_t row)
{
    place_t *p = &r->places[index];
    p->table = t;
    p->row = row;
    r->b->index[index] = (bridge_entity_t){ t, row, p->generation };
}

/* the entity at `index` has row `row` of a sparse set's table: kept here, and written where
 * modules read it */
static void seated(storage_t *r, const table_t *tab, uint32_t index, uint32_t row)
{
    const storage_type_t *type = r->set_type[tab->set - 1];
    where_of(r, tab->set - 1)[index] = row + 1;
    cells_of(r, tab->set - 1)[index] = cell_of(type, columns_of(r, (uint32_t)(tab - r->tables))[type->id], row);
}

/* a row at the end of `t`, which has room for it, for the entity at `index` */
static uint32_t appended(storage_t *r, uint32_t t, uint32_t index)
{
    table_t *tab = &r->tables[t];
    uint64_t id = (uint64_t)r->places[index].generation << 32 | index;
    uint32_t row = tab->rows ++;
    who_of(r, tab)[row] = id;
    memcpy(r->b->shared + tab->block + 8ull * row, &id, 8);
    touch(r, t);
    return row;
}

/* a row leaves `t`: the last one is moved into its place, and the entity that was in it is told */
[[gnu::always_inline]] static inline void removed(storage_t *r, uint32_t t, uint32_t row)
{
    table_t *tab = &r->tables[t];
    uint32_t last = -- tab->rows, *columns = columns_of(r, t);
    touch(r, t);
    if (row == last) return;
    for (uint32_t w = 0; w < r->words; w ++) {
        for (uint64_t bits = set_of(r, t)[w]; bits; bits &= bits - 1) {
            const storage_type_t *type = r->types[w * 64 + (uint32_t)__builtin_ctzll(bits)];
            uint8_t *column = r->b->shared + columns[type->id];
            bytes_copy(column + (size_t)row * type->size, column + (size_t)last * type->size, type->size);
            if (type->kept == BRIDGE_TOGGLE) {
                turned(r, columns[type->id], tab->room, row, on_at(r, columns[type->id], tab->room, last));
            }
        }
    }
    who_of(r, tab)[row] = who_of(r, tab)[last];
    memcpy(r->b->shared + tab->block + 8ull * row, &who_of(r, tab)[row], 8);
    if (tab->set) seated(r, tab, (uint32_t)who_of(r, tab)[row], row);
    else placed(r, (uint32_t)who_of(r, tab)[row], t, row);
}

/* the entity at `index` joins the sparse set of `slot`, whose table has room for it: with a
 * value of zero, until an attach's is written */
static void joined(storage_t *r, uint32_t slot, uint32_t index)
{
    const storage_type_t *type = r->set_type[slot];
    uint32_t t = r->set_table[slot], row = appended(r, t, index);
    bytes_none(r->b->shared + columns_of(r, t)[type->id] + (size_t)row * type->size, type->size);
    seated(r, &r->tables[t], index, row);
}

/* and leaves it: the set's last row takes its place */
static void left(storage_t *r, uint32_t slot, uint32_t index)
{
    uint32_t row = where_of(r, slot)[index] - 1;
    where_of(r, slot)[index] = 0;
    cells_of(r, slot)[index] = 0;
    removed(r, r->set_table[slot], row);
}

/* where the entity at `index` has its value of `type`, from the shared base: 0 if it has none */
[[gnu::always_inline]] static inline uint32_t value_of(const storage_t *r, uint32_t index, const storage_type_t *type)
{
    uint32_t t = r->places[index].table, column;
    if (type->kept == BRIDGE_SPARSE_SET) {
        uint32_t row = where_of(r, type->slot)[index];
        return row ? columns_of(r, r->set_table[type->slot])[type->id] + (row - 1) * type->size : 0;
    }
    if (type->kept == BRIDGE_DIRECT_INDEXED) {
        return has(given_of(r, type->slot), index) ? type->cells + (uint32_t)COLUMN + index * type->size : 0;
    }
    if (type->kept == BRIDGE_SINGLETON) return type->set ? type->cells : 0;
    column = t ? columns_of(r, t)[type->id] : 0;
    return column ? column + r->places[index].row * type->size : 0;
}

/* a singleton comes to have a value, of zero until one is written, or to have none: and
 * every module is written for again, since where it is reached says which */
static void single_set(storage_t *r, storage_type_t *type, bool set)
{
    type->set = set;
    memset(r->b->shared + type->cells, 0, type->size);
    r->b->layout_moved = 1;
}

/* the entity at `index` is given, or loses, the direct-indexed component of `slot`: its bit,
 * here and where modules read it, and its cell made zero either way */
static void cell_given(storage_t *r, uint32_t slot, uint32_t index, bool on)
{
    const storage_type_t *type = r->direct_type[slot];
    if (on) put(given_of(r, slot), index), put(shown_of(r, slot), index);
    else take(given_of(r, slot), index), take(shown_of(r, slot), index);
    bytes_none(r->b->shared + type->cells + COLUMN + (size_t)index * type->size, type->size);
}

/* the entity at `index` goes to `to`, which has room: what it carried that `to` also holds
 * goes with it, and what `to` holds that it did not is zero */
[[gnu::always_inline]] static inline void moved(storage_t *r, uint32_t index, uint32_t to)
{
    place_t *p = &r->places[index];
    uint32_t from = p->table, row = p->row, at = 0;
    if (from == to) return;
    if (to) {
        uint32_t *into = columns_of(r, to), *out = from ? columns_of(r, from) : nullptr;
        at = appended(r, to, index);
        for (uint32_t w = 0; w < r->words; w ++) {
            for (uint64_t bits = set_of(r, to)[w]; bits; bits &= bits - 1) {
                const storage_type_t *type = r->types[w * 64 + (uint32_t)__builtin_ctzll(bits)];
                uint8_t *cell = r->b->shared + into[type->id] + (size_t)at * type->size;
                if (out && out[type->id]) {
                    bytes_copy(cell, r->b->shared + out[type->id] + (size_t)row * type->size, type->size);
                } else {
                    bytes_none(cell, type->size);
                }
                /* a toggled component is on when it is given, and as it was when it moves */
                if (type->kept == BRIDGE_TOGGLE) {
                    turned(r, into[type->id], r->tables[to].room, at, !out || !out[type->id]
                        || on_at(r, out[type->id], r->tables[from].room, row));
                }
            }
        }
    }
    if (from) removed(r, from, row);
    placed(r, index, to, at);
}

static bool alive(const storage_t *r, uint64_t id)
{
    uint32_t index = (uint32_t)id;
    return index && index < r->high && r->places[index].table != DEAD
        && r->places[index].generation == (uint32_t)(id >> 32);
}

/* gone: its row, and its index, which comes back under another generation */
[[gnu::always_inline]] static inline void killed(storage_t *r, uint32_t index)
{
    place_t *p = &r->places[index];
    for (uint32_t slot = 0; r->apart && slot < r->set_room; slot ++) {
        if (r->set_type[slot] && where_of(r, slot)[index]) left(r, slot, index);
    }
    for (uint32_t slot = 0; r->apart && slot < r->direct_room; slot ++) {
        if (r->direct_type[slot] && has(given_of(r, slot), index)) cell_given(r, slot, index, false);
    }
    if (p->table) removed(r, p->table, p->row);
    r->b->index[index] = (bridge_entity_t){ 0, 0, DEAD };
    p->table = DEAD;
    p->generation = p->generation + 1 >= DEAD ? 1 : p->generation + 1;
    p->row = r->free_head;
    r->free_head = index;
}

/* ---- observers: what each is owed for an entity, from what it carried and what it carries ---- */

/* data.md, *Observers*. `before` and `after` are null for an entity that carried nothing, or is
 * gone; `attached` is what an attach wrote a value of, and is still carried. */
static void told(const storage_t *r, uint64_t id, const uint64_t *before, const uint64_t *after,
                 const uint64_t *attached)
{
    for (uint32_t i = 0; i < r->observer_count; i ++) {
        const storage_terms_t *o = r->observers[i];
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

static uint64_t *work_of(const storage_t *r, uint32_t index) { return r->work + (size_t)index * r->words; }
static uint64_t *attached_of(const storage_t *r, uint32_t index) { return r->attached + (size_t)index * r->words; }

static void start_from(storage_t *r, uint32_t index)
{
    carried(r, index, work_of(r, index));
}

/* budgets.md, *What running out does*: the entity is left as it was, and it is a failure of
 * the module that asked the last change of it, toward that module's own budget */
static void dropped(storage_t *r, uint32_t index, bool for_columns)
{
    r->folded[index] |= for_columns ? DROPPED | SHORT : DROPPED;
}

/* false if there was not the memory for the set: asked before a type is made, so that
 * storage_writes cannot fail */
bool storage_writable(storage_t *r, module_t *m)
{
    if (!m->writable) m->writable = calloc(r->words, sizeof *m->writable);
    return m->writable != nullptr;
}

void storage_writes([[maybe_unused]] storage_t *r, module_t *m, storage_type_t *type)
{
    put(m->writable, type->id);
}

/* grants.md, *What a module may destroy*: an entity that carries something it may write, or
 * that carries nothing. Read from where the entity is, before this barrier moves it. */
[[gnu::always_inline]] static inline bool stake(const storage_t *r, const module_t *m, uint32_t index)
{
    uint32_t t = r->places[index].table;
    bool any = t != 0;
    for (uint32_t w = 0; t && m->writable && w < r->words; w ++) {
        if (set_of(r, t)[w] & m->writable[w]) return true;
    }
    if (!r->apart) return !any;
    for (uint32_t slot = 0; slot < r->set_room; slot ++) {
        if (!r->set_type[slot] || !where_of(r, slot)[index]) continue;
        if (m->writable && has(m->writable, r->set_type[slot]->id)) return true;
        any = true;
    }
    for (uint32_t slot = 0; slot < r->direct_room; slot ++) {
        if (!r->direct_type[slot] || !has(given_of(r, slot), index)) continue;
        if (m->writable && has(m->writable, r->direct_type[slot]->id)) return true;
        any = true;
    }
    return !any;
}

/* Room for one more of the rows table `t` is to gain at this barrier. The first time it is
 * asked of a table, room is found for all of them, or for as many as the column region is
 * found to hold: by halves down to a number that fits, and from there back up between it and
 * the last that did not. budgets.md, *What running out does* */
[[gnu::always_inline]] static inline bool roomed(storage_t *r, uint32_t t)
{
    table_t *tab = &r->tables[t];
    if (!tab->sized) {
        uint32_t most = tab->incoming;
        while (tab->incoming && !grow(r, t, tab->rows + tab->incoming)) tab->incoming /= 2;
        while (most - tab->incoming > 1) {
            uint32_t between = tab->incoming + (most - tab->incoming) / 2;
            if (grow(r, t, tab->rows + between)) tab->incoming = between;
            else most = between;
        }
        tab->sized = true;
    }
    if (!tab->incoming) return false;
    tab->incoming --;
    return true;
}

/* whether the entity at `index`, as the fold has it, is to join the sparse set of `slot` */
static bool joins(const storage_t *r, uint32_t slot, uint32_t index)
{
    return r->set_type[slot] && !where_of(r, slot)[index]
        && has(r->work + (size_t)index * r->words, r->set_type[slot]->id);
}

/* A barrier's second step, and a move's: a table for each changed entity, and room in it and
 * in each sparse set it is to join, before anything moves. What cannot be had is known here,
 * while every entity is still where it was. */
[[gnu::always_inline]] static inline void reserve_all(storage_t *r)
{
    uint32_t i;
    for (i = 0; i < r->changed_count; i ++) {
        uint32_t index = r->changed[i], t = 0;
        if (r->folded[index] & DESTROYED) continue;
        /* what it carries already, it carries where it is: an attach of what it has is a value
         * and no move, in whichever table of that set it happens to be */
        uint32_t from = r->places[index].table;
        /* its table is of what it carries that is kept in tables, and no more */
        const uint64_t *want = r->work + (size_t)index * r->words;
        if (r->apart) {
            for (uint32_t w = 0; w < r->words; w ++) r->masked[w] = want[w] & r->in_table[w];
            want = r->masked;
        }
        if (from && set_same(r, want, set_of(r, from))) {
            t = from;
        } else if (!empty(r, want)) {
            t = table_for(r, want);
            if (!t) {
                dropped(r, index, false);
                continue;
            }
        }
        r->dest[index] = t;
        if (t && t != from) {
            r->tables[t].incoming ++;
            r->folded[index] |= MOVES;
        }
        /* and a row in each sparse set it is to join */
        for (uint32_t slot = 0; r->apart && slot < r->set_room; slot ++) {
            if (!joins(r, slot, index)) continue;
            if (!r->set_table[slot]) {
                set_none(r, r->scratch);
                put(r->scratch, r->set_type[slot]->id);
                if (!table_for(r, r->scratch)) unreachable();
            }
            r->tables[r->set_table[slot]].incoming ++;
            r->folded[index] |= MOVES;
        }
    }
    for (i = 0; i < r->changed_count; i ++) {
        uint32_t index = r->changed[i], t = r->dest[index];
        /* one that is to have no row it has not is asked nothing: an attach of what an
         * entity carries is most of what a barrier sees */
        if ((r->folded[index] & (DESTROYED | DROPPED | MOVES)) != MOVES) continue;
        if (t && t != r->places[index].table && !roomed(r, t)) {
            dropped(r, index, true);
            continue;
        }
        for (uint32_t slot = 0; r->apart && slot < r->set_room; slot ++) {
            if (!joins(r, slot, index) || roomed(r, r->set_table[slot])) continue;
            dropped(r, index, true);
            break;
        }
    }
}

/* the entity at `index` put where the fold has it: its table, its sparse sets, its cells */
[[gnu::always_inline]] static inline void place_one(storage_t *r, uint32_t index)
{
    if (r->folded[index] & DESTROYED) {
        killed(r, index);
        return;
    }
    moved(r, index, r->dest[index]);
    if (!r->apart) return;
    for (uint32_t slot = 0; slot < r->set_room; slot ++) {
        if (joins(r, slot, index)) joined(r, slot, index);
        else if (r->set_type[slot] && where_of(r, slot)[index]
                 && !has(r->work + (size_t)index * r->words, r->set_type[slot]->id)) left(r, slot, index);
    }
    /* a direct-indexed component has its cell already, so it cannot be short of room */
    for (uint32_t slot = 0; slot < r->direct_room; slot ++) {
        bool was, is;
        if (!r->direct_type[slot]) continue;
        was = has(given_of(r, slot), index);
        is = has(r->work + (size_t)index * r->words, r->direct_type[slot]->id);
        if (was != is) cell_given(r, slot, index, is);
    }
    for (uint32_t id = 0; !index && id < r->type_room; id ++) {
        storage_type_t *type = r->types[id];
        if (!type || type->kept != BRIDGE_SINGLETON || type->set == has(r->work, id)) continue;
        single_set(r, type, !type->set);
    }
}

/* a table nothing is left in is gone, and the fold is forgotten */
static void forget_all(storage_t *r)
{
    uint32_t i;
    for (i = 0; i < r->changed_count; i ++) {
        uint32_t index = r->changed[i], t = r->dest[index];
        if (t) r->tables[t].incoming = 0, r->tables[t].sized = false;
        r->folded[index] = 0;
        r->dest[index] = 0;
    }
    for (i = 0; i < r->set_room; i ++) {
        if (r->set_table[i]) r->tables[r->set_table[i]].incoming = 0, r->tables[r->set_table[i]].sized = false;
    }
    r->changed_count = 0;
    for (i = 0; i < r->dirty_count; i ++) {
        uint32_t t = r->dirty[i];
        if (r->tables[t].live && !r->tables[t].rows) table_gone(r, t);
    }
}

/* threads.md: the queues are applied in the order the calls that filled them ran, and not a
 * module at a time. The next run of changes: one module's, of one turn. A module's own queue
 * is in that order already, so this takes the earliest of what each has next. */
static module_t *next_run(bridge_t *b, uint32_t *from, uint32_t *to)
{
    module_t *m, *first = nullptr;
    for (m = b->modules; m; m = m->next) {
        if (m->mark_at == m->mark_count) continue;
        if (!first || m->marks[m->mark_at].turn < first->marks[first->mark_at].turn) first = m;
    }
    if (!first) return nullptr;
    *from = first->marks[first->mark_at ++].at;
    *to = first->mark_at < first->mark_count ? first->marks[first->mark_at].at : first->queue_len;
    return first;
}

void storage_apply(storage_t *r)
{
    bridge_t *b = r->b;
    module_t *m;
    uint32_t i, at, end;

    /* 1. the fold: what each changed entity will carry, and nothing moved */
    for (m = b->modules; m; m = m->next) m->mark_at = 0;
    while ((m = next_run(b, &at, &end))) {
        while (at < end) {
            change_t ch;
            uint32_t index;
            memcpy(&ch, m->queue + at, sizeof ch);
            at += (uint32_t)sizeof ch + ((ch.size + 7u) & ~7u);
            index = (uint32_t)ch.id;
            /* a singleton is given and taken with the id 0, which is no entity: it is folded
             * as the one thing index 0 carries - storage.md, *A singleton* */
            if (ch.op != CHANGE_DESTROY && ch.c->type->kept == BRIDGE_SINGLETON ? ch.id != 0
                : !alive(r, ch.id) || r->folded[index] & DESTROYED) continue;
            if (!(r->folded[index] & TOUCHED)) {
                r->folded[index] = TOUCHED;
                r->changed[r->changed_count ++] = index;
                start_from(r, index);
                if (r->observer_count) set_none(r, attached_of(r, index));
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

    reserve_all(r);

    /* 3. what observers are owed: for each entity, what it carried against what it will */
    for (i = 0; r->observer_count && i < r->changed_count; i ++) {
        uint32_t index = r->changed[i];
        if (r->folded[index] & DROPPED) continue;
        carried(r, index, r->before);
        told(r, (uint64_t)r->places[index].generation << 32 | index, r->before,
            r->folded[index] & DESTROYED ? nullptr : work_of(r, index), attached_of(r, index));
    }

    /* 4. each entity placed, once */
    for (i = 0; i < r->changed_count; i ++) {
        if (!(r->folded[r->changed[i]] & DROPPED)) place_one(r, r->changed[i]);
    }

    /* 5. the values, in the order they were asked: the last one written is the one that stays */
    for (m = b->modules; m; m = m->next) m->mark_at = 0;
    while ((m = next_run(b, &at, &end))) {
        while (at < end) {
            change_t ch;
            const void *value = m->queue + at + sizeof ch;
            uint32_t index, to;
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
            to = value_of(r, index, ch.c->type);
            if (to) bytes_copy(b->shared + to, value, ch.size);
        }
    }
    for (m = b->modules; m; m = m->next) {
        m->changed_now = m->queue_len != 0;
        m->queue_len = m->mark_count = 0;
    }

    forget_all(r);
    for (m = b->modules; m; m = m->next) {
        static const char *const text[2] = {
            "module '%s': [budget] tables of %u is used up. Not applied, of the changes it "
            "asked: %u",
            "module '%s': [budget] columns of %u bytes is used up. Not applied, of the changes "
            "it asked: %u" };
        for (i = 0; i < 2; i ++) {
            if (!m->left[i]) continue;
            say_about(b, &(say_t){ m, nullptr, BRIDGE_ERR }, text[i], m->identity, i ? b->columns_len : b->tables,
                m->left[i]);
            m->left[i] = 0;
            m->unplaced_now = 1;                /* failure.md, *The budget* */
        }
        if (m->barred) {
            say_about(b, &(say_t){ m, nullptr, BRIDGE_ERR }, "module '%s' asked to destroy entities that carry "
                "nothing it may write. Left as they were: %u", m->identity, m->barred);
            m->barred = 0;
            m->unplaced_now = 1;
        }
    }
}

uint32_t storage_reserve(storage_t *r, uint64_t *ids, uint32_t want)
{
    uint32_t count = 0;
    while (count < want) {
        uint32_t index = r->free_head;
        if (index) r->free_head = r->places[index].row;
        else if (r->high < r->b->entities) index = r->high ++, r->places[index].generation = 1;
        else break;
        r->places[index].table = 0;
        r->places[index].row = 0;
        r->b->index[index] = (bridge_entity_t){ 0, 0, r->places[index].generation };
        ids[count ++] = (uint64_t)r->places[index].generation << 32 | index;
    }
    return count;
}

uint32_t storage_tables(storage_t *r)
{
    return r->table_room + 1;
}

/* one span: how many rows, where their ids are, and a base for each term */
static void span(const storage_t *r, const storage_terms_t *s, uint32_t t, uint8_t *at)
{
    const table_t *tab = &r->tables[t];
    const uint32_t *columns = columns_of(r, t);
    uint32_t value = tab->rows;
    memset(at, 0, BRIDGE_SPAN_SIZE(s->count));
    memcpy(at, &value, 4);
    memcpy(at + 4, &tab->block, 4);
    for (uint32_t i = 0; i < s->count; i ++) {
        value = s->oper[i] == BRIDGE_NOT ? 0 : handed(r, s->type[i], columns[s->type[i]]);
        memcpy(at + BRIDGE_SPAN_BASE + 4u * i, &value, 4);
    }
}

int storage_publish(storage_t *r, module_t *m)
{
    bridge_t *b = r->b;
    auto h = (bridge_header_t *)m->priv;
    auto cell = (uint32_t *)(m->priv + m->spans + (uint32_t)sizeof(bridge_query_t) * m->system_count
                             + (uint32_t)sizeof(bridge_list_t) * m->observer_count);
    uint64_t end = (uint64_t)m->spans + m->spans_len, at;
    uint32_t tables = r->table_room + 1, q, i;
    bool all = b->layout_moved || !m->published, again = all;

    /* column_base, and after it a cell for each type it declared: where a component that is
     * not kept in a table is reached */
    auto kept = cell + (size_t)m->declared_count * tables;
    at = ((uint64_t)((uint8_t *)kept - m->priv) + 4ull * m->declared_count + 7u) & ~7ull;
    if (at > end) return 1;

    /* column_base: every table when a declaration changed, and otherwise the ones touched */
    if (all) {
        block_write_header(b, m);
        memset(cell, 0, 4u * m->declared_count * tables);
        for (uint32_t t = 1; t < r->table_high; t ++) {
            if (!r->tables[t].live) continue;
            for (uint32_t s = 0; s < m->declared_count; s ++) {
                uint32_t id = m->declared[s]->type->id;
                cell[t * m->declared_count + s] = handed(r, id, columns_of(r, t)[id]);
            }
        }
        for (uint32_t s = 0; s < m->declared_count; s ++) {
            const storage_type_t *type = m->declared[s]->type;
            kept[s] = type->kept == BRIDGE_SPARSE_SET
                    ? b->sparse + (uint32_t)(4ull * type->slot * b->entities)
                    : type->kept == BRIDGE_DIRECT_INDEXED ? type->cells + (uint32_t)COLUMN
                    : type->kept == BRIDGE_SINGLETON && type->set
                        ? type->size ? type->cells : BRIDGE_CARRIED : 0;
        }
    } else {
        for (i = 0; i < r->dirty_count; i ++) {
            uint32_t t = r->dirty[i];
            for (uint32_t s = 0; s < m->declared_count; s ++) {
                uint32_t id = m->declared[s]->type->id;
                cell[t * m->declared_count + s] = handed(r, id, columns_of(r, t)[id]);
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
            storage_terms_t *s = m->systems[q]->matched;
            auto slot = (bridge_query_t *)(m->priv + m->spans) + q;
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
            const storage_terms_t *s = m->systems[q]->matched;
            auto slot = (const bridge_query_t *)(m->priv + m->spans) + q;
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
            bridge_system_t *sys = m->observers[q];
            auto slot = (bridge_list_t *)(m->priv + h->observers) + q;
            if (at + (uint64_t)sys->pending_count * sizeof(bridge_trigger_t) > end) return 1;
            slot->count = sys->pending_count;
            slot->at = (uint32_t)at;
            for (uint32_t k = 0; k < sys->pending_count; k ++) {
                bridge_trigger_t trigger = { sys->pending[k].id, sys->pending[k].events, 0 };
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
void storage_written(storage_t *r)
{
    for (uint32_t i = 0; i < r->dirty_count; i ++) r->tables[r->dirty[i]].touched = false;
    r->dirty_count = 0;
}

/* ---- at load: a type, and terms ---- */

uint32_t storage_sets_free(const storage_t *r)
{
    uint32_t n = 0;
    for (uint32_t slot = 0; slot < r->set_room; slot ++) n += !r->set_type[slot];
    return n;
}

uint32_t storage_direct_free(const storage_t *r)
{
    uint32_t n = 0;
    for (uint32_t slot = 0; slot < r->direct_room; slot ++) n += !r->direct_type[slot];
    return n;
}

storage_type_t *storage_declare(storage_t *r, uint32_t size, uint32_t align, enum bridge_storage kept)
{
    uint32_t id, slot = 0, cells = 0, len = 0;
    for (id = 0; r->types[id]; id ++) {}        /* its declarer counted: there is one */
    /* and asked storage_sets_free or storage_direct_free first, so there is one of these too */
    for (; kept == BRIDGE_SPARSE_SET && r->set_type[slot]; slot ++) {}
    for (; kept == BRIDGE_DIRECT_INDEXED && r->direct_type[slot]; slot ++) {}
    if (kept == BRIDGE_DIRECT_INDEXED) {
        /* a cell for every entity there may be, behind the sixteen bytes that say where its
         * bits are: the one thing a declare asks of [budget] columns */
        uint64_t want = padded(COLUMN + (uint64_t)size * r->b->entities);
        uint32_t bits = r->b->direct + (uint32_t)(8ull * slot * r->bit_words);
        uint8_t *block = want > INT32_MAX ? nullptr : block_column_regrow(r->b, nullptr, 0, (uint32_t)want);
        if (!block) return nullptr;
        memset(block, 0, (size_t)want);
        memcpy(block, &bits, 4);
        cells = (uint32_t)(block - r->b->shared);
        len = (uint32_t)want;
        memset(given_of(r, slot), 0, 8u * r->bit_words);
        memset(shown_of(r, slot), 0, 8u * r->bit_words);
    }
    if (kept == BRIDGE_SINGLETON) {
        /* its one value, in a block of its own */
        uint64_t want = padded(size ? size : 1);
        uint8_t *block = want > INT32_MAX ? nullptr : block_column_regrow(r->b, nullptr, 0, (uint32_t)want);
        if (!block) return nullptr;
        memset(block, 0, (size_t)want);
        cells = (uint32_t)(block - r->b->shared);
        len = (uint32_t)want;
    }
    r->types[id] = &r->type_pool[id];
    *r->types[id] = (storage_type_t){ id, size, align, kept, slot, cells, len };
    if (kept == BRIDGE_SPARSE_SET) r->set_type[slot] = r->types[id];
    else if (kept == BRIDGE_DIRECT_INDEXED) r->direct_type[slot] = r->types[id];
    else if (kept != BRIDGE_SINGLETON) put(r->in_table, id);
    r->apart += kept != BRIDGE_TABLE && kept != BRIDGE_TOGGLE;
    r->tags += !size;
    return r->types[id];
}

void storage_undeclare(storage_t *r, storage_type_t *type)
{
    r->apart -= type->kept != BRIDGE_TABLE && type->kept != BRIDGE_TOGGLE;
    r->tags -= !type->size;
    if (type->kept == BRIDGE_SPARSE_SET) {
        r->set_type[type->slot] = nullptr;
    } else if (type->kept == BRIDGE_DIRECT_INDEXED) {
        block_column_free(r->b, r->b->shared + type->cells, type->cells_len);
        r->direct_type[type->slot] = nullptr;
    } else if (type->kept == BRIDGE_SINGLETON) {
        block_column_free(r->b, r->b->shared + type->cells, type->cells_len);
        type->set = false;
    } else {
        take(r->in_table, type->id);
    }
    r->types[type->id] = nullptr;
}

static storage_terms_t *terms_of(storage_t *r, const storage_term_t *terms, uint32_t count)
{
    storage_terms_t *t;
    uint32_t k;
    for (k = 0; r->terms_pool[k].used; k ++) {}     /* whoever registers counted: there is one */
    t = &r->terms_pool[k];
    *t = (storage_terms_t){ .count = count, .used = true };
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

storage_terms_t *storage_system(storage_t *r, const storage_term_t *terms, uint32_t count)
{
    storage_terms_t *s = terms_of(r, terms, count);
    for (uint32_t t = 1; t < r->table_high; t ++) {
        if (!r->tables[t].live || !reads(r, s, t)) continue;
        s->list[s->matched] = t;
        s->pos[t] = ++ s->matched;
    }
    s->changed = true;
    r->systems[r->system_count ++] = s;
    return s;
}

storage_terms_t *storage_observer(storage_t *r, const storage_term_t *terms, uint32_t count,
                          enum bridge_event events, storage_heard_t heard, void *ctx)
{
    storage_terms_t *o = terms_of(r, terms, count);
    o->events = events;
    o->heard = heard;
    o->ctx = ctx;
    r->observers[r->observer_count ++] = o;
    return o;
}

void storage_forget(storage_t *r, storage_terms_t *terms)
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

static void destroyed(storage_t *r, uint32_t index)
{
    uint64_t id = (uint64_t)r->places[index].generation << 32 | index;
    uint32_t from = r->places[index].table;
    carried(r, index, r->before);
    told(r, id, r->before, nullptr, nullptr);
    killed(r, index);
    if (from && r->tables[from].live && !r->tables[from].rows) table_gone(r, from);
}

/* a sparse set nothing is left in gives its table back, as a barrier's end does */
static void sets_swept(storage_t *r)
{
    for (uint32_t slot = 0; slot < r->set_room; slot ++) {
        if (r->set_table[slot] && !r->tables[r->set_table[slot]].rows) table_gone(r, r->set_table[slot]);
    }
}

/* whether the entity at `index` carries something that is not kept in a table */
static bool in_a_set(const storage_t *r, uint32_t index)
{
    for (uint32_t slot = 0; slot < r->set_room; slot ++) {
        if (r->set_type[slot] && where_of(r, slot)[index]) return true;
    }
    for (uint32_t slot = 0; slot < r->direct_room; slot ++) {
        if (r->direct_type[slot] && has(given_of(r, slot), index)) return true;
    }
    return false;
}

/* storage_drop, of a direct-indexed component: every entity that has been given one */
static void cells_dropped(storage_t *r, const storage_type_t *type, enum bridge_on_unload policy)
{
    for (uint32_t index = 1; index < r->high; index ++) {
        uint64_t id = (uint64_t)r->places[index].generation << 32 | index;
        if (!has(given_of(r, type->slot), index)) continue;
        carried(r, index, r->before);
        memcpy(r->scratch, r->before, 8u * r->words);
        take(r->scratch, type->id);
        if (policy == BRIDGE_DELETE || (policy == BRIDGE_SWEEP && empty(r, r->scratch))) {
            destroyed(r, index);
        } else {
            told(r, id, r->before, r->scratch, nullptr);
            cell_given(r, type->slot, index, false);
        }
    }
    sets_swept(r);
}

/* storage_drop, of a component kept in a sparse set: an entity at a time, from the last */
static void set_dropped(storage_t *r, const storage_type_t *type, enum bridge_on_unload policy)
{
    uint32_t t;
    while ((t = r->set_table[type->slot]) && r->tables[t].rows) {
        uint64_t id = who_of(r, &r->tables[t])[r->tables[t].rows - 1];
        carried(r, (uint32_t)id, r->before);
        memcpy(r->scratch, r->before, 8u * r->words);
        take(r->scratch, type->id);
        if (policy == BRIDGE_DELETE || (policy == BRIDGE_SWEEP && empty(r, r->scratch))) {
            destroyed(r, (uint32_t)id);
        } else {
            told(r, id, r->before, r->scratch, nullptr);
            left(r, type->slot, (uint32_t)id);
        }
    }
    sets_swept(r);
}

/* lifetime.md, *What unload leaves behind*. A table at a time. */
void storage_drop(storage_t *r, storage_type_t *type, enum bridge_on_unload policy)
{
    uint64_t *set = r->scratch;
    uint32_t apart = 0;
    if (type->kept == BRIDGE_SPARSE_SET) {
        set_dropped(r, type, policy);
        return;
    }
    if (type->kept == BRIDGE_DIRECT_INDEXED) {
        cells_dropped(r, type, policy);
        return;
    }
    if (type->kept == BRIDGE_SINGLETON) {
        /* it is on no entity, so the three policies say one thing: it has no value */
        if (!type->set) return;
        carried(r, 0, r->before);
        memcpy(set, r->before, 8u * r->words);
        take(set, type->id);
        told(r, 0, r->before, set, nullptr);
        single_set(r, type, false);
        return;
    }
    for (uint32_t t = 1; t < r->table_high; t ++) {
        table_t *tab = &r->tables[t];
        uint32_t to;
        bool none;
        if (!tab->live || !has(set_of(r, t), type->id)) continue;
        memcpy(set, set_of(r, t), 8u * r->words);
        take(set, type->id);
        none = empty(r, set);
        if (policy == BRIDGE_DELETE || (policy == BRIDGE_SWEEP && none)) {
            /* from the last row, so that none is moved into the place of one already seen.
             * An entity that is in a sparse set carries something still, and is not swept */
            while (tab->live) {
                uint64_t id = who_of(r, tab)[tab->rows - 1];
                if (policy == BRIDGE_DELETE || !in_a_set(r, (uint32_t)id)) {
                    destroyed(r, (uint32_t)id);
                    continue;
                }
                carried(r, (uint32_t)id, r->before);
                memcpy(r->masked, r->before, 8u * r->words);
                take(r->masked, type->id);
                told(r, id, r->before, r->masked, nullptr);
                moved(r, (uint32_t)id, 0);
                if (!tab->rows) table_gone(r, t);
            }
            continue;
        }
        for (uint32_t row = tab->rows; row --;) {
            carried(r, (uint32_t)who_of(r, tab)[row], r->before);
            memcpy(r->masked, r->before, 8u * r->words);
            take(r->masked, type->id);
            told(r, who_of(r, tab)[row], r->before, r->masked, nullptr);
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
        say_about(r->b, &(say_t){ nullptr, nullptr, BRIDGE_WARN }, "'%s' is stripped, and %u rows stay in a second "
            "table of their archetype: [budget] columns of %u bytes has no room to join them to "
            "the first", r->b->types[i]->name, apart, r->b->columns_len);
    }
}

/* ---- at a reload ---- */

/* `visit` for every entity that carries `type`, whichever way it is kept */
static void each_carrier(storage_t *r, const storage_type_t *type, void (*visit)(storage_t *r, uint32_t index, void *ctx),
                         void *ctx)
{
    if (type->kept == BRIDGE_SINGLETON) {
        if (type->set) visit(r, 0, ctx);
    } else if (type->kept == BRIDGE_DIRECT_INDEXED) {
        for (uint32_t index = 1; index < r->high; index ++) {
            if (has(given_of(r, type->slot), index)) visit(r, index, ctx);
        }
    } else {
        for (uint32_t t = 1; t < r->table_high; t ++) {
            const table_t *tab = &r->tables[t];
            if (!tab->live || !has(set_of(r, t), type->id)) continue;
            for (uint32_t row = 0; row < tab->rows; row ++) visit(r, (uint32_t)who_of(r, tab)[row], ctx);
        }
    }
}

static void counted_one([[maybe_unused]] storage_t *r, [[maybe_unused]] uint32_t index, void *ctx)
{
    ++ *(uint32_t *)ctx;
}

uint32_t storage_count(storage_t *r, const storage_type_t *type)
{
    uint32_t n = 0;
    each_carrier(r, type, counted_one, &n);
    return n;
}

uint32_t storage_off(storage_t *r, const storage_type_t *type)
{
    uint32_t n = 0;
    for (uint32_t t = 1; type->kept == BRIDGE_TOGGLE && t < r->table_high; t ++) {
        const table_t *tab = &r->tables[t];
        if (!tab->live || !has(set_of(r, t), type->id)) continue;
        for (uint32_t row = 0; row < tab->rows; row ++) {
            n += !on_at(r, columns_of(r, t)[type->id], tab->room, row);
        }
    }
    return n;
}

/* the entity at `index` is among those this move changes: folded from what it carries */
static void folded_in(storage_t *r, uint32_t index)
{
    if (r->folded[index] & TOUCHED) return;
    r->folded[index] = TOUCHED;
    r->changed[r->changed_count ++] = index;
    carried(r, index, r->work + (size_t)index * r->words);
}

static void retyped(storage_t *r, uint32_t index, void *ctx)
{
    const storage_move_t *move = ctx;
    folded_in(r, index);
    take(r->work + (size_t)index * r->words, move->from->id);
    /* a singleton is on no entity: what the one that carried it had becomes index 0's */
    if (move->to->kept == BRIDGE_SINGLETON) {
        folded_in(r, 0);
        put(r->work, move->to->id);
    } else {
        put(r->work + (size_t)index * r->words, move->to->id);
    }
}

bool storage_convert(storage_t *r, const storage_move_t *moves, uint32_t count)
{
    uint32_t total = 0, i, k;
    uint8_t *was;
    bool short_of = false;
    for (k = 0; k < count; k ++) total += moves[k].from->size;
    was = malloc(total ? total : 1);
    if (!was) return false;

    /* what each entity that carries one of them will carry, and nothing moved */
    for (k = 0; k < count; k ++) {
        /* index 0 first, so that a singleton has its place before the value that goes to it */
        if (moves[k].to->kept == BRIDGE_SINGLETON && storage_count(r, moves[k].from)) folded_in(r, 0);
    }
    for (k = 0; k < count; k ++) each_carrier(r, moves[k].from, retyped, (void *)(uintptr_t)&moves[k]);
    reserve_all(r);
    for (i = 0; i < r->changed_count; i ++) short_of |= (r->folded[r->changed[i]] & DROPPED) != 0;
    if (short_of) {
        forget_all(r);
        free(was);
        return false;
    }

    /* each placed once, its values taken before it leaves where they were */
    for (i = 0; i < r->changed_count; i ++) {
        uint32_t index = r->changed[i], at = 0, had[64] = {};
        for (k = 0; k < count && k < 64; k ++) {
            had[k] = value_of(r, index, moves[k].from);
            if (had[k]) memcpy(was + at, r->b->shared + had[k], moves[k].from->size);
            at += moves[k].from->size;
        }
        place_one(r, index);
        for (k = 0, at = 0; k < count && k < 64; at += moves[k].from->size, k ++) {
            uint32_t to;
            if (!had[k]) continue;
            to = value_of(r, moves[k].to->kept == BRIDGE_SINGLETON ? 0 : index, moves[k].to);
            for (uint32_t run = 0; run < moves[k].run_count; run ++) {
                memcpy(r->b->shared + to + moves[k].runs[run].to, was + at + moves[k].runs[run].from,
                    moves[k].runs[run].len);
            }
        }
    }
    forget_all(r);
    free(was);
    return true;
}

void storage_release(storage_t *r, const uint64_t *ids, uint32_t count)
{
    for (uint32_t i = 0; i < count; i ++) if (alive(r, ids[i])) destroyed(r, (uint32_t)ids[i]);
    sets_swept(r);
}

void storage_single(storage_t *r, storage_type_t *type, const void *bytes, uint32_t size)
{
    uint64_t *set = r->scratch;
    carried(r, 0, r->before);
    memcpy(set, r->before, 8u * r->words);
    put(set, type->id);
    told(r, 0, r->before, set, set);
    single_set(r, type, true);
    memcpy(r->b->shared + type->cells, bytes, size);
}

bool storage_alive(storage_t *r, uint64_t id)
{
    return alive(r, id);
}

bool storage_any(storage_t *r, storage_type_t *type, bool (*is)(void *ctx, const uint8_t *row), void *ctx)
{
    if (type->kept == BRIDGE_SINGLETON) return type->set && is(ctx, r->b->shared + type->cells);
    for (uint32_t index = 1; type->kept == BRIDGE_DIRECT_INDEXED && index < r->high; index ++) {
        if (has(given_of(r, type->slot), index)
            && is(ctx, r->b->shared + type->cells + COLUMN + (size_t)index * type->size)) return true;
    }
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

storage_t *storage_create(bridge_t *b)
{
    storage_t *r = calloc(1, sizeof *r);
    size_t tables = (size_t)b->tables + b->sparse_sets + 1;
    if (bridge_lacks(b, r, "storage")) return nullptr;
    r->b = b;
    /* a table for each archetype there may be, and one for each sparse set */
    r->table_room = b->tables + b->sparse_sets;
    r->set_room = b->sparse_sets;
    r->direct_room = b->direct_indexed;
    r->bit_words = (b->entities + 63) / 64;
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
    r->before = calloc(r->words, 8);
    r->masked = calloc(r->words, 8);
    r->in_table = calloc(r->words, 8);
    r->set_type = calloc((size_t)r->set_room + 1, sizeof *r->set_type);
    r->set_table = calloc((size_t)r->set_room + 1, sizeof *r->set_table);
    r->where = calloc((size_t)(r->set_room ? r->set_room : 1) * b->entities, sizeof *r->where);
    r->direct_type = calloc((size_t)r->direct_room + 1, sizeof *r->direct_type);
    r->given = calloc((size_t)(r->direct_room ? r->direct_room : 1) * r->bit_words, 8);
    /* one more than there may be, so that looking for a free one always ends */
    r->types = calloc((size_t)r->type_room + 1, sizeof *r->types);
    r->type_pool = calloc(r->type_room, sizeof *r->type_pool);
    r->terms_pool = calloc((size_t)r->terms_room + 1, sizeof *r->terms_pool);
    r->lists = calloc((size_t)r->terms_room * 2 * (tables + 1), sizeof *r->lists);
    r->term_types = calloc((size_t)r->terms_room * b->max_terms, sizeof *r->term_types);
    r->term_opers = calloc((size_t)r->terms_room * b->max_terms, sizeof *r->term_opers);
    r->systems = calloc(r->terms_room, sizeof *r->systems);
    r->observers = calloc(r->terms_room, sizeof *r->observers);
    /* failure.md, *Out of memory*: all of it, or none */
    if (!r->places || !r->tables || !r->table_next || !r->dirty || !r->hash || !r->sets || !r->columns
        || !r->work || !r->changed || !r->folded || !r->dest || !r->attached || !r->at || !r->scratch
        || !r->types || !r->type_pool || !r->terms_pool || !r->lists || !r->term_types
        || !r->term_opers || !r->systems || !r->observers || !r->before || !r->masked
        || !r->in_table || !r->set_type || !r->set_table || !r->where || !r->direct_type
        || !r->given) {
        (void)bridge_lacks(b, nullptr, "storage, as [budget] sizes it");
        storage_destroy(r);
        return nullptr;
    }
    if (!r->who) {
        say_error(b, "[budget] cannot be satisfied: the operating system would not "
            "give storage %u bytes of its own for the columns it keeps", b->columns_len);
        storage_destroy(r);
        return nullptr;
    }
    return r;
}

void storage_destroy(storage_t *r)
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
    free(r->before);
    free(r->masked);
    free(r->in_table);
    free(r->set_type);
    free(r->set_table);
    free(r->where);
    free(r->direct_type);
    free(r->given);
    free(r);
}
