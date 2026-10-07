#include "storage_bytes.h"
#include "storage_carried.h"
#include "storage_reach.h"
#include "storage_set.h"
#include "storage_sets.h"
#include "storage_grow.h"
#include "storage_make.h"
#include "storage_strip.h"
#include "storage_table.h"
#include "storage_walk.h"
#include "storage_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- tables, by their set ---- */

/* ---- rows ---- */

static void place_row(storage_t *r, uint32_t index, uint32_t t, uint32_t row)
{
    place_t *p = &r->places[index];
    p->table = t;
    p->row = row;
    r->b->index[index] = (bridge_entity_t){ t, row, p->generation };
}

/* the entity at `index` has row `row` of a sparse set's table: kept here, and written where
 * modules read it */
static void seat_row(storage_t *r, const table_t *tab, uint32_t index, uint32_t row)
{
    const storage_type_t *type = r->set_type[tab->set - 1];
    get_where(r, tab->set - 1)[index] = row + 1;
    get_cells(r, tab->set - 1)[index] = get_cell(type, get_columns(r, (uint32_t)(tab - r->tables))[type->id], row);
}

/* a row at the end of `t`, which has room for it, for the entity at `index` */
static uint32_t append_row(storage_t *r, uint32_t t, uint32_t index)
{
    table_t *tab = &r->tables[t];
    uint64_t id = (uint64_t)r->places[index].generation << 32 | index;
    uint32_t row = tab->rows ++;
    get_who(r, tab)[row] = id;
    memcpy(r->b->shared + tab->block + 8ull * row, &id, 8);
    touch(r, t);
    return row;
}

/* a row leaves `t`: the last one is moved into its place, and the entity that was in it is told */
[[gnu::always_inline]] static inline void remove_row(storage_t *r, uint32_t t, uint32_t row)
{
    table_t *tab = &r->tables[t];
    uint32_t last = -- tab->rows, *columns = get_columns(r, t);
    touch(r, t);
    if (row == last) return;
    for (uint32_t w = 0; w < r->words; w ++) {
        for (uint64_t bits = get_set(r, t)[w]; bits; bits &= bits - 1) {
            const storage_type_t *type = r->types[w * 64 + (uint32_t)__builtin_ctzll(bits)];
            uint8_t *column = r->b->shared + columns[type->id];
            copy_bytes(column + (size_t)row * type->size, column + (size_t)last * type->size, type->size);
            if (type->kept == BRIDGE_TOGGLE) {
                turn_row(r, get_bits(columns[type->id], tab->room), row, is_on(r, columns[type->id], tab->room, last));
            }
        }
    }
    get_who(r, tab)[row] = get_who(r, tab)[last];
    memcpy(r->b->shared + tab->block + 8ull * row, &get_who(r, tab)[row], 8);
    if (tab->set) seat_row(r, tab, (uint32_t)get_who(r, tab)[row], row);
    else place_row(r, (uint32_t)get_who(r, tab)[row], t, row);
}

/* the entity at `index` joins the sparse set of `slot`, whose table has room for it: with a
 * value of zero, until an attach's is written */
static void join_set(storage_t *r, uint32_t slot, uint32_t index)
{
    const storage_type_t *type = r->set_type[slot];
    uint32_t t = r->set_table[slot], row = append_row(r, t, index);
    clear_bytes(r->b->shared + get_columns(r, t)[type->id] + (size_t)row * type->size, type->size);
    seat_row(r, &r->tables[t], index, row);
}

/* and leaves it: the set's last row takes its place */
static void leave_set(storage_t *r, uint32_t slot, uint32_t index)
{
    uint32_t row = get_where(r, slot)[index] - 1;
    get_where(r, slot)[index] = 0;
    get_cells(r, slot)[index] = 0;
    remove_row(r, r->set_table[slot], row);
}

