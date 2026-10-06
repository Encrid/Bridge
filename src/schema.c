/* A type from its text. docs/modules/schema.md, *The type set*.
 *
 * The text is what a module's compiler made of the body of its BRIDGE_SCHEMA_STRUCT, BRIDGE_SCHEMA_ENUM or
 * BRIDGE_SCHEMA_BITMASK: tokens with one space between them. What it may say is written out here in
 * full, and nothing else is accepted:
 *
 *     a struct      { field field ... }
 *     a field       type name ;   or   type name [ count ] ;
 *     a type        bool char int8_t int16_t int32_t int64_t uint8_t uint16_t uint32_t
 *                   uint64_t float double bridge_text_t, or a type its namespace declared
 *     an enum       integer { name , name = integer , ... }   counted from zero, or from
 *                                                             the last
 *     a bitmask     unsigned { name = integer , ... }         every bit stated
 *
 * It lays a struct out as C does on every target a module is built for: each field at the
 * next multiple of its alignment, the whole a multiple of its widest. An enum and a bitmask
 * are the integer their author stated before the brace: int8_t to uint32_t for an enum, an
 * unsigned one to uint64_t for a bitmask. Every constant is checked to fit it. */
#include "schema.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const struct { const char *name; enum schema_kind kind; uint32_t size; } given[] = {
    { "bool", SCHEMA_BOOL, 1 },       { "char", SCHEMA_CHAR, 1 },
    { "int8_t", SCHEMA_I8, 1 },       { "uint8_t", SCHEMA_U8, 1 },
    { "int16_t", SCHEMA_I16, 2 },     { "uint16_t", SCHEMA_U16, 2 },
    { "int32_t", SCHEMA_I32, 4 },     { "uint32_t", SCHEMA_U32, 4 },
    { "int64_t", SCHEMA_I64, 8 },     { "uint64_t", SCHEMA_U64, 8 },
    { "float", SCHEMA_F32, 4 },       { "double", SCHEMA_F64, 8 },
};

typedef struct {
    const char *at;
    char        word[64];           /* the token just read: a name, a number, or one mark */
    bool        cut;                /* it was longer than any name may be */
    char       *why;
    size_t      why_len;
} reader_t;

static bool is_name(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
}

/* the next token into r->word: a run of name characters, or a single other character. Empty
 * at the end of the text. A run too long for any name is cut, and will be refused as a name. */
static void next(reader_t *r)
{
    size_t n = 0;
    r->cut = false;
    while (*r->at == ' ' || *r->at == '\t' || *r->at == '\n' || *r->at == '\r') r->at ++;
    if (is_name(*r->at) || (*r->at == '-' && is_name(r->at[1]))) {
        do {
            if (n + 1 < sizeof r->word) r->word[n ++] = *r->at;
            else r->cut = true;
            r->at ++;
        } while (is_name(*r->at));
    } else if (*r->at) {
        r->word[n ++] = *r->at ++;
    }
    r->word[n] = 0;
}

static int refuse(reader_t *r, const char *what)
{
    snprintf(r->why, r->why_len, what, r->word[0] ? r->word : "the end of it");
    return 1;
}

static bool is(const reader_t *r, const char *word)
{
    return !strcmp(r->word, word);
}

/* a name a field or a constant may have: it begins with a letter or an underscore */
static bool names(const reader_t *r)
{
    char c = r->word[0];
    return !r->cut && ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_');
}

/* the token as an integer: decimal or 0x, signed. false if it is not one */
/* an integer as it is written: below zero, or its bits, which reach past INT64_MAX */
static bool integer(const reader_t *r, int64_t *value, bool *below)
{
    char *end;
    if (!r->word[0] || (!(r->word[0] >= '0' && r->word[0] <= '9') && r->word[0] != '-')) return false;
    *below = r->word[0] == '-';
    errno = 0;
    if (*below) *value = strtoll(r->word, &end, 0);
    else *value = (int64_t)strtoull(r->word, &end, 0);
    return !*end && end != r->word && !errno;
}

/* whether the integer `base` is holds a value */
static bool fits(enum schema_kind base, int64_t value, bool below)
{
    switch (base) {
    case SCHEMA_I8:  return below ? value >= INT8_MIN : (uint64_t)value <= INT8_MAX;
    case SCHEMA_I16: return below ? value >= INT16_MIN : (uint64_t)value <= INT16_MAX;
    case SCHEMA_I32: return below ? value >= INT32_MIN : (uint64_t)value <= INT32_MAX;
    case SCHEMA_U8:  return !below && (uint64_t)value <= UINT8_MAX;
    case SCHEMA_U16: return !below && (uint64_t)value <= UINT16_MAX;
    case SCHEMA_U32: return !below && (uint64_t)value <= UINT32_MAX;
    case SCHEMA_U64: return !below;
    default: return false;
    }
}

static char *kept(const char *s)
{
    size_t n = strlen(s) + 1;
    char *c = malloc(n);
    if (c) memcpy(c, s, n);
    return c;
}

