/* What a descriptor may say, word by word: docs/modules/schema.md, *The type set*.
 *
 * Every spelling the grammar accepts is read here and its layout checked against what this
 * compiler lays the same struct out as; every spelling it does not accept is refused, and the
 * refusal has to name the word. From inside: it calls the parser and nothing else. */
#include "schema.h"

#include <stdio.h>
#include <string.h>

/* a namespace that has declared three types already */
static bool find([[maybe_unused]] void *ctx, const char *name, schema_named_t *named)
{
    if (!strcmp(name, "Vec2")) *named = (schema_named_t){ BRIDGE_STRUCT, 8, 4, nullptr };
    else if (!strcmp(name, "Team")) *named = (schema_named_t){ BRIDGE_ENUM, 4, 4, nullptr };
    else if (!strcmp(name, "Flags")) *named = (schema_named_t){ BRIDGE_BITMASK, 4, 4, nullptr };
    else return false;
    return true;
}

static int failed;

static void fail(const char *text, const char *what)
{
    printf("FAIL: %s: %s\n", text, what);
    failed ++;
}

/* the same structs, as this compiler lays them out */
typedef struct { float x; float y; } Vec2;
typedef enum Team : int32_t { Red } Team;
typedef enum Flags : uint32_t { Solid = 1 } Flags;
typedef struct { bool vsync; int32_t width; float scale; Team mode; char title[16]; bridge_text_t path;
                 uint8_t small; Flags flags; double ratio; int64_t big; } Settings;
typedef struct { uint64_t ids[5]; } Roster;
typedef struct { Vec2 at; Team team; Flags flags; float mass; } Body;
typedef struct { uint8_t a; uint16_t b; uint8_t c; uint64_t d; int8_t e; int16_t f; uint32_t g; } Mixed;

#define FIELD(T, f, k, n) { #f, (uint32_t)offsetof(T, f), n, k }
typedef struct { const char *name; uint32_t offset, count; enum schema_kind kind; } want_field_t;

static void a_struct(const char *text, uint32_t size, uint32_t align, const want_field_t *want, uint32_t count)
{
    schema_t s;
    char why[160];
    if (schema_parse(&(schema_source_t){ BRIDGE_STRUCT, text, find, nullptr, why, sizeof why }, &s)) { fail(text, why); return; }
    if (s.size != size || s.align != align || s.field_count != count) fail(text, "its size, alignment or field count");
    for (uint32_t i = 0; i < count && i < s.field_count; i ++) {
        if (strcmp(s.fields[i].name, want[i].name) || s.fields[i].offset != want[i].offset
            || s.fields[i].count != want[i].count || s.fields[i].kind != want[i].kind) {
            fail(text, want[i].name);
        }
    }
    schema_free(&s);
}

static void constants(enum bridge_kind kind, const char *text, uint32_t size, const int64_t *want,
                      uint32_t count)
{
    schema_t s;
    char why[160];
    if (schema_parse(&(schema_source_t){ kind, text, find, nullptr, why, sizeof why }, &s)) { fail(text, why); return; }
    if (s.size != size || s.align != size || s.constant_count != count) fail(text, "its size or how many constants");
    for (uint32_t i = 0; i < count && i < s.constant_count; i ++) {
        if (s.constants[i].value != want[i]) fail(text, s.constants[i].name);
    }
    schema_free(&s);
}

/* refused, and the refusal says the word it stopped at */
static void refused(enum bridge_kind kind, const char *text, const char *word)
{
    schema_t s;
    char why[160] = "";
    if (!schema_parse(&(schema_source_t){ kind, text, find, nullptr, why, sizeof why }, &s)) {
        schema_free(&s);
        fail(text, "was accepted");
        return;
    }
    if (!strstr(why, word)) {
        printf("FAIL: %s: refused without naming '%s': its descriptor %s\n", text, word, why);
        failed ++;
    }
}

