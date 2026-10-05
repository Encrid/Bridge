/* rows.h, kept by flecs. docs/bridge/flecs.md.
 *
 * This is what store.c, abi.c, lifecycle.c and config.c did with flecs, moved behind the one
 * header and not otherwise changed: the entity index as a mirror of flecs' own, table ids for
 * tables flecs makes and frees, and a barrier worked one change at a time, because that is
 * the only way flecs is driven. It goes when storage of our own passes the gate -
 * docs/plan/PHASE10.md. */
#include "rows-flecs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* flecs_confine is a global, as ecs_os_api is, because the functions that allocate a column
 * have no world in scope. So which storage a column is for is whichever this thread is working
 * for: every way into Bridge-ASM700 says so before it reaches flecs. Two bridges in a process
 * are then two worlds that never see each other's block. */
static thread_local rows *live;

void rows_enter(rows *r)
{
    live = r;
}

/* ---- columns: flecs' allocator, carving the column region ---- */

static void *column_alloc(ecs_size_t size)
{
    live->b->layout_moved = 1;
    return store_column_alloc(live->b, size);
}

static void column_free(void *ptr, ecs_size_t size)
{
    live->b->layout_moved = 1;
    store_column_free(live->b, ptr, size);
}

static void *column_realloc(void *ptr, ecs_size_t old_size, ecs_size_t new_size)
{
    live->b->layout_moved = 1;
    return store_column_realloc(live->b, ptr, old_size, new_size);
}

/* ---- table ids: dense, ours, given the first time a table is seen holding something ---- */