/* where the entity at `index` has its value of `type`, from the shared base: 0 if it has none */
[[gnu::always_inline]] static inline uint32_t get_value(const storage_t *r, uint32_t index, const storage_type_t *type)
{
    uint32_t t = r->places[index].table, column;
    if (type->kept == BRIDGE_SPARSE_SET) {
        uint32_t row = get_where(r, type->slot)[index];
        return row ? get_columns(r, r->set_table[type->slot])[type->id] + (row - 1) * type->size : 0;
    }
    if (type->kept == BRIDGE_DIRECT_INDEXED) {
        return set_has(get_given(r, type->slot), index) ? type->cells + (uint32_t)COLUMN + index * type->size : 0;
    }
    if (type->kept == BRIDGE_SINGLETON) return type->set ? type->cells : 0;
    column = t ? get_columns(r, t)[type->id] : 0;
    return column ? column + r->places[index].row * type->size : 0;
}

/* a singleton comes to have a value, of zero until one is written, or to have none: and
 * every module is written for again, since where it is reached says which */
static void set_single(storage_t *r, storage_type_t *type, bool set)
{
    type->set = set;
    memset(r->b->shared + type->cells, 0, type->size);
    r->b->layout_moved = 1;
}

/* the entity at `index` is given, or loses, the direct-indexed component of `slot`: its bit,
 * here and where modules read it, and its cell made zero either way */
static void give_cell(storage_t *r, uint32_t slot, uint32_t index, bool on)
{
    const storage_type_t *type = r->direct_type[slot];
    if (on) set_put(get_given(r, slot), index), set_put(get_shown(r, slot), index);
    else set_take(get_given(r, slot), index), set_take(get_shown(r, slot), index);
    clear_bytes(r->b->shared + type->cells + COLUMN + (size_t)index * type->size, type->size);
}

/* the entity at `index` goes to `to`, which has room: what it carried that `to` also holds
 * goes with it, and what `to` holds that it did not is zero */
[[gnu::always_inline]] static inline void move_row(storage_t *r, uint32_t index, uint32_t to)
{
    place_t *p = &r->places[index];
    uint32_t from = p->table, row = p->row, at = 0;
    if (from == to) return;
    if (to) {
        uint32_t *into = get_columns(r, to), *out = from ? get_columns(r, from) : nullptr;
        at = append_row(r, to, index);
        for (uint32_t w = 0; w < r->words; w ++) {
            for (uint64_t bits = get_set(r, to)[w]; bits; bits &= bits - 1) {
                const storage_type_t *type = r->types[w * 64 + (uint32_t)__builtin_ctzll(bits)];
                uint8_t *cell = r->b->shared + into[type->id] + (size_t)at * type->size;
                if (out && out[type->id]) {
                    copy_bytes(cell, r->b->shared + out[type->id] + (size_t)row * type->size, type->size);
                } else {
                    clear_bytes(cell, type->size);
                }
                /* a toggled component is on when it is given, and as it was when it moves */
                if (type->kept == BRIDGE_TOGGLE) {
                    turn_row(r, get_bits(into[type->id], r->tables[to].room), at, !out || !out[type->id]
                        || is_on(r, out[type->id], r->tables[from].room, row));
                }
            }
        }
    }
    if (from) remove_row(r, from, row);
    place_row(r, index, to, at);
}

static bool is_alive(const storage_t *r, uint64_t id)
{
    uint32_t index = (uint32_t)id;
    return index && index < r->high && r->places[index].table != DEAD
        && r->places[index].generation == (uint32_t)(id >> 32);
}

/* gone: its row, and its index, which comes back under another generation */
[[gnu::always_inline]] static inline void kill_entity(storage_t *r, uint32_t index)
{
    place_t *p = &r->places[index];
    for (uint32_t slot = 0; r->apart && slot < r->set_room; slot ++) {
        if (r->set_type[slot] && get_where(r, slot)[index]) leave_set(r, slot, index);
    }
    for (uint32_t slot = 0; r->apart && slot < r->direct_room; slot ++) {
        if (r->direct_type[slot] && set_has(get_given(r, slot), index)) give_cell(r, slot, index, false);
    }
    if (p->table) remove_row(r, p->table, p->row);
    r->b->index[index] = (bridge_entity_t){ 0, 0, DEAD };
    p->table = DEAD;
    p->generation = p->generation + 1 >= DEAD ? 1 : p->generation + 1;
    p->row = r->free_head;
    r->free_head = index;
}

/* ---- observers: what each is owed for an entity, from what it carried and what it carries ---- */

