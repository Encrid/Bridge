#include "storage_bytes.h"
#include "storage_carried.h"
#include "storage_reach.h"
#include "storage_set.h"
#include "storage_sets.h"
#include "storage_grow.h"
#include "storage_make.h"
#include "storage_strip.h"
#include "storage_table.h"
#include "storage_tell.h"
#include "storage_unload.h"
#include "storage_change.h"
#include "storage_forget.h"
#include "storage_place.h"
#include "storage_room.h"
#include "storage_runs.h"
#include "storage_walk.h"
#include "storage_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- tables, by their set ---- */

/* ---- rows ---- */

/* ---- observers: what each is owed for an entity, from what it carried and what it carries ---- */

/* ---- at a barrier ---- */

/* false if there was not the memory for the set: asked before a type is made, so that
 * storage_let_write cannot fail */
bool storage_make_writable(storage_t *r, module_t *m)
{
    if (!m->writable) m->writable = calloc(r->words, sizeof *m->writable);
    return m->writable != nullptr;
}

void storage_let_write([[maybe_unused]] storage_t *r, module_t *m, storage_type_t *type)
{
    set_put(m->writable, type->id);
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

uint32_t storage_count_tables(storage_t *r)
{
    return r->table_room + 1;
}

/* ---- at load: a type, and terms ---- */

uint32_t storage_count_free_sets(const storage_t *r)
{
    uint32_t n = 0;
    for (uint32_t slot = 0; slot < r->set_room; slot ++) n += !r->set_type[slot];
    return n;
}

uint32_t storage_count_free_direct(const storage_t *r)
{
    uint32_t n = 0;
    for (uint32_t slot = 0; slot < r->direct_room; slot ++) n += !r->direct_type[slot];
    return n;
}

storage_type_t *storage_declare(storage_t *r, uint32_t size, uint32_t align, enum bridge_storage kept)
{
    uint32_t id, slot = 0, cells = 0, len = 0;
    for (id = 0; r->types[id]; id ++) {}        /* its declarer counted: there is one */
    /* and asked storage_count_free_sets or storage_count_free_direct first, so there is one of these too */
    for (; kept == BRIDGE_SPARSE_SET && r->set_type[slot]; slot ++) {}
    for (; kept == BRIDGE_DIRECT_INDEXED && r->direct_type[slot]; slot ++) {}
    if (kept == BRIDGE_DIRECT_INDEXED) {
        /* a cell for every entity there may be, behind the sixteen bytes that say where its
         * bits are: the one thing a declare asks of [budget] columns */
        uint64_t want = pad_column(COLUMN + (uint64_t)size * r->b->entities);
        uint32_t bits = r->b->direct + (uint32_t)(8ull * slot * r->bit_words);
        uint8_t *block = want > INT32_MAX ? nullptr : block_column_regrow(r->b, nullptr, 0, (uint32_t)want);
        if (!block) return nullptr;
        memset(block, 0, (size_t)want);
        memcpy(block, &bits, 4);
        cells = (uint32_t)(block - r->b->shared);
        len = (uint32_t)want;
        memset(get_given(r, slot), 0, 8u * r->bit_words);
        memset(get_shown(r, slot), 0, 8u * r->bit_words);
    }
    if (kept == BRIDGE_SINGLETON) {
        /* its one value, in a block of its own */
        uint64_t want = pad_column(size ? size : 1);
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
    else if (kept != BRIDGE_SINGLETON) set_put(r->in_table, id);
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
        set_take(r->in_table, type->id);
    }
    r->types[type->id] = nullptr;
}

static storage_terms_t *take_terms(storage_t *r, const storage_term_t *terms, uint32_t count)
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

storage_terms_t *storage_add_system(storage_t *r, const storage_term_t *terms, uint32_t count)
{
    storage_terms_t *s = take_terms(r, terms, count);
    for (uint32_t t = 1; t < r->table_high; t ++) {
        if (!r->tables[t].live || !terms_read(r, s, t)) continue;
        s->list[s->matched] = t;
        s->pos[t] = ++ s->matched;
    }
    s->changed = true;
    r->systems[r->system_count ++] = s;
    return s;
}

storage_terms_t *storage_add_observer(storage_t *r, const storage_term_t *terms, uint32_t count,
                          enum bridge_event events, storage_heard_t heard, void *ctx)
{
    storage_terms_t *o = take_terms(r, terms, count);
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

/* ---- at a reload ---- */

/* `visit` for every entity that carries `type`, whichever way it is kept */
static void visit_carriers(storage_t *r, const storage_type_t *type, void (*visit)(storage_t *r, uint32_t index, void *ctx),
                         void *ctx)
{
    if (type->kept == BRIDGE_SINGLETON) {
        if (type->set) visit(r, 0, ctx);
    } else if (type->kept == BRIDGE_DIRECT_INDEXED) {
        for (uint32_t index = 1; index < r->high; index ++) {
            if (set_has(get_given(r, type->slot), index)) visit(r, index, ctx);
        }
    } else {
        for (uint32_t t = 1; t < r->table_high; t ++) {
            const table_t *tab = &r->tables[t];
            if (!tab->live || !set_has(get_set(r, t), type->id)) continue;
            for (uint32_t row = 0; row < tab->rows; row ++) visit(r, (uint32_t)get_who(r, tab)[row], ctx);
        }
    }
}

static void count_one([[maybe_unused]] storage_t *r, [[maybe_unused]] uint32_t index, void *ctx)
{
    ++ *(uint32_t *)ctx;
}

uint32_t storage_count_carriers(storage_t *r, const storage_type_t *type)
{
    uint32_t n = 0;
    visit_carriers(r, type, count_one, &n);
    return n;
}

uint32_t storage_count_off(storage_t *r, const storage_type_t *type)
{
    uint32_t n = 0;
    for (uint32_t t = 1; type->kept == BRIDGE_TOGGLE && t < r->table_high; t ++) {
        const table_t *tab = &r->tables[t];
        if (!tab->live || !set_has(get_set(r, t), type->id)) continue;
        for (uint32_t row = 0; row < tab->rows; row ++) {
            n += !is_on(r, get_columns(r, t)[type->id], tab->room, row);
        }
    }
    return n;
}

/* the entity at `index` is among those this move changes: folded from what it carries */
static void fold_in(storage_t *r, uint32_t index)
{
    if (r->folded[index] & TOUCHED) return;
    r->folded[index] = TOUCHED;
    r->changed[r->changed_count ++] = index;
    get_carried(r, index, r->work + (size_t)index * r->words);
}

static void retype(storage_t *r, uint32_t index, void *ctx)
{
    const storage_move_t *move = ctx;
    fold_in(r, index);
    set_take(r->work + (size_t)index * r->words, move->from->id);
    /* a singleton is on no entity: what the one that carried it had becomes index 0's */
    if (move->to->kept == BRIDGE_SINGLETON) {
        fold_in(r, 0);
        set_put(r->work, move->to->id);
    } else {
        set_put(r->work + (size_t)index * r->words, move->to->id);
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
        if (moves[k].to->kept == BRIDGE_SINGLETON && storage_count_carriers(r, moves[k].from)) fold_in(r, 0);
    }
    for (k = 0; k < count; k ++) visit_carriers(r, moves[k].from, retype, (void *)(uintptr_t)&moves[k]);
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
            had[k] = get_value(r, index, moves[k].from);
            if (had[k]) memcpy(was + at, r->b->shared + had[k], moves[k].from->size);
            at += moves[k].from->size;
        }
        place_one(r, index);
        for (k = 0, at = 0; k < count && k < 64; at += moves[k].from->size, k ++) {
            uint32_t to;
            if (!had[k]) continue;
            to = get_value(r, moves[k].to->kept == BRIDGE_SINGLETON ? 0 : index, moves[k].to);
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
    for (uint32_t i = 0; i < count; i ++) if (is_alive(r, ids[i])) destroy_entity(r, (uint32_t)ids[i]);
    sweep_sets(r);
}

void storage_give_single(storage_t *r, storage_type_t *type, const void *bytes, uint32_t size)
{
    uint64_t *set = r->scratch;
    get_carried(r, 0, r->before);
    memcpy(set, r->before, 8u * r->words);
    set_put(set, type->id);
    tell_observers(r, &(seen_t){ 0, r->before, set, set });
    set_single(r, type, true);
    memcpy(r->b->shared + type->cells, bytes, size);
}

bool storage_is_alive(storage_t *r, uint64_t id)
{
    return is_alive(r, id);
}

bool storage_has_any(storage_t *r, storage_type_t *type, bool (*is)(void *ctx, const uint8_t *row), void *ctx)
{
    if (type->kept == BRIDGE_SINGLETON) return type->set && is(ctx, r->b->shared + type->cells);
    for (uint32_t index = 1; type->kept == BRIDGE_DIRECT_INDEXED && index < r->high; index ++) {
        if (set_has(get_given(r, type->slot), index)
            && is(ctx, r->b->shared + type->cells + COLUMN + (size_t)index * type->size)) return true;
    }
    for (uint32_t t = 1; t < r->table_high; t ++) {
        const uint8_t *column;
        if (!r->tables[t].live || !set_has(get_set(r, t), type->id)) continue;
        column = r->b->shared + get_columns(r, t)[type->id];
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