static int a_struct(reader_t *r, schema_find_t find, void *ctx, schema_t *out)
{
    uint32_t room = 0;
    for (next(r); !is(r, "}"); next(r)) {
        schema_field_t f = { .count = 1 };
        uint32_t size = 0, align = 0, i;
        schema_named_t named;

        for (i = 0; i < sizeof given / sizeof *given && !is(r, given[i].name); i ++) {}
        if (i < sizeof given / sizeof *given) {
            f.kind = given[i].kind;
            size = align = given[i].size;
        } else if (is(r, "bridge_text_t")) {
            /* given and not declared: (offset, length) into the persistent arena */
            f.kind = SCHEMA_TEXT;
            size = sizeof(bridge_text_t);
            align = alignof(bridge_text_t);
        } else if (names(r) && find(ctx, r->word, &named)) {
            f.kind = named.kind == BRIDGE_STRUCT ? SCHEMA_STRUCT
                   : named.kind == BRIDGE_ENUM ? SCHEMA_ENUM : SCHEMA_BITMASK;
            f.type = named.type;
            size = named.size;
            align = named.align;
        } else {
            return refuse(r, "says '%s', which is neither a type a field can be nor one its "
                "namespace has declared");
        }

        next(r);
        if (!names(r)) return refuse(r, "says '%s' where a field's name belongs");
        if (out->field_count == room) {
            schema_field_t *more = realloc(out->fields, (room ? room * 2 : 8) * sizeof *out->fields);
            if (!more) return refuse(r, "ran out of memory at '%s'");
            out->fields = more;
            room = room ? room * 2 : 8;
        }
        f.name = kept(r->word);
        if (!f.name) return refuse(r, "ran out of memory at '%s'");
        out->fields[out->field_count ++] = f;

        next(r);
        if (is(r, "[")) {
            int64_t count;
            next(r);
            bool below;
            if (!integer(r, &count, &below) || below || count < 1 || count > UINT32_MAX / size) {
                return refuse(r, "gives '%s' as the length of an array");
            }
            out->fields[out->field_count - 1].count = (uint32_t)count;
            size *= (uint32_t)count;
            next(r);
            if (!is(r, "]")) return refuse(r, "says '%s' where an array's length ends");
            next(r);
        }
        if (!is(r, ";")) return refuse(r, "says '%s' where a field ends");

        out->size = (out->size + align - 1) & ~(align - 1);
        out->fields[out->field_count - 1].offset = out->size;
        if (size > UINT32_MAX - out->size - 8) return refuse(r, "is more bytes than can be counted, at '%s'");
        out->size += size;
        if (align > out->align) out->align = align;
    }
    if (!out->field_count) return refuse(r, "has no field");
    out->size = (out->size + out->align - 1) & ~(out->align - 1);
    return 0;
}

static int constants(reader_t *r, bool bits, schema_t *out)
{
    uint32_t room = 0, i;
    int64_t value = -1;
    bool below = true;
    /* the integer it is, stated: schema.md, *The type set* */
    for (i = 0; i < sizeof given / sizeof *given && !is(r, given[i].name); i ++) {}
    if (i == sizeof given / sizeof *given || given[i].kind < SCHEMA_I8 || given[i].kind > SCHEMA_U64
        || given[i].kind == SCHEMA_I64 || (given[i].kind == SCHEMA_U64 && !bits)) {
        return refuse(r, "begins with '%s' and not with the integer it is");
    }
    if (bits && (given[i].kind == SCHEMA_I8 || given[i].kind == SCHEMA_I16 || given[i].kind == SCHEMA_I32)) {
        return refuse(r, "begins with '%s', and a bitmask is an unsigned integer");
    }
    out->base = given[i].kind;
    out->size = out->align = given[i].size;
    next(r);
    if (!is(r, "{")) return refuse(r, "says '%s' where its brace belongs");
    next(r);
    while (!is(r, "}")) {
        if (!names(r)) return refuse(r, "says '%s' where a constant's name belongs");
        if (out->constant_count == room) {
            schema_constant_t *more = realloc(out->constants, (room ? room * 2 : 8) * sizeof *out->constants);
            if (!more) return refuse(r, "ran out of memory at '%s'");
            out->constants = more;
            room = room ? room * 2 : 8;
        }
        out->constants[out->constant_count ++].name = kept(r->word);
        if (!out->constants[out->constant_count - 1].name) return refuse(r, "ran out of memory at '%s'");
        next(r);
        if (is(r, "=")) {
            next(r);
            if (!integer(r, &value, &below) || !fits(out->base, value, below)) {
                return refuse(r, "gives a constant the value '%s', which the integer it is cannot hold");
            }
            next(r);
        } else if (bits) {
            return refuse(r, "says '%s' where a bit's value belongs: a bitmask states every one");
        } else {
            value ++;
            below = value < 0;
            if (!fits(out->base, value, below)) {
                return refuse(r, "counts up to a constant before '%s' that the integer it is cannot hold");
            }
        }
        out->constants[out->constant_count - 1].value = value;
        if (is(r, ",")) next(r);
        else if (!is(r, "}")) return refuse(r, "says '%s' where a constant ends");
    }
    if (!out->constant_count) return refuse(r, "has no constant");
    return 0;
}

int schema_parse(enum bridge_kind kind, const char *text, schema_find_t find, void *ctx, schema_t *out,
                 char *why, size_t why_len)
{
    reader_t r = { .at = text, .why = why, .why_len = why_len };
    int refused;
    *out = (schema_t){};
    next(&r);
    if (kind != BRIDGE_STRUCT) refused = constants(&r, kind == BRIDGE_BITMASK, out);
    else if (!is(&r, "{")) refused = refuse(&r, "begins with '%s' and not with a brace");
    else refused = a_struct(&r, find, ctx, out);
    if (!refused) {
        next(&r);
        if (r.word[0]) refused = refuse(&r, "goes on after its closing brace, with '%s'");
    }
    if (refused) schema_free(out);
    return refused;
}

void schema_free(schema_t *s)
{
    for (uint32_t i = 0; i < s->field_count; i ++) free((char *)s->fields[i].name);
    for (uint32_t i = 0; i < s->constant_count; i ++) free(s->constants[i].name);
    free(s->fields);
    free(s->constants);
    *s = (schema_t){};
}
