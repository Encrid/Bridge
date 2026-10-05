/* A module's own settings: one row of the component its author named, written after init by
 * what each field is. docs/seam/host-api/config.md, *A module's own settings are a component*. */
#include "inside.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *const kinds[] = { "unset", "a bool", "an integer", "a float", "text" };

static int misfit(char *why, size_t len, const bridge_setting *s, const char *field)
{
    snprintf(why, len, "its setting '%.80s' is %s, and the field it names is %s", s->key,
        s->kind <= BRIDGE_STRING ? kinds[s->kind] : kinds[0], field);
    return 1;
}

/* an integer into an integer field it fits */
static int put_integer(uint8_t *at, ecs_primitive_kind_t kind, int64_t v)
{
    switch (kind) {
    case EcsChar: case EcsI8: if (v < INT8_MIN || v > INT8_MAX) return 1; *(int8_t *)at = (int8_t)v; return 0;
    case EcsByte: case EcsU8: if (v < 0 || v > UINT8_MAX) return 1; *at = (uint8_t)v; return 0;
    case EcsI16: if (v < INT16_MIN || v > INT16_MAX) return 1; { int16_t x = (int16_t)v; memcpy(at, &x, 2); } return 0;
    case EcsU16: if (v < 0 || v > UINT16_MAX) return 1; { uint16_t x = (uint16_t)v; memcpy(at, &x, 2); } return 0;
    case EcsI32: if (v < INT32_MIN || v > INT32_MAX) return 1; { int32_t x = (int32_t)v; memcpy(at, &x, 4); } return 0;
    case EcsU32: if (v < 0 || v > UINT32_MAX) return 1; { uint32_t x = (uint32_t)v; memcpy(at, &x, 4); } return 0;
    case EcsI64: memcpy(at, &v, 8); return 0;
    case EcsU64: case EcsEntity: case EcsId: if (v < 0) return 1; memcpy(at, &v, 8); return 0;
    default: return 1;
    }
}

/* the bytes of a text setting, in this module's block of the arena */
static uint32_t put_text(bridge *b, module *m, uint32_t *used, const char *text, uint32_t len)
{
    uint32_t at = m->text_at + *used;
    memcpy(b->shared + at, text, len);
    *used += (len + 7u) & ~7u;
    return at;
}

static int seed_one(bridge *b, module *m, uint8_t *row, const ecs_member_t *member,
                    const bridge_setting *s, uint32_t *text_used, char *why, size_t len)
{
    ecs_world_t *world = b->world;
    const EcsType *type = ecs_get(world, member->type, EcsType);
    uint8_t *at = row + member->offset;
    uint32_t text_len = s->kind == BRIDGE_STRING && s->v.s ? (uint32_t)strlen(s->v.s) : 0;

    if (!type) return misfit(why, len, s, "of a type nothing describes");

    /* char name[N]: the bytes, inline, and a zero after them */
    if (member->count > 1 || type->kind == EcsArrayType) {
        ecs_entity_t element = type->kind == EcsArrayType ? ecs_get(world, member->type, EcsArray)->type
                                                          : member->type;
        int32_t room = type->kind == EcsArrayType ? ecs_get(world, member->type, EcsArray)->count
                                                  : member->count;
        const EcsPrimitive *p = ecs_get(world, element, EcsPrimitive);
        if (!p || p->kind != EcsChar || s->kind != BRIDGE_STRING) {
            return misfit(why, len, s, "an array, which only text into a char array fills");
        }
        if (text_len + 1 > (uint32_t)room) {
            snprintf(why, len, "its setting '%.80s' is %u bytes of text, and the field it names "
                "holds %d with the zero that ends it", s->key, text_len, room);
            return 1;
        }
        memcpy(at, s->v.s, text_len);
        return 0;
    }

    switch (type->kind) {
    case EcsPrimitiveType: {
        ecs_primitive_kind_t kind = ecs_get(world, member->type, EcsPrimitive)->kind;
        if (kind == EcsBool) {
            if (s->kind != BRIDGE_BOOL) return misfit(why, len, s, "a bool");
            *at = s->v.b ? 1 : 0;
            return 0;
        }
        if (kind == EcsF32 || kind == EcsF64) {
            double v = s->kind == BRIDGE_FLOAT ? s->v.f : (double)s->v.i;
            float f = (float)v;
            if (s->kind != BRIDGE_FLOAT && s->kind != BRIDGE_INT) return misfit(why, len, s, "a float");
            if (kind == EcsF32) memcpy(at, &f, 4);
            else memcpy(at, &v, 8);
            return 0;
        }
        if (s->kind != BRIDGE_INT) return misfit(why, len, s, "an integer");
        if (put_integer(at, kind, s->v.i)) {
            snprintf(why, len, "its setting '%.80s' is %lld, which the field it names cannot hold",
                s->key, (long long)s->v.i);
            return 1;
        }
        return 0;
    }
    case EcsEnumType: {
        int32_t value = (int32_t)s->v.i;
        if (s->kind == BRIDGE_STRING) {
            /* by the name of a constant: the one thing a deployment can be expected to know */
            ecs_entity_t constant = s->v.s ? ecs_lookup_child(world, member->type, s->v.s) : 0;
            const int32_t *v = constant
                ? ecs_get_id(world, constant, ecs_pair(EcsConstant, ecs_id(ecs_i32_t))) : NULL;
            if (!v) {
                snprintf(why, len, "its setting '%.80s' is '%.60s', which the enum of the field "
                    "it names has no constant for", s->key, s->v.s ? s->v.s : "");
                return 1;
            }
            value = *v;
        } else if (s->kind != BRIDGE_INT) {
            return misfit(why, len, s, "an enum");
        }
        memcpy(at, &value, 4);
        return 0;
    }
    case EcsBitmaskType:
        if (s->kind != BRIDGE_INT || s->v.i < 0 || s->v.i > UINT32_MAX) return misfit(why, len, s, "a bitmask");
        { uint32_t x = (uint32_t)s->v.i; memcpy(at, &x, 4); }
        return 0;
    case EcsStructType:
        /* bridge_text: (offset, length) into the persistent arena - strings.md */
        if (member->type == b->text_type && s->kind == BRIDGE_STRING) {
            bridge_text text = { put_text(b, m, text_used, s->v.s, text_len), text_len };
            memcpy(at, &text, sizeof text);
            return 0;
        }
        return misfit(why, len, s, "a struct, which only text into a bridge_text fills");
    default:
        return misfit(why, len, s, "of a type no setting fills");
    }
}