/* data.md, *Observers*. `before` and `after` are null for an entity that carried nothing, or is
 * gone; `attached` is what an attach wrote a value of, and is still carried. */
static void tell_observers(const storage_t *r, uint64_t id, const uint64_t *before, const uint64_t *after,
                 const uint64_t *attached)
{
    for (uint32_t i = 0; i < r->observer_count; i ++) {
        const storage_terms_t *o = r->observers[i];
        /* an entity that carries nothing is in no table, and matches nothing */
        bool was = before && !set_is_empty(r, before) && terms_match(o, before);
        bool is = after && !set_is_empty(r, after) && terms_match(o, after), given = false;
        uint32_t owed;
        if (!was && !is) continue;
        for (uint32_t k = 0; attached && k < o->count; k ++) {
            given |= o->oper[k] != BRIDGE_NOT && set_has(attached, o->type[k]);
        }
        owed = !is ? BRIDGE_ON_LEAVE : (was ? 0 : BRIDGE_ON_ENTER) | (given ? BRIDGE_ON_ATTACH : 0);
        owed &= o->events;
        if (owed) o->heard(o->ctx, id, (enum bridge_event)owed);
    }
}

/* ---- at a barrier ---- */

static uint64_t *get_work(const storage_t *r, uint32_t index) { return r->work + (size_t)index * r->words; }
static uint64_t *get_attached(const storage_t *r, uint32_t index) { return r->attached + (size_t)index * r->words; }

static void start_fold(storage_t *r, uint32_t index)
{
    get_carried(r, index, get_work(r, index));
}

/* budgets.md, *What running out does*: the entity is left as it was, and it is a failure of
 * the module that asked the last change of it, toward that module's own budget */
static void drop_change(storage_t *r, uint32_t index, bool for_columns)
{
    r->folded[index] |= for_columns ? DROPPED | SHORT : DROPPED;
}

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

/* grants.md, *What a module may destroy*: an entity that carries something it may write, or
 * that carries nothing. Read from where the entity is, before this barrier moves it. */
[[gnu::always_inline]] static inline bool has_stake(const storage_t *r, const module_t *m, uint32_t index)
{
    uint32_t t = r->places[index].table;
    bool any = t != 0;
    for (uint32_t w = 0; t && m->writable && w < r->words; w ++) {
        if (get_set(r, t)[w] & m->writable[w]) return true;
    }
    if (!r->apart) return !any;
    for (uint32_t slot = 0; slot < r->set_room; slot ++) {
        if (!r->set_type[slot] || !get_where(r, slot)[index]) continue;
        if (m->writable && set_has(m->writable, r->set_type[slot]->id)) return true;
        any = true;
    }
    for (uint32_t slot = 0; slot < r->direct_room; slot ++) {
        if (!r->direct_type[slot] || !set_has(get_given(r, slot), index)) continue;
        if (m->writable && set_has(m->writable, r->direct_type[slot]->id)) return true;
        any = true;
    }
    return !any;
}

/* Room for one more of the rows table `t` is to gain at this barrier. The first time it is
 * asked of a table, room is found for all of them, or for as many as the column region is
 * found to hold: by halves down to a number that fits, and from there back up between it and
 * the last that did not. budgets.md, *What running out does* */
[[gnu::always_inline]] static inline bool make_room(storage_t *r, uint32_t t)
{
    table_t *tab = &r->tables[t];
    if (!tab->sized) {
        uint32_t most = tab->incoming;
        while (tab->incoming && !grow_table(r, t, tab->rows + tab->incoming)) tab->incoming /= 2;
        while (most - tab->incoming > 1) {
            uint32_t between = tab->incoming + (most - tab->incoming) / 2;
            if (grow_table(r, t, tab->rows + between)) tab->incoming = between;
            else most = between;
        }
        tab->sized = true;
    }
    if (!tab->incoming) return false;
    tab->incoming --;
    return true;
}

