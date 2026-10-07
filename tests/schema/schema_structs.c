/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/schema/schema_structs.c -- the structs a descriptor is read as,
** each beside what this compiler lays it out as. The last is the first
** again, with no spaces at all.
*/

#include "schema_test.h"

static const want_field_t SCHEMA_VEC2[] = {
    SCHEMA_FIELD(vec2_t, x, SCHEMA_F32, 1),
    SCHEMA_FIELD(vec2_t, y, SCHEMA_F32, 1)
};

static const want_field_t SCHEMA_SETTINGS[] = {
    SCHEMA_FIELD(settings_t, vsync, SCHEMA_BOOL, 1),
    SCHEMA_FIELD(settings_t, width, SCHEMA_I32, 1),
    SCHEMA_FIELD(settings_t, scale, SCHEMA_F32, 1),
    SCHEMA_FIELD(settings_t, mode, SCHEMA_ENUM, 1),
    SCHEMA_FIELD(settings_t, title, SCHEMA_CHAR, 16),
    SCHEMA_FIELD(settings_t, path, SCHEMA_TEXT, 1),
    SCHEMA_FIELD(settings_t, small, SCHEMA_U8, 1),
    SCHEMA_FIELD(settings_t, flags, SCHEMA_BITMASK, 1),
    SCHEMA_FIELD(settings_t, ratio, SCHEMA_F64, 1),
    SCHEMA_FIELD(settings_t, big, SCHEMA_I64, 1)
};

static const want_field_t SCHEMA_ROSTER[] = {
    SCHEMA_FIELD(roster_t, ids, SCHEMA_U64, 5)
};

static const want_field_t SCHEMA_BODY[] = {
    SCHEMA_FIELD(body_t, at, SCHEMA_STRUCT, 1),
    SCHEMA_FIELD(body_t, team, SCHEMA_ENUM, 1),
    SCHEMA_FIELD(body_t, flags, SCHEMA_BITMASK, 1),
    SCHEMA_FIELD(body_t, mass, SCHEMA_F32, 1)
};

static const want_field_t SCHEMA_MIXED[] = {
    SCHEMA_FIELD(mixed_t, a, SCHEMA_U8, 1),
    SCHEMA_FIELD(mixed_t, b, SCHEMA_U16, 1),
    SCHEMA_FIELD(mixed_t, c, SCHEMA_U8, 1),
    SCHEMA_FIELD(mixed_t, d, SCHEMA_U64, 1),
    SCHEMA_FIELD(mixed_t, e, SCHEMA_I8, 1),
    SCHEMA_FIELD(mixed_t, f, SCHEMA_I16, 1),
    SCHEMA_FIELD(mixed_t, g, SCHEMA_U32, 1)
};

static const want_struct_t SCHEMA_STRUCTS[] = {
    { "{ float x; float y; }", sizeof(vec2_t), alignof(vec2_t), SCHEMA_VEC2,
        2 },
    { "{ bool vsync; int32_t width; float scale; Team mode; char title[16]; "
        "bridge_text_t path; uint8_t small; Flags flags; double ratio; "
        "int64_t big; }", sizeof(settings_t), alignof(settings_t),
        SCHEMA_SETTINGS, 10 },
    { "{ uint64_t ids[5]; }", sizeof(roster_t), alignof(roster_t),
        SCHEMA_ROSTER, 1 },
    { "{ Vec2 at; Team team; Flags flags; float mass; }", sizeof(body_t),
        alignof(body_t), SCHEMA_BODY, 4 },
    { "{ uint8_t a; uint16_t b; uint8_t c; uint64_t d; int8_t e; int16_t f; "
        "uint32_t g; }", sizeof(mixed_t), alignof(mixed_t), SCHEMA_MIXED, 7 },
    { "{float x;float y;}", sizeof(vec2_t), alignof(vec2_t), SCHEMA_VEC2, 2 }
};

/*
** schema_run_structs
**
** Checks every struct there is a row for.
*/
int schema_run_structs(void)
{
    constexpr size_t count = sizeof(SCHEMA_STRUCTS) / sizeof(*SCHEMA_STRUCTS);
    int failed = 0;
    size_t i = 0;

    while (i < count) {
        failed += schema_check_struct(&SCHEMA_STRUCTS[i]);
        i++;
    }
    return (failed);
}