/* 0, or `why` says which setting or which name was refused. */
int config_seed(bridge *b, module *m, const bridge_module_config *cfg, char *why, size_t len)
{
    ecs_world_t *world = b->world;
    component *c = NULL;
    const EcsStruct *layout;
    ecs_member_t *members;
    int32_t member_count, k;
    uint32_t i, text_total = 0, text_used = 0;
    uint8_t *row;
    module *other;
    ecs_entity_t e;

    if (!cfg->config) {
        if (!cfg->setting_count) return 0;
        snprintf(why, len, "it is handed %u settings and names no config component for them",
            cfg->setting_count);
        return 1;
    }
    for (i = 0; i < m->declared_count; i ++) {
        if (!strcmp(m->declared[i]->name, cfg->config)) c = m->declared[i];
    }
    layout = c ? ecs_get(world, c->id, EcsStruct) : NULL;
    if (!layout) {
        snprintf(why, len, "its config is '%.100s', which is not a struct its init declared",
            cfg->config);
        return 1;
    }
    /* one row, so one module: two would be two rows of a singleton */
    for (other = b->modules; other; other = other->next) {
        if (other->config == c) {
            snprintf(why, len, "its config is '%.100s', which is already the settings of module "
                "'%.80s'", c->name, other->identity);
            return 1;
        }
    }
    members = ecs_vec_first(&layout->members);
    member_count = ecs_vec_count(&layout->members);

    /* the text bridge_text fields will hold, to know where it goes before any is written */
    for (i = 0; i < cfg->setting_count; i ++) {
        const bridge_setting *s = &cfg->settings[i];
        for (k = 0; k < member_count; k ++) {
            if (s->key && !strcmp(members[k].name, s->key) && members[k].type == b->text_type
                && s->kind == BRIDGE_STRING && s->v.s) {
                text_total += ((uint32_t)strlen(s->v.s) + 7u) & ~7u;
            }
        }
    }
    if (text_total) {
        /* strings.md, *Bridge-ASM700 reuses its own range*: where this module's text went the
         * last time it was loaded, when it still fits there */
        for (i = 0; i < b->kept_count; i ++) {
            if (!strcmp(b->kept[i].stem, m->stem) && b->kept[i].room >= text_total) {
                m->text_at = b->kept[i].at;
                m->text_room = b->kept[i].room;
            }
        }
        if (!m->text_room) {
            if (m->persistent <= 0 || text_total > m->arena_left) {
                snprintf(why, len, "its settings hold %u bytes of text, and its persistent share "
                    "has %u left", text_total, m->persistent > 0 ? m->arena_left : 0);
                return 1;
            }
            m->text_at = m->arena_at;
            m->text_room = text_total;
            m->arena_at += text_total;
            m->arena_left -= text_total;
        }
    }

    row = calloc(1, c->size);
    for (i = 0; i < cfg->setting_count; i ++) {
        const bridge_setting *s = &cfg->settings[i];
        for (k = 0; k < member_count && (!s->key || strcmp(members[k].name, s->key)); k ++) {}
        if (k == member_count) {
            snprintf(why, len, "its settings have a key '%.80s', and '%.100s' has no field of "
                "that name", s->key ? s->key : "", c->name);
            free(row);
            return 1;
        }
        if (seed_one(b, m, row, &members[k], s, &text_used, why, len)) {
            free(row);
            return 1;
        }
    }

    e = ecs_new(world);
    if ((uint32_t)e >= b->entities) {
        ecs_delete(world, e);
        free(row);
        snprintf(why, len, "the row its settings go on does not fit under [budget] entities of %u",
            b->entities);
        return 1;
    }
    ecs_set_id(world, e, c->id, c->size, row);
    store_touch(b, e);
    free(row);
    m->config = c;
    m->config_entity = e;
    return 0;
}

/* The block a module's settings text is in outlives the module: its next load writes there. */
void config_keep(bridge *b, module *m)
{
    uint32_t i;
    if (!m->text_room) return;
    for (i = 0; i < b->kept_count; i ++) {
        if (strcmp(b->kept[i].stem, m->stem)) continue;
        b->kept[i].at = m->text_at;
        b->kept[i].room = m->text_room;
        return;
    }
    b->kept = realloc(b->kept, (b->kept_count + 1) * sizeof *b->kept);
    b->kept[b->kept_count].stem = copy(m->stem, strlen(m->stem));
    b->kept[b->kept_count].at = m->text_at;
    b->kept[b->kept_count ++].room = m->text_room;
}