static uint32_t table_id(rows *r, ecs_table_t *t)
{
    uint32_t mask, i;
    if (!t) return 0;
    if (r->table_count * 2 >= r->table_hash_room || !r->table_hash) {
        uint32_t room = r->table_hash_room ? r->table_hash_room : 256, k;
        while (r->table_count * 2 >= room) room *= 2;
        free(r->table_hash);
        r->table_hash = calloc(room, sizeof *r->table_hash);
        r->table_hash_room = room;
        for (k = 1; k < r->table_count; k ++) {
            if (!r->tables[k].table) continue;      /* retired */
            i = (uint32_t)(((uintptr_t)r->tables[k].table >> 4) * 2'654'435'761u) & (room - 1);
            while (r->table_hash[i]) i = (i + 1) & (room - 1);
            r->table_hash[i] = k;
        }
    }
    mask = r->table_hash_room - 1;
    i = (uint32_t)(((uintptr_t)t >> 4) * 2'654'435'761u) & mask;
    while (r->table_hash[i]) {
        if (r->tables[r->table_hash[i]].table == t) return r->table_hash[i];
        i = (i + 1) & mask;
    }
    r->b->layout_moved = 1;
    if (r->free_count) {
        /* an id whose table is gone, and which nothing in the index still says */
        uint32_t id = r->free_tables[-- r->free_count];
        r->tables[id].table = t;
        r->tables[id].rows = 0;
        r->table_hash[i] = id;
        return id;
    }
    if (r->table_count >= r->table_room) {
        r->table_room = r->table_room ? r->table_room * 2 : 64;
        r->tables = realloc(r->tables, r->table_room * sizeof *r->tables);
    }
    r->tables[r->table_count].table = t;
    r->tables[r->table_count].rows = 0;
    r->table_hash[i] = r->table_count;
    return r->table_count ++;
}

/* flecs says when it frees a table, from the one function every path to that goes through:
 * a component deleted, the children of something deleted, its own sweep of empty tables. The
 * id is dead until the next sync has moved every index record off it, then free. A table
 * with several ids in its type is announced once for each, so this has to be safe to repeat. */
static void on_table_gone(ecs_iter_t *it)
{
    rows *r = it->ctx;
    uint32_t id;
    for (id = 1; id < r->table_count && r->tables[id].table != it->table; id ++) {}
    if (id >= r->table_count) return;
    r->tables[id].table = nullptr;
    r->tables[id].rows = 0;
    if (r->dead_count == r->dead_room) {
        r->dead_room = r->dead_room ? r->dead_room * 2 : 16;
        r->dead_tables = realloc(r->dead_tables, r->dead_room * sizeof *r->dead_tables);
    }
    r->dead_tables[r->dead_count ++] = id;
    free(r->table_hash);            /* rebuilt without it at the next lookup */
    r->table_hash = nullptr;
    r->b->layout_moved = 1;
}

/* ---- the entity index mirror - flecs.md, *flecs will not tell you which* ---- */

void rows_flecs_touch(rows *r, ecs_entity_t e)
{
    uint32_t index = (uint32_t)e;
    if (index >= r->b->entities || r->is_dirty[index]) return;
    r->is_dirty[index] = 1;
    if (r->dirty_count == r->dirty_room) {
        r->dirty_room = r->dirty_room ? r->dirty_room * 2 : 1024;
        r->dirty = realloc(r->dirty, r->dirty_room * sizeof *r->dirty);
    }
    r->dirty[r->dirty_count ++] = e;
}

/* an event names an entity whose components changed: where it ended up is read at the barrier */
static void on_change(ecs_iter_t *it)
{
    int32_t i;
    for (i = 0; i < it->count; i ++) rows_flecs_touch(it->ctx, it->entities[i]);
}

/* and the hook names the one no event does: the entity a delete moved into the hole */
static void on_row_moved([[maybe_unused]] ecs_world_t *world, ecs_entity_t e, ecs_table_t *table,
                         int32_t row)
{
    rows *r = live;
    uint32_t index = (uint32_t)e;
    if (index >= r->b->entities) return;
    if (r->b->index[index].table == table_id(r, table)) r->b->index[index].row = (uint32_t)row;
    else rows_flecs_touch(r, e);
}

void rows_settle(rows *r)
{
    uint32_t i;
    for (i = 0; i < r->dirty_count; i ++) {
        uint32_t index = (uint32_t)r->dirty[i];
        /* by index, not by the id that was named: the index may have been recycled since */
        ecs_entity_t e = ecs_get_alive(r->world, index);
        bridge_entity *record = &r->b->index[index], now = { 0, 0, 0xffffffffu };
        if (e) {
            const ecs_record_t *found = ecs_record_find(r->world, e);
            now.table = table_id(r, found->table);
            now.row = (uint32_t)ECS_RECORD_TO_ROW(found->row);
            now.generation = (uint32_t)(e >> 32);
        }
        if (record->table) r->tables[record->table].rows --;
        if (now.table) r->tables[now.table].rows ++;
        *record = now;
        r->is_dirty[index] = 0;
    }
    r->dirty_count = 0;
    /* every entity a retired table held was named by an event and has just been placed */
    for (i = 0; i < r->dead_count; i ++) {
        if (r->free_count == r->free_room) {
            r->free_room = r->free_room ? r->free_room * 2 : 16;
            r->free_tables = realloc(r->free_tables, r->free_room * sizeof *r->free_tables);
        }
        r->free_tables[r->free_count ++] = r->dead_tables[i];
    }
    r->dead_count = 0;
}

/* ---- at a barrier ---- */

/* Each module's changes, in the order it loaded and then the order it asked: the same every
 * run, whichever thread ran what. An id that died in the meantime is passed over. */
void rows_apply(rows *r)
{
    module *m;
    for (m = r->b->modules; m; m = m->next) {
        uint32_t at = 0;
        while (at < m->queue_len) {
            change ch;
            const void *value = m->queue + at + sizeof ch;
            memcpy(&ch, m->queue + at, sizeof ch);
            at += (uint32_t)sizeof ch + ((ch.size + 7u) & ~7u);
            if (!ch.id || !ecs_is_alive(r->world, ch.id)) continue;
            if (ch.op == CHANGE_DESTROY) ecs_delete(r->world, ch.id);
            else if (ch.op == CHANGE_ATTACH) ecs_set_id(r->world, ch.id, ch.c->type->id, ch.size, value);
            else ecs_remove_id(r->world, ch.id, ch.c->type->id);
            rows_flecs_touch(r, ch.id);
        }
        m->queue_len = 0;
    }
}

/* They are real entities with nothing on them, which no query matches. */
uint32_t rows_reserve(rows *r, uint64_t *ids, uint32_t want)
{
    uint32_t count = 0;
    while (count < want) {
        ecs_entity_t e = ecs_new(r->world);
        /* the index is twelve bytes times the highest id, and that is what was budgeted */
        if ((uint32_t)e >= r->b->entities) {
            ecs_delete(r->world, e);
            break;
        }
        rows_flecs_touch(r, e);
        ids[count ++] = e;
    }
    return count;
}

uint32_t rows_tables(rows *r)
{
    return r->table_count;
}

void rows_written([[maybe_unused]] rows *r) { }

/* 1 if the module's span region cannot hold what its queries match */
int rows_publish(rows *r, module *m)
{
    bridge *b = r->b;
    auto h = (bridge_header *)m->priv;
    uint64_t end = (uint64_t)m->spans + m->spans_len, at;
    uint32_t q, t, s;

    store_header(b, m);
    at = ((uint64_t)h->column_base + 4ull * m->declared_count * r->table_count + 7u) & ~7ull;
    if (at > end) return 1;

    if (b->layout_moved) {
        auto cell = (uint32_t *)(m->priv + h->column_base);
        memset(cell, 0, 4u * m->declared_count);        /* table 0 is no table */
        for (t = 1; t < r->table_count; t ++) {
            for (s = 0; s < m->declared_count; s ++) {
                ecs_table_t *table = r->tables[t].table;
                int32_t column = !table ? -1
                    : ecs_table_get_column_index(r->world, table, m->declared[s]->type->id);
                cell[t * m->declared_count + s] =
                    column < 0 ? 0 : store_offset(b, ecs_table_get_column(table, column, 0));
            }
        }
    }

    for (q = 0; q < m->system_count; q ++) {
        bridge_system *sys = m->systems[q];
        auto slot = (bridge_query *)(m->priv + m->spans) + q;
        uint32_t size = BRIDGE_SPAN_SIZE(sys->terms), spans = 0;
        ecs_iter_t it;
        slot->spans = 0;
        slot->at = (uint32_t)at;
        if (!sys->matched) continue;
        it = ecs_query_iter(r->world, sys->matched->query);
        while (ecs_query_next(&it)) {
            uint8_t *span = m->priv + at;
            uint32_t id = table_id(r, it.table), value;
            if (at + size > end) {
                ecs_iter_fini(&it);
                return 1;
            }
            /* validation.md, *Always on*: what this catches is our own patch failing */
            if (r->tables[id].rows != it.count) {
                say(b, nullptr, nullptr, BRIDGE_ERR, "the entity index is out of step with flecs: "
                    "table %u has %d rows and the index counts %d", id, it.count, r->tables[id].rows);
            }
            memset(span, 0, size);
            value = (uint32_t)it.count;
            memcpy(span, &value, 4);
            value = store_offset(b, it.entities);
            memcpy(span + 4, &value, 4);
            for (t = 0; t < sys->terms; t ++) {
                uint64_t source = it.sources[t];
                value = ecs_field_is_set(&it, (int8_t)t)
                    ? store_offset(b, ecs_field_w_size(&it, sys->matched->sizes[t], (int8_t)t)) : 0;
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
        auto slot = (bridge_list *)(m->priv + h->observers) + q;
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

/* ---- at load: a type from its text ---- */

static rows_type *type_of(rows *r, ecs_entity_t id)
{
    for (uint32_t i = 0; i < r->type_count; i ++) if (r->types[i]->id == id) return r->types[i];
    return nullptr;
}

/* schema.md, *The type set*: flecs', minus anything pointer-sized or pointer-holding */
static const char *outside_type_set(ecs_world_t *world, ecs_entity_t type)
{
    const EcsType *t = ecs_get(world, type, EcsType);
    if (!t) return "a type nothing describes";
    switch (t->kind) {
    case EcsPrimitiveType: {
        ecs_primitive_kind_t kind = ecs_get(world, type, EcsPrimitive)->kind;
        return kind == EcsString || kind == EcsUPtr || kind == EcsIPtr
            ? "a pointer-sized type" : nullptr;
    }
    case EcsStructType: {
        const EcsStruct *s = ecs_get(world, type, EcsStruct);
        ecs_member_t *members = ecs_vec_first(&s->members);
        int32_t i, count = ecs_vec_count(&s->members);
        for (i = 0; i < count; i ++) {
            const char *why = outside_type_set(world, members[i].type);
            if (why) return why;
        }
        return nullptr;
    }
    case EcsArrayType:
        return outside_type_set(world, ecs_get(world, type, EcsArray)->type);
    case EcsEnumType:
    case EcsBitmaskType:
        return nullptr;
    default:
        return "a vector or an opaque type";
    }
}

/* what one field holds, as rows.h says it: flecs' kinds, an array taken as its element */
static void describe(rows *r, const ecs_member_t *member, rows_field *f)
{
    ecs_entity_t held = member->type;
    const EcsType *t = ecs_get(r->world, held, EcsType);
    f->offset = (uint32_t)member->offset;
    f->count = member->count > 1 ? (uint32_t)member->count : 1;
    if (t && t->kind == EcsArrayType) {
        const EcsArray *array = ecs_get(r->world, held, EcsArray);
        f->count = (uint32_t)array->count;
        held = array->type;
        t = ecs_get(r->world, held, EcsType);
    }
    if (!t) return;
    switch (t->kind) {
    case EcsPrimitiveType:
        switch (ecs_get(r->world, held, EcsPrimitive)->kind) {
        case EcsBool: f->kind = ROWS_BOOL; break;
        case EcsChar: f->kind = ROWS_CHAR; break;
        case EcsI8: f->kind = ROWS_I8; break;
        case EcsByte: case EcsU8: f->kind = ROWS_U8; break;
        case EcsI16: f->kind = ROWS_I16; break;
        case EcsU16: f->kind = ROWS_U16; break;
        case EcsI32: f->kind = ROWS_I32; break;
        case EcsU32: f->kind = ROWS_U32; break;
        case EcsI64: f->kind = ROWS_I64; break;
        case EcsU64: case EcsEntity: case EcsId: f->kind = ROWS_U64; break;
        case EcsF32: f->kind = ROWS_F32; break;
        case EcsF64: f->kind = ROWS_F64; break;
        default: break;
        }
        break;
    case EcsEnumType: f->kind = ROWS_ENUM; f->type = type_of(r, held); break;
    case EcsBitmaskType: f->kind = ROWS_BITMASK; f->type = type_of(r, held); break;
    case EcsStructType:
        /* bridge_text is given and not declared: it is no type of anybody's */
        if (held == r->text_type) f->kind = ROWS_TEXT;
        else { f->kind = ROWS_STRUCT; f->type = type_of(r, held); }
        break;
    default: break;
    }
}

enum rows_declared rows_declare(rows *r, const char *ns, const char *name, enum bridge_kind kind,
                                const char *desc, rows_type **type, uint32_t *size,
                                uint32_t *align, const char **why)
{
    static const ecs_type_kind_t kinds[] = { 0, EcsStructType, EcsEnumType, EcsBitmaskType };
    const EcsComponent *layout;
    ecs_entity_t id, scope, before;
    rows_type *t;
    int parsed;

    /* flecs parses the descriptor and computes the layout: Bridge-ASM700 parses no C.
     * A field's type is a bare C name, and flecs looks it up from the scope it is in. So
     * a declared type is named in flecs under its namespace, and a descriptor is parsed
     * from inside that namespace: `Vec3` in game:Body is game:Vec3 - schema.md. */
    scope = ecs_entity(r->world, { .name = ns, .parent = r->names });
    id = ecs_entity(r->world, { .name = name, .parent = scope });
    /* flecs makes an entity an enum when its underlying type is set, which parsing a
     * descriptor does not do. 32 bits, as a C enum is on every target a module has. */
    if (id && kind == BRIDGE_ENUM) {
        /* in place: flecs forbids copying an EcsEnum, so it cannot be set by value */
        ecs_ensure(r->world, id, EcsEnum)->underlying_type = ecs_id(ecs_i32_t);
        ecs_modified(r->world, id, EcsEnum);
    }
    before = ecs_set_scope(r->world, scope);
    parsed = id && !ecs_meta_from_desc(r->world, id, kinds[kind], desc);
    ecs_set_scope(r->world, before);
    layout = parsed ? ecs_get(r->world, id, EcsComponent) : nullptr;
    if (!layout) {
        if (id) ecs_delete(r->world, id);
        return ROWS_NOT_DESCRIBED;
    }
    *why = outside_type_set(r->world, id);
    if (*why) {
        ecs_delete(r->world, id);
        return ROWS_NOT_STORABLE;
    }

    t = calloc(1, sizeof *t);
    t->id = id;
    t->size = (uint32_t)layout->size;
    if (kind == BRIDGE_STRUCT) {
        const EcsStruct *s = ecs_get(r->world, id, EcsStruct);
        ecs_member_t *members = ecs_vec_first(&s->members);
        t->field_count = (uint32_t)ecs_vec_count(&s->members);
        t->fields = calloc(t->field_count ? t->field_count : 1, sizeof *t->fields);
        t->names = calloc(t->field_count ? t->field_count : 1, sizeof *t->names);
        for (uint32_t k = 0; k < t->field_count; k ++) {
            t->names[k] = copy(members[k].name, strlen(members[k].name));
            t->fields[k].name = t->names[k];
            describe(r, &members[k], &t->fields[k]);
        }
    }
    r->types = realloc(r->types, (r->type_count + 1) * sizeof *r->types);
    r->types[r->type_count ++] = t;
    *type = t;
    *size = (uint32_t)layout->size;
    *align = (uint32_t)layout->alignment;
    return ROWS_DECLARED;
}

static void type_free(rows_type *t)
{
    for (uint32_t k = 0; k < t->field_count; k ++) free(t->names[k]);
    free(t->names);
    free(t->fields);
    free(t);
}

void rows_undeclare(rows *r, rows_type *type)
{
    uint32_t k;
    ecs_delete(r->world, type->id);
    for (k = 0; r->types[k] != type; k ++) {}
    r->types[k] = r->types[-- r->type_count];
    type_free(type);
}

uint32_t rows_fields([[maybe_unused]] rows *r, rows_type *type, const rows_field **fields)
{
    *fields = type->fields;
    return type->field_count;
}

bool rows_constant(rows *r, rows_type *type, const char *name, int32_t *value)
{
    ecs_entity_t constant = ecs_lookup_child(r->world, type->id, name);
    const int32_t *v = constant
        ? ecs_get_id(r->world, constant, ecs_pair(EcsConstant, ecs_id(ecs_i32_t))) : nullptr;
    if (!v) return false;
    *value = *v;
    return true;
}

/* ---- at load: terms ---- */

static void terms_of(const rows_term *terms, uint32_t count, ecs_term_t *out, rows_terms *t)
{
    static const int16_t oper[] = { 0, EcsAnd, EcsNot, EcsOptional };
    for (uint32_t i = 0; i < count; i ++) {
        out[i].id = terms[i].type->id;
        out[i].oper = oper[terms[i].oper];
        t->sizes[i] = terms[i].type->size;
    }
    t->count = count;
}

rows_terms *rows_system(rows *r, const rows_term *terms, uint32_t count)
{
    rows_terms *t = calloc(1, sizeof *t);
    ecs_query_desc_t desc = {};
    terms_of(terms, count, desc.terms, t);
    /* flecs fails creation unless every term matches whole tables, which one span per
     * table assumes; what is left needs a read per row */
    desc.cache_kind = EcsQueryCacheAll;
    t->query = ecs_query_init(r->world, &desc);
    if (t->query && t->query->row_fields) {
        ecs_query_fini(t->query);
        t->query = nullptr;
    }
    if (!t->query) {
        free(t);
        return nullptr;
    }
    return t;
}

/* flecs says who and what; the observer itself runs at the barrier, after the republish */
static void on_trigger(ecs_iter_t *it)
{
    rows_terms *t = it->ctx;
    enum bridge_event event = it->event == EcsOnAdd ? BRIDGE_ON_ADD
                            : it->event == EcsOnSet ? BRIDGE_ON_SET : BRIDGE_ON_REMOVE;
    for (int32_t i = 0; i < it->count; i ++) t->heard(t->ctx, it->entities[i], event);
}

rows_terms *rows_observer(rows *r, const rows_term *terms, uint32_t count,
                          enum bridge_event events, rows_heard heard, void *ctx)
{
    rows_terms *t = calloc(1, sizeof *t);
    ecs_observer_desc_t od = {};
    uint32_t k = 0;
    terms_of(terms, count, od.query.terms, t);
    if (events & BRIDGE_ON_ADD) od.events[k ++] = EcsOnAdd;
    if (events & BRIDGE_ON_SET) od.events[k ++] = EcsOnSet;
    if (events & BRIDGE_ON_REMOVE) od.events[k ++] = EcsOnRemove;
    od.callback = on_trigger;
    od.ctx = t;
    t->heard = heard;
    t->ctx = ctx;
    t->observing = ecs_observer_init(r->world, &od);
    if (!t->observing) {
        free(t);
        return nullptr;
    }
    return t;
}

void rows_forget(rows *r, rows_terms *terms)
{
    if (terms->query) ecs_query_fini(terms->query);
    if (terms->observing) ecs_delete(r->world, terms->observing);
    free(terms);
}

/* ---- at unload, and for settings ---- */

/* lifetime.md, *What unload leaves behind*: what becomes of the entities that carried it */
void rows_drop(rows *r, rows_type *type, enum bridge_on_unload policy)
{
    ecs_entity_t *carried = nullptr;
    uint32_t count = 0, room = 0, i;
    if (policy == BRIDGE_DELETE) {
        ecs_delete_with(r->world, type->id);
        return;
    }
    if (policy == BRIDGE_SWEEP) {
        ecs_iter_t it = ecs_each_id(r->world, type->id);
        while (ecs_each_next(&it)) {
            for (int32_t row = 0; row < it.count; row ++) {
                if (count == room) {
                    room = room ? room * 2 : 64;
                    carried = realloc(carried, room * sizeof *carried);
                }
                carried[count ++] = it.entities[row];
            }
        }
    }
    ecs_remove_all(r->world, type->id);
    /* sweep: an entity left with nothing could never be found again, so it goes too */
    for (i = 0; i < count; i ++) {
        const ecs_type_t *left = ecs_is_alive(r->world, carried[i])
                               ? ecs_get_type(r->world, carried[i]) : nullptr;
        if (left && !left->count) ecs_delete(r->world, carried[i]);
    }
    free(carried);
}

void rows_release(rows *r, const uint64_t *ids, uint32_t count)
{
    for (uint32_t i = 0; i < count; i ++) ecs_delete(r->world, ids[i]);
}

uint64_t rows_row(rows *r, rows_type *type, const void *bytes, uint32_t size)
{
    ecs_entity_t e = ecs_new(r->world);
    if ((uint32_t)e >= r->b->entities) {
        ecs_delete(r->world, e);
        return 0;
    }
    ecs_set_id(r->world, e, type->id, size, bytes);
    rows_flecs_touch(r, e);
    return e;
}

bool rows_alive(rows *r, uint64_t id)
{
    return ecs_is_alive(r->world, id);
}

/* ---- create and destroy ---- */

static void flecs_says(int32_t level, [[maybe_unused]] const char *file,
                       [[maybe_unused]] int32_t line, const char *msg)
{
    if (live && level <= BRIDGE_WARN) say(live->b, nullptr, nullptr, (int8_t)level, "flecs: %s", msg);
}

rows *rows_create(bridge *b)
{
    rows *r = calloc(1, sizeof *r);
    ecs_os_api_t os;
    r->b = b;
    r->is_dirty = calloc(b->entities, 1);
    r->table_count = 1;                 /* table 0 is no table */

    live = r;
    /* flecs' own globals are set up by its first world, and are nobody's to race for */
    os_global_lock();
    flecs_confine = (flecs_confine_t){ column_alloc, column_realloc, column_free, on_row_moved };
    ecs_os_set_api_defaults();
    os = ecs_os_get_api();
    if (os.log_ != flecs_says) {
        os.log_ = flecs_says;
        ecs_os_set_api(&os);
    }
    r->world = ecs_init();
    os_global_unlock();
    ecs_observer(r->world, {
        .query.terms = {{ .id = EcsWildcard }},
        .events = { EcsOnAdd, EcsOnRemove },
        .callback = on_change,
        .ctx = r
    });
    /* EcsAny, not a wildcard: a wildcard is not told of a table whose type is only pairs,
     * and the tables of an enum's constants are that */
    ecs_observer(r->world, {
        .query.terms = {{ .id = EcsAny }},
        .events = { EcsOnTableDelete },
        .callback = on_table_gone,
        .ctx = r
    });
    r->names = ecs_entity(r->world, { .name = "bridge-asm700" });
    /* the one type that is given and not declared, where every namespace's lookup ends */
    r->text_type = ecs_struct(r->world, {
        .entity = ecs_entity(r->world, { .name = "bridge_text", .parent = r->names }),
        .members = {{ .name = "at", .type = ecs_id(ecs_u32_t) },
                    { .name = "len", .type = ecs_id(ecs_u32_t) }}
    });
    return r;
}

void rows_destroy(rows *r)
{
    live = r;
    if (r->world) {
        os_global_lock();
        ecs_fini(r->world);
        os_global_unlock();
    }
    for (uint32_t i = 0; i < r->type_count; i ++) type_free(r->types[i]);
    free(r->types);
    free(r->dead_tables);
    free(r->free_tables);
    free(r->tables);
    free(r->table_hash);
    free(r->dirty);
    free(r->is_dirty);
    free(r);
    live = nullptr;
}