int main()
{
    static const want_field_t vec2[] = { FIELD(Vec2, x, SCHEMA_F32, 1), FIELD(Vec2, y, SCHEMA_F32, 1) };
    static const want_field_t settings[] = {
        FIELD(Settings, vsync, SCHEMA_BOOL, 1), FIELD(Settings, width, SCHEMA_I32, 1),
        FIELD(Settings, scale, SCHEMA_F32, 1), FIELD(Settings, mode, SCHEMA_ENUM, 1),
        FIELD(Settings, title, SCHEMA_CHAR, 16), FIELD(Settings, path, SCHEMA_TEXT, 1),
        FIELD(Settings, small, SCHEMA_U8, 1), FIELD(Settings, flags, SCHEMA_BITMASK, 1),
        FIELD(Settings, ratio, SCHEMA_F64, 1), FIELD(Settings, big, SCHEMA_I64, 1),
    };
    static const want_field_t roster[] = { FIELD(Roster, ids, SCHEMA_U64, 5) };
    static const want_field_t body[] = {
        FIELD(Body, at, SCHEMA_STRUCT, 1), FIELD(Body, team, SCHEMA_ENUM, 1),
        FIELD(Body, flags, SCHEMA_BITMASK, 1), FIELD(Body, mass, SCHEMA_F32, 1),
    };
    static const want_field_t mixed[] = {
        FIELD(Mixed, a, SCHEMA_U8, 1), FIELD(Mixed, b, SCHEMA_U16, 1), FIELD(Mixed, c, SCHEMA_U8, 1),
        FIELD(Mixed, d, SCHEMA_U64, 1), FIELD(Mixed, e, SCHEMA_I8, 1), FIELD(Mixed, f, SCHEMA_I16, 1),
        FIELD(Mixed, g, SCHEMA_U32, 1),
    };

    a_struct("{ float x; float y; }", sizeof(Vec2), alignof(Vec2), vec2, 2);
    a_struct("{ bool vsync; int32_t width; float scale; Team mode; char title[16]; bridge_text_t path; "
             "uint8_t small; Flags flags; double ratio; int64_t big; }",
             sizeof(Settings), alignof(Settings), settings, 10);
    a_struct("{ uint64_t ids[5]; }", sizeof(Roster), alignof(Roster), roster, 1);
    a_struct("{ Vec2 at; Team team; Flags flags; float mass; }", sizeof(Body), alignof(Body), body, 4);
    a_struct("{ uint8_t a; uint16_t b; uint8_t c; uint64_t d; int8_t e; int16_t f; uint32_t g; }",
             sizeof(Mixed), alignof(Mixed), mixed, 7);
    a_struct("{float x;float y;}", sizeof(Vec2), alignof(Vec2), vec2, 2);    /* no spaces at all */

    constants(BRIDGE_ENUM, "int32_t { Off, On, Adaptive }", 4, (int64_t[]){ 0, 1, 2 }, 3);
    constants(BRIDGE_ENUM, "int32_t { A = 5, B, C = -1, D, }", 4, (int64_t[]){ 5, 6, -1, 0 }, 4);
    constants(BRIDGE_ENUM, "int32_t { X = 0x10 }", 4, (int64_t[]){ 16 }, 1);
    constants(BRIDGE_BITMASK, "uint32_t { Loud = 1, Wide = 2 }", 4, (int64_t[]){ 1, 2 }, 2);
    constants(BRIDGE_BITMASK, "uint32_t { Top = 0x80000000 }", 4, (int64_t[]){ 2147483648 }, 1);
    /* the integer its author stated is its width, and its range */
    constants(BRIDGE_ENUM, "int8_t { Low = -128, High = 127 }", 1, (int64_t[]){ -128, 127 }, 2);
    constants(BRIDGE_ENUM, "uint8_t { A = 254, B }", 1, (int64_t[]){ 254, 255 }, 2);
    constants(BRIDGE_ENUM, "uint16_t { A = 65535 }", 2, (int64_t[]){ 65535 }, 1);
    constants(BRIDGE_BITMASK, "uint8_t { Top = 0x80 }", 1, (int64_t[]){ 128 }, 1);
    constants(BRIDGE_BITMASK, "uint64_t { Top = 0x8000000000000000 }", 8, (int64_t[]){ INT64_MIN }, 1);

    /* what a looser parser would take and this one does not, and what was never C */
    refused(BRIDGE_STRUCT, "{ unsigned x; }", "'unsigned'");
    refused(BRIDGE_STRUCT, "{ _Bool b; }", "'_Bool'");
    refused(BRIDGE_STRUCT, "{ ecs_entity_t e; }", "'ecs_entity_t'");
    refused(BRIDGE_STRUCT, "{ const float x; }", "'const'");
    refused(BRIDGE_STRUCT, "{ Nope n; }", "'Nope'");
    refused(BRIDGE_STRUCT, "{ float *p; }", "'*'");
    refused(BRIDGE_STRUCT, "{ char* s; }", "'*'");
    refused(BRIDGE_STRUCT, "{ float x, y; }", "','");
    refused(BRIDGE_STRUCT, "{ float m[2][2]; }", "'['");
    refused(BRIDGE_STRUCT, "{ float x[0]; }", "'0'");
    refused(BRIDGE_STRUCT, "{ float x[]; }", "']'");
    refused(BRIDGE_STRUCT, "{ float x[n]; }", "'n'");
    refused(BRIDGE_STRUCT, "{ float; }", "';'");
    refused(BRIDGE_STRUCT, "{ float 9x; }", "'9x'");
    refused(BRIDGE_STRUCT, "{ float x }", "'}'");
    refused(BRIDGE_STRUCT, "{ }", "no field");
    refused(BRIDGE_STRUCT, "float x;", "'float'");
    refused(BRIDGE_STRUCT, "{ float x; } junk", "'junk'");
    refused(BRIDGE_STRUCT, "{ float x;", "the end of it");
    refused(BRIDGE_STRUCT, "{ float a_name_that_goes_on_for_longer_than_any_field_of_a_struct_is_called; }", "'a_name");
    refused(BRIDGE_ENUM, "int32_t { }", "no constant");
    refused(BRIDGE_ENUM, "int32_t { A = }", "'}'");
    refused(BRIDGE_ENUM, "int32_t { A B }", "'B'");
    refused(BRIDGE_ENUM, "int32_t { A = 1 << 2 }", "'<'");
    refused(BRIDGE_ENUM, "int32_t { A = 4294967296 }", "'4294967296'");
    refused(BRIDGE_BITMASK, "uint32_t { A }", "'}'");
    refused(BRIDGE_BITMASK, "uint32_t { A = -1 }", "'-1'");
    /* and with no integer stated, or one it cannot be, or a constant past it */
    refused(BRIDGE_ENUM, "{ A }", "'{'");
    refused(BRIDGE_ENUM, "float { A }", "'float'");
    refused(BRIDGE_ENUM, "int64_t { A }", "'int64_t'");
    refused(BRIDGE_ENUM, "uint64_t { A }", "'uint64_t'");
    refused(BRIDGE_BITMASK, "int8_t { A = 1 }", "'int8_t'");
    refused(BRIDGE_ENUM, "int8_t { A = 128 }", "'128'");
    refused(BRIDGE_ENUM, "uint8_t { A = -1 }", "'-1'");
    refused(BRIDGE_ENUM, "uint8_t { A = 255, B }", "'}'");
    refused(BRIDGE_BITMASK, "uint8_t { A = 256 }", "'256'");

    printf("%s\n", failed ? "FAIL" : "every descriptor read as it should, and every other refused by its word");
    printf("alive\n");
    return failed != 0;
}