/* whether the entity at `index`, as the fold has it, is to join the sparse set of `slot` */
static bool is_joining(const storage_t *r, uint32_t slot, uint32_t index)
{
    return r->set_type[slot] && !get_where(r, slot)[index]
        && set_has(r->work + (size_t)index * r->words, r->set_type[slot]->id);
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
        if (from && set_is_same(r, want, get_set(r, from))) {
            t = from;
        } else if (!set_is_empty(r, want)) {
            t = make_table(r, want);
            if (!t) {
                drop_change(r, index, false);
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
            if (!is_joining(r, slot, index)) continue;
            if (!r->set_table[slot]) {
                set_clear(r, r->scratch);
                set_put(r->scratch, r->set_type[slot]->id);
                if (!make_table(r, r->scratch)) unreachable();
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
        if (t && t != r->places[index].table && !make_room(r, t)) {
            drop_change(r, index, true);
            continue;
        }
        for (uint32_t slot = 0; r->apart && slot < r->set_room; slot ++) {
            if (!is_joining(r, slot, index) || make_room(r, r->set_table[slot])) continue;
            drop_change(r, index, true);
            break;
        }
    }
}

/* the entity at `index` put where the fold has it: its table, its sparse sets, its cells */
[[gnu::always_inline]] static inline void place_one(storage_t *r, uint32_t index)
{
    if (r->folded[index] & DESTROYED) {
        kill_entity(r, index);
        return;
    }
    move_row(r, index, r->dest[index]);
    if (!r->apart) return;
    for (uint32_t slot = 0; slot < r->set_room; slot ++) {
        if (is_joining(r, slot, index)) join_set(r, slot, index);
        else if (r->set_type[slot] && get_where(r, slot)[index]
                 && !set_has(r->work + (size_t)index * r->words, r->set_type[slot]->id)) leave_set(r, slot, index);
    }
    /* a direct-indexed component has its cell already, so it cannot be short of room */
    for (uint32_t slot = 0; slot < r->direct_room; slot ++) {
        bool was, is;
        if (!r->direct_type[slot]) continue;
        was = set_has(get_given(r, slot), index);
        is = set_has(r->work + (size_t)index * r->words, r->direct_type[slot]->id);
        if (was != is) give_cell(r, slot, index, is);
    }
    for (uint32_t id = 0; !index && id < r->type_room; id ++) {
        storage_type_t *type = r->types[id];
        if (!type || type->kept != BRIDGE_SINGLETON || type->set == set_has(r->work, id)) continue;
        set_single(r, type, !type->set);
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
        if (r->tables[t].live && !r->tables[t].rows) free_table(r, t);
    }
}

/* threads.md: the queues are applied in the order the calls that filled them ran, and not a
 * module at a time. The next run of changes: one module's, of one turn. A module's own queue
 * is in that order already, so this takes the earliest of what each has next. */
static module_t *take_next_run(bridge_t *b, uint32_t *from, uint32_t *to)
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
    while ((m = take_next_run(b, &at, &end))) {
        while (at < end) {
            change_t ch;
            uint32_t index;
            memcpy(&ch, m->queue + at, sizeof ch);
            at += (uint32_t)sizeof ch + ((ch.size + 7u) & ~7u);
            index = (uint32_t)ch.id;
            /* a singleton is given and taken with the id 0, which is no entity: it is folded
             * as the one thing index 0 carries - storage.md, *A singleton* */
            if (ch.op != CHANGE_DESTROY && ch.c->type->kept == BRIDGE_SINGLETON ? ch.id != 0
                : !is_alive(r, ch.id) || r->folded[index] & DESTROYED) continue;
            if (!(r->folded[index] & TOUCHED)) {
                r->folded[index] = TOUCHED;
                r->changed[r->changed_count ++] = index;
                start_fold(r, index);
                if (r->observer_count) set_clear(r, get_attached(r, index));
            }
            if (ch.op == CHANGE_DESTROY) {
                if (has_stake(r, m, index)) r->folded[index] |= DESTROYED;
                else m->barred ++;
            } else if (ch.op == CHANGE_ATTACH) {
                set_put(get_work(r, index), ch.c->type->id);
                if (r->observer_count) set_put(get_attached(r, index), ch.c->type->id);
            } else {
                set_take(get_work(r, index), ch.c->type->id);
                if (r->observer_count) set_take(get_attached(r, index), ch.c->type->id);
            }
        }
    }

    reserve_all(r);

    /* 3. what observers are owed: for each entity, what it carried against what it will */
    for (i = 0; r->observer_count && i < r->changed_count; i ++) {
        uint32_t index = r->changed[i];
        if (r->folded[index] & DROPPED) continue;
        get_carried(r, index, r->before);
        tell_observers(r, (uint64_t)r->places[index].generation << 32 | index, r->before,
            r->folded[index] & DESTROYED ? nullptr : get_work(r, index), get_attached(r, index));
    }

    /* 4. each entity placed, once */
    for (i = 0; i < r->changed_count; i ++) {
        if (!(r->folded[r->changed[i]] & DROPPED)) place_one(r, r->changed[i]);
    }

    /* 5. the values, in the order they were asked: the last one written is the one that stays */
    for (m = b->modules; m; m = m->next) m->mark_at = 0;
    while ((m = take_next_run(b, &at, &end))) {
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
            to = get_value(r, index, ch.c->type);
            if (to) copy_bytes(b->shared + to, value, ch.size);
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

uint32_t storage_count_tables(storage_t *r)
{
    return r->table_room + 1;
}

/* one span: how many rows, where their ids are, and a base for each term */
static void write_span(const storage_t *r, const storage_terms_t *s, uint32_t t, uint8_t *at)
{
    const table_t *tab = &r->tables[t];
    const uint32_t *columns = get_columns(r, t);
    uint32_t value = tab->rows;
    memset(at, 0, BRIDGE_SPAN_SIZE(s->count));
    memcpy(at, &value, 4);
    memcpy(at + 4, &tab->block, 4);
    for (uint32_t i = 0; i < s->count; i ++) {
        value = s->oper[i] == BRIDGE_NOT ? 0 : get_handed(r, s->type[i], columns[s->type[i]]);
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
                cell[t * m->declared_count + s] = get_handed(r, id, get_columns(r, t)[id]);
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
                cell[t * m->declared_count + s] = get_handed(r, id, get_columns(r, t)[id]);
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
                write_span(r, s, s->list[i], m->priv + at);
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
                if (where) write_span(r, s, r->dirty[i], m->priv + slot->at + (where - 1) * BRIDGE_SPAN_SIZE(s->count));
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
void storage_end_publish(storage_t *r)
{
    for (uint32_t i = 0; i < r->dirty_count; i ++) r->tables[r->dirty[i]].touched = false;
    r->dirty_count = 0;
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

static void destroy_entity(storage_t *r, uint32_t index)
{
    uint64_t id = (uint64_t)r->places[index].generation << 32 | index;
    uint32_t from = r->places[index].table;
    get_carried(r, index, r->before);
    tell_observers(r, id, r->before, nullptr, nullptr);
    kill_entity(r, index);
    if (from && r->tables[from].live && !r->tables[from].rows) free_table(r, from);
}

/* a sparse set nothing is left in gives its table back, as a barrier's end does */
static void sweep_sets(storage_t *r)
{
    for (uint32_t slot = 0; slot < r->set_room; slot ++) {
        if (r->set_table[slot] && !r->tables[r->set_table[slot]].rows) free_table(r, r->set_table[slot]);
    }
}

/* whether the entity at `index` carries something that is not kept in a table */
static bool is_in_a_set(const storage_t *r, uint32_t index)
{
    for (uint32_t slot = 0; slot < r->set_room; slot ++) {
        if (r->set_type[slot] && get_where(r, slot)[index]) return true;
    }
    for (uint32_t slot = 0; slot < r->direct_room; slot ++) {
        if (r->direct_type[slot] && set_has(get_given(r, slot), index)) return true;
    }
    return false;
}

/* storage_drop, of a direct-indexed component: every entity that has been given one */
static void drop_cells(storage_t *r, const storage_type_t *type, enum bridge_on_unload policy)
{
    for (uint32_t index = 1; index < r->high; index ++) {
        uint64_t id = (uint64_t)r->places[index].generation << 32 | index;
        if (!set_has(get_given(r, type->slot), index)) continue;
        get_carried(r, index, r->before);
        memcpy(r->scratch, r->before, 8u * r->words);
        set_take(r->scratch, type->id);
        if (policy == BRIDGE_DELETE || (policy == BRIDGE_SWEEP && set_is_empty(r, r->scratch))) {
            destroy_entity(r, index);
        } else {
            tell_observers(r, id, r->before, r->scratch, nullptr);
            give_cell(r, type->slot, index, false);
        }
    }
    sweep_sets(r);
}

/* storage_drop, of a component kept in a sparse set: an entity at a time, from the last */
static void drop_set(storage_t *r, const storage_type_t *type, enum bridge_on_unload policy)
{
    uint32_t t;
    while ((t = r->set_table[type->slot]) && r->tables[t].rows) {
        uint64_t id = get_who(r, &r->tables[t])[r->tables[t].rows - 1];
        get_carried(r, (uint32_t)id, r->before);
        memcpy(r->scratch, r->before, 8u * r->words);
        set_take(r->scratch, type->id);
        if (policy == BRIDGE_DELETE || (policy == BRIDGE_SWEEP && set_is_empty(r, r->scratch))) {
            destroy_entity(r, (uint32_t)id);
        } else {
            tell_observers(r, id, r->before, r->scratch, nullptr);
            leave_set(r, type->slot, (uint32_t)id);
        }
    }
    sweep_sets(r);
}

/* lifetime.md, *What unload leaves behind*. A table at a time. */
void storage_drop(storage_t *r, storage_type_t *type, enum bridge_on_unload policy)
{
    uint64_t *set = r->scratch;
    uint32_t apart = 0;
    if (type->kept == BRIDGE_SPARSE_SET) {
        drop_set(r, type, policy);
        return;
    }
    if (type->kept == BRIDGE_DIRECT_INDEXED) {
        drop_cells(r, type, policy);
        return;
    }
    if (type->kept == BRIDGE_SINGLETON) {
        /* it is on no entity, so the three policies say one thing: it has no value */
        if (!type->set) return;
        get_carried(r, 0, r->before);
        memcpy(set, r->before, 8u * r->words);
        set_take(set, type->id);
        tell_observers(r, 0, r->before, set, nullptr);
        set_single(r, type, false);
        return;
    }
    for (uint32_t t = 1; t < r->table_high; t ++) {
        table_t *tab = &r->tables[t];
        uint32_t to;
        bool none;
        if (!tab->live || !set_has(get_set(r, t), type->id)) continue;
        memcpy(set, get_set(r, t), 8u * r->words);
        set_take(set, type->id);
        none = set_is_empty(r, set);
        if (policy == BRIDGE_DELETE || (policy == BRIDGE_SWEEP && none)) {
            /* from the last row, so that none is moved into the place of one already seen.
             * An entity that is in a sparse set carries something still, and is not swept */
            while (tab->live) {
                uint64_t id = get_who(r, tab)[tab->rows - 1];
                if (policy == BRIDGE_DELETE || !is_in_a_set(r, (uint32_t)id)) {
                    destroy_entity(r, (uint32_t)id);
                    continue;
                }
                get_carried(r, (uint32_t)id, r->before);
                memcpy(r->masked, r->before, 8u * r->words);
                set_take(r->masked, type->id);
                tell_observers(r, id, r->before, r->masked, nullptr);
                move_row(r, (uint32_t)id, 0);
                if (!tab->rows) free_table(r, t);
            }
            continue;
        }
        for (uint32_t row = tab->rows; row --;) {
            get_carried(r, (uint32_t)get_who(r, tab)[row], r->before);
            memcpy(r->masked, r->before, 8u * r->words);
            set_take(r->masked, type->id);
            tell_observers(r, get_who(r, tab)[row], r->before, r->masked, nullptr);
        }
        to = none ? 0 : find_table(r, set);
        if (none || (to && grow_table(r, to, r->tables[to].rows + tab->rows))) {
            /* nothing left to carry, or a table of what is left with room for them all */
            while (tab->rows) move_row(r, (uint32_t)get_who(r, tab)[tab->rows - 1], to);
            free_table(r, t);
        } else {
            /* no table of what is left, and this one becomes it; or one that has no room,
             * and until either empties there are two */
            if (to) apart += tab->rows;
            strip_table(r, t, type, !to);
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
    tell_observers(r, 0, r->before, set, set);
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
