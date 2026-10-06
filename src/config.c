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
static int put_integer(uint8_t *at, enum schema_kind kind, int64_t v)
{
    switch (kind) {
    case SCHEMA_CHAR: case SCHEMA_I8: if (v < INT8_MIN || v > INT8_MAX) return 1; *(int8_t *)at = (int8_t)v; return 0;
    case SCHEMA_U8: if (v < 0 || v > UINT8_MAX) return 1; *at = (uint8_t)v; return 0;
    case SCHEMA_I16: if (v < INT16_MIN || v > INT16_MAX) return 1; { int16_t x = (int16_t)v; memcpy(at, &x, 2); } return 0;
    case SCHEMA_U16: if (v < 0 || v > UINT16_MAX) return 1; { uint16_t x = (uint16_t)v; memcpy(at, &x, 2); } return 0;
    case SCHEMA_I32: if (v < INT32_MIN || v > INT32_MAX) return 1; { int32_t x = (int32_t)v; memcpy(at, &x, 4); } return 0;
    case SCHEMA_U32: if (v < 0 || v > UINT32_MAX) return 1; { uint32_t x = (uint32_t)v; memcpy(at, &x, 4); } return 0;
    case SCHEMA_I64: memcpy(at, &v, 8); return 0;
    case SCHEMA_U64: if (v < 0) return 1; memcpy(at, &v, 8); return 0;
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

/* the value of an enum's constant, by its name */
static bool constant(const declared_type *of, const char *name, int64_t *value)
{
    for (uint32_t i = 0; i < of->s.constant_count; i ++) {
        if (strcmp(of->s.constants[i].name, name)) continue;
        *value = of->s.constants[i].value;
        return true;
    }
    return false;
}

static int seed_one(bridge *b, module *m, uint8_t *row, const schema_field *field,
                    const bridge_setting *s, uint32_t *text_used, char *why, size_t len)
{
    uint8_t *at = row + field->offset;
    uint32_t text_len = s->kind == BRIDGE_STRING && s->v.s ? (uint32_t)strlen(s->v.s) : 0;

    /* char name[N]: the bytes, inline, and a zero after them */
    if (field->count > 1) {
        if (field->kind != SCHEMA_CHAR || s->kind != BRIDGE_STRING) {
            return misfit(why, len, s, "an array, which only text into a char array fills");
        }
        if (text_len + 1 > field->count) {
            snprintf(why, len, "its setting '%.80s' is %u bytes of text, and the field it names "
                "holds %d with the zero that ends it", s->key, text_len, (int)field->count);
            return 1;
        }
        memcpy(at, s->v.s, text_len);
        return 0;
    }

    switch (field->kind) {
    case SCHEMA_BOOL:
        if (s->kind != BRIDGE_BOOL) return misfit(why, len, s, "a bool");
        *at = s->v.b ? 1 : 0;
        return 0;
    case SCHEMA_F32:
    case SCHEMA_F64: {
        double v = s->kind == BRIDGE_FLOAT ? s->v.f : (double)s->v.i;
        float f = (float)v;
        if (s->kind != BRIDGE_FLOAT && s->kind != BRIDGE_INT) return misfit(why, len, s, "a float");
        if (field->kind == SCHEMA_F32) memcpy(at, &f, 4);
        else memcpy(at, &v, 8);
        return 0;
    }
    case SCHEMA_CHAR: case SCHEMA_I8: case SCHEMA_U8: case SCHEMA_I16: case SCHEMA_U16:
    case SCHEMA_I32: case SCHEMA_U32: case SCHEMA_I64: case SCHEMA_U64:
        if (s->kind != BRIDGE_INT) return misfit(why, len, s, "an integer");
        if (put_integer(at, field->kind, s->v.i)) {
            snprintf(why, len, "its setting '%.80s' is %lld, which the field it names cannot hold",
                s->key, (long long)s->v.i);
            return 1;
        }
        return 0;
    case SCHEMA_ENUM: {
        const declared_type *of = field->type;
        int64_t value = s->v.i, most = INT64_MAX >> (64 - 8 * of->s.size);
        /* an enum is the integer its author stated: as many bytes, signed or not */
        bool is_signed = of->s.base == SCHEMA_I8 || of->s.base == SCHEMA_I16 || of->s.base == SCHEMA_I32;
        if (s->kind == BRIDGE_INT && (is_signed ? value < -most - 1 || value > most
                                                : value < 0 || value > most * 2 + 1)) {
            snprintf(why, len, "its setting '%.80s' is %lld, which the enum of the field it names "
                "cannot hold", s->key, (long long)value);
            return 1;
        }
        if (s->kind == BRIDGE_STRING) {
            /* by the name of a constant: the one thing a deployment can be expected to know */
            if (!s->v.s || !field->type || !constant(field->type, s->v.s, &value)) {
                snprintf(why, len, "its setting '%.80s' is '%.60s', which the enum of the field "
                    "it names has no constant for", s->key, s->v.s ? s->v.s : "");
                return 1;
            }
        } else if (s->kind != BRIDGE_INT) {
            return misfit(why, len, s, "an enum");
        }
        memcpy(at, &value, of->s.size);         /* the low bytes: every target is little-endian */
        return 0;
    }
    case SCHEMA_BITMASK: {
        const declared_type *of = field->type;
        uint64_t most = UINT64_MAX >> (64 - 8 * of->s.size);
        if (s->kind != BRIDGE_INT || s->v.i < 0 || (uint64_t)s->v.i > most) return misfit(why, len, s, "a bitmask");
        memcpy(at, &s->v.i, of->s.size);
        return 0;
    }
    case SCHEMA_TEXT:
        /* bridge_text: (offset, length) into the persistent arena - strings.md */
        if (s->kind == BRIDGE_STRING) {
            bridge_text text = { put_text(b, m, text_used, s->v.s, text_len), text_len };
            memcpy(at, &text, sizeof text);
            return 0;
        }
        return misfit(why, len, s, "a struct, which only text into a bridge_text fills");
    case SCHEMA_STRUCT:
        return misfit(why, len, s, "a struct, which only text into a bridge_text fills");
    default:
        return misfit(why, len, s, "of a type no setting fills");
    }
}

/* 0, or `why` says which setting or which name was refused. */
int config_seed(bridge *b, module *m, const bridge_module_config *cfg, char *why, size_t len)
{
    declared_type *c = nullptr;
    const schema_field *fields = nullptr;
    uint32_t field_count, k;
    uint32_t i, text_total = 0, text_used = 0;
    uint8_t *row;
    module *other;
    uint64_t e;

    if (!strcmp(cfg->config, BRIDGE_NO_CONFIG)) {
        if (!cfg->setting_count) return 0;
        snprintf(why, len, "it is handed %u settings, and states that it has no config component",
            cfg->setting_count);
        return 1;
    }
    for (i = 0; i < m->declared_count; i ++) {
        if (!strcmp(m->declared[i]->name, cfg->config)) c = m->declared[i];
    }
    if (!c || c->kind != BRIDGE_STRUCT) {
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
    fields = c->s.fields;
    field_count = c->s.field_count;

    /* the text bridge_text fields will hold, to know where it goes before any is written */
    for (i = 0; i < cfg->setting_count; i ++) {
        const bridge_setting *s = &cfg->settings[i];
        for (k = 0; k < field_count; k ++) {
            if (s->key && !strcmp(fields[k].name, s->key) && fields[k].kind == SCHEMA_TEXT
                && fields[k].count == 1 && s->kind == BRIDGE_STRING && s->v.s) {
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
        for (k = 0; k < field_count && (!s->key || strcmp(fields[k].name, s->key)); k ++) {}
        if (k == field_count) {
            snprintf(why, len, "its settings have a key '%.80s', and '%.100s' has no field of "
                "that name", s->key ? s->key : "", c->name);
            free(row);
            return 1;
        }
        if (seed_one(b, m, row, &fields[k], s, &text_used, why, len)) {
            free(row);
            return 1;
        }
    }

    e = storage_row(b->storage, c->type, row, c->size);
    free(row);
    if (!e) {
        snprintf(why, len, "the row its settings go on does not fit under [budget] entities of %u",
            b->entities);
        return 1;
    }
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
    {
        /* failure.md, *Out of memory*: an unload does not fail. What is lost is the reuse of
         * that range by the module's next load, and it is said */
        kept_text *more = realloc(b->kept, (b->kept_count + 1) * sizeof *b->kept);
        char *stem = copy(m->stem, strlen(m->stem));
        if (more) b->kept = more;
        if (!more || !stem) {
            free(stem);
            (void)bridge_short(b, nullptr, "the place of an unloaded module's settings text, which "
                "its next load will not write over");
            return;
        }
        b->kept[b->kept_count].stem = stem;
        b->kept[b->kept_count].at = m->text_at;
        b->kept[b->kept_count ++].room = m->text_room;
    }
}


/* Its share has gone back to the arena, and the block its settings text was in with it. */
void config_forget(bridge *b, const module *m)
{
    for (uint32_t i = 0; i < b->kept_count; i ++) {
        if (strcmp(b->kept[i].stem, m->stem)) continue;
        free(b->kept[i].stem);
        b->kept[i] = b->kept[-- b->kept_count];
        return;
    }
}
