/* A type from its text. docs/modules/schema.md, *The type set*.
 *
 * The text is what a module's compiler made of the body of its ECS_STRUCT, ECS_ENUM or
 * ECS_BITMASK: tokens with one space between them. What it may say is written out here in
 * full, and nothing else is accepted:
 *
 *     a struct      { field field ... }
 *     a field       type name ;   or   type name [ count ] ;
 *     a type        bool char int8_t int16_t int32_t int64_t uint8_t uint16_t uint32_t
 *                   uint64_t float double bridge_text, or a type its namespace declared
 *     an enum       { name , name = integer , ... }     counted from zero, or from the last
 *     a bitmask     { name = integer , ... }            every bit stated
 *
 * It lays a struct out as C does on every target a module is built for: each field at the
 * next multiple of its alignment, the whole a multiple of its widest. An enum is four bytes
 * signed and a bitmask four unsigned, which is what the binding headers state of them. */
#include "schema.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const struct { const char *name; enum rows_kind kind; uint32_t size; } given[] = {
    { "bool", ROWS_BOOL, 1 },       { "char", ROWS_CHAR, 1 },
    { "int8_t", ROWS_I8, 1 },       { "uint8_t", ROWS_U8, 1 },
    { "int16_t", ROWS_I16, 2 },     { "uint16_t", ROWS_U16, 2 },
    { "int32_t", ROWS_I32, 4 },     { "uint32_t", ROWS_U32, 4 },
    { "int64_t", ROWS_I64, 8 },     { "uint64_t", ROWS_U64, 8 },
    { "float", ROWS_F32, 4 },       { "double", ROWS_F64, 8 },
};

typedef struct {
    const char *at;
    char        word[64];           /* the token just read: a name, a number, or one mark */
    bool        cut;                /* it was longer than any name may be */
    char       *why;
    size_t      why_len;
} reader;

static bool is_name(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
}

/* the next token into r->word: a run of name characters, or a single other character. Empty
 * at the end of the text. A run too long for any name is cut, and will be refused as a name. */
static void next(reader *r)
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

static int refuse(reader *r, const char *what)
{
    snprintf(r->why, r->why_len, what, r->word[0] ? r->word : "the end of it");
    return 1;
}

static bool is(const reader *r, const char *word)
{
    return !strcmp(r->word, word);
}

/* a name a field or a constant may have: it begins with a letter or an underscore */
static bool names(const reader *r)
{
    char c = r->word[0];
    return !r->cut && ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_');
}

/* the token as an integer: decimal or 0x, signed. false if it is not one */
static bool integer(const reader *r, int64_t *value)
{
    char *end;
    if (!r->word[0] || (!(r->word[0] >= '0' && r->word[0] <= '9') && r->word[0] != '-')) return false;
    *value = strtoll(r->word, &end, 0);
    return !*end && end != r->word;
}

static char *kept(const char *s)
{
    size_t n = strlen(s) + 1;
    char *c = malloc(n);
    memcpy(c, s, n);
    return c;
}

static int a_struct(reader *r, schema_find find, void *ctx, schema *out)
{
    uint32_t room = 0;
    for (next(r); !is(r, "}"); next(r)) {
        rows_field f = { .count = 1 };
        uint32_t size = 0, align = 0, i;
        schema_named named;

        for (i = 0; i < sizeof given / sizeof *given && !is(r, given[i].name); i ++) {}
        if (i < sizeof given / sizeof *given) {
            f.kind = given[i].kind;
            size = align = given[i].size;
        } else if (is(r, "bridge_text")) {
            /* given and not declared: (offset, length) into the persistent arena */
            f.kind = ROWS_TEXT;
            size = sizeof(bridge_text);
            align = alignof(bridge_text);
        } else if (names(r) && find(ctx, r->word, &named)) {
            f.kind = named.kind == BRIDGE_STRUCT ? ROWS_STRUCT
                   : named.kind == BRIDGE_ENUM ? ROWS_ENUM : ROWS_BITMASK;
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
            room = room ? room * 2 : 8;
            out->fields = realloc(out->fields, room * sizeof *out->fields);
        }
        f.name = kept(r->word);
        out->fields[out->field_count ++] = f;

        next(r);
        if (is(r, "[")) {
            int64_t count;
            next(r);
            if (!integer(r, &count) || count < 1 || count > UINT32_MAX / size) {
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

static int constants(reader *r, bool bits, schema *out)
{
    uint32_t room = 0;
    int64_t value = -1;
    next(r);
    while (!is(r, "}")) {
        if (!names(r)) return refuse(r, "says '%s' where a constant's name belongs");
        if (out->constant_count == room) {
            room = room ? room * 2 : 8;
            out->constants = realloc(out->constants, room * sizeof *out->constants);
        }
        out->constants[out->constant_count ++].name = kept(r->word);
        next(r);
        if (is(r, "=")) {
            next(r);
            if (!integer(r, &value) || value < (bits ? 0 : INT32_MIN) || value > (bits ? UINT32_MAX : INT32_MAX)) {
                return refuse(r, "gives a constant the value '%s'");
            }
            next(r);
        } else if (bits) {
            return refuse(r, "says '%s' where a bit's value belongs: a bitmask states every one");
        } else {
            value ++;
        }
        out->constants[out->constant_count - 1].value = (int32_t)(uint32_t)value;
        if (is(r, ",")) next(r);
        else if (!is(r, "}")) return refuse(r, "says '%s' where a constant ends");
    }
    if (!out->constant_count) return refuse(r, "has no constant");
    out->size = out->align = 4;
    return 0;
}

int schema_parse(enum bridge_kind kind, const char *text, schema_find find, void *ctx, schema *out,
                 char *why, size_t why_len)
{
    reader r = { .at = text, .why = why, .why_len = why_len };
    int refused;
    *out = (schema){};
    next(&r);
    if (!is(&r, "{")) refused = refuse(&r, "begins with '%s' and not with a brace");
    else if (kind == BRIDGE_STRUCT) refused = a_struct(&r, find, ctx, out);
    else refused = constants(&r, kind == BRIDGE_BITMASK, out);
    if (!refused) {
        next(&r);
        if (r.word[0]) refused = refuse(&r, "goes on after its closing brace, with '%s'");
    }
    if (refused) schema_free(out);
    return refused;
}

void schema_free(schema *s)
{
    for (uint32_t i = 0; i < s->field_count; i ++) free((char *)s->fields[i].name);
    for (uint32_t i = 0; i < s->constant_count; i ++) free(s->constants[i].name);
    free(s->fields);
    free(s->constants);
    *s = (schema){};
}
