/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "schema_test.h"

static const want_refusal_t SCHEMA_REFUSALS[] = {
    { BRIDGE_STRUCT, "{ unsigned x; }", "'unsigned'" },
    { BRIDGE_STRUCT, "{ _Bool b; }", "'_Bool'" },
    { BRIDGE_STRUCT, "{ ecs_entity_t e; }", "'ecs_entity_t'" },
    { BRIDGE_STRUCT, "{ const float x; }", "'const'" },
    { BRIDGE_STRUCT, "{ Nope n; }", "'Nope'" },
    { BRIDGE_STRUCT, "{ float *p; }", "'*'" },
    { BRIDGE_STRUCT, "{ char* s; }", "'*'" },
    { BRIDGE_STRUCT, "{ float x, y; }", "','" },
    { BRIDGE_STRUCT, "{ float m[2][2]; }", "'['" },
    { BRIDGE_STRUCT, "{ float x[0]; }", "'0'" },
    { BRIDGE_STRUCT, "{ float x[]; }", "']'" },
    { BRIDGE_STRUCT, "{ float x[n]; }", "'n'" },
    { BRIDGE_STRUCT, "{ float; }", "';'" },
    { BRIDGE_STRUCT, "{ float 9x; }", "'9x'" },
    { BRIDGE_STRUCT, "{ float x }", "'}'" },
    { BRIDGE_STRUCT, "{ }", "no field" },
    { BRIDGE_STRUCT, "float x;", "'float'" },
    { BRIDGE_STRUCT, "{ float x; } junk", "'junk'" },
    { BRIDGE_STRUCT, "{ float x;", "the end of it" },
    { BRIDGE_STRUCT, "{ float a_name_that_goes_on_for_longer_than_any_field_"
        "of_a_struct_is_called; }", "'a_name" },
    { BRIDGE_ENUM, "int32_t { }", "no constant" },
    { BRIDGE_ENUM, "int32_t { A = }", "'}'" },
    { BRIDGE_ENUM, "int32_t { A B }", "'B'" },
    { BRIDGE_ENUM, "int32_t { A = 1 << 2 }", "'<'" },
    { BRIDGE_ENUM, "int32_t { A = 4294967296 }", "'4294967296'" },
    { BRIDGE_BITMASK, "uint32_t { A }", "'}'" },
    { BRIDGE_BITMASK, "uint32_t { A = -1 }", "'-1'" },
    { BRIDGE_ENUM, "{ A }", "'{'" },
    { BRIDGE_ENUM, "float { A }", "'float'" },
    { BRIDGE_ENUM, "int64_t { A }", "'int64_t'" },
    { BRIDGE_ENUM, "uint64_t { A }", "'uint64_t'" },
    { BRIDGE_BITMASK, "int8_t { A = 1 }", "'int8_t'" },
    { BRIDGE_ENUM, "int8_t { A = 128 }", "'128'" },
    { BRIDGE_ENUM, "uint8_t { A = -1 }", "'-1'" },
    { BRIDGE_ENUM, "uint8_t { A = 255, B }", "'}'" },
    { BRIDGE_BITMASK, "uint8_t { A = 256 }", "'256'" },
    { BRIDGE_ENUM, "int32_t { 9 }", "'9'" },
    { BRIDGE_ENUM, "int32_t A }", "'A'" },
    { BRIDGE_STRUCT, "{ float x[2; }", "';'" },
    { BRIDGE_STRUCT, "{ uint64_t a[400000000]; uint64_t b[400000000]; }",
        "more bytes" },
    { BRIDGE_STRUCT, "{ char a[4294967287]; double b; }", "more bytes" },
    { BRIDGE_ENUM, "uint8_t { A = 0x"
        "0000000000000000000000000000000000000000"
        "00000000000000000000000" "1 }", "'0x000" },
    { BRIDGE_BITMASK, "int16_t { A = 1 }", "'int16_t'" },
    { BRIDGE_BITMASK, "int32_t { A = 1 }", "'int32_t'" },
    { BRIDGE_ENUM, "int32_t { A = - 1 }", "'-'" },
    { BRIDGE_STRUCT, "{ float x[-1]; }", "'-1'" },
    { BRIDGE_STRUCT, "{ double x[1000000000]; }", "'1000000000'" },
    { BRIDGE_STRUCT, "{ float ~; }", "'~'" },
    { BRIDGE_STRUCT, "{ float x[2x]; }", "'2x'" },
    { BRIDGE_ENUM, "uint32_t { A = 99999999999999999999 }",
        "'99999999999999999999'" },
    { BRIDGE_ENUM, "uint16_t { A = -1 }", "'-1'" },
    { BRIDGE_ENUM, "uint16_t { A = 65536 }", "'65536'" },
    { BRIDGE_ENUM, "uint32_t { A = 4294967296 }", "'4294967296'" }
};

static int check_refusal(const want_refusal_t *w)
{
    char why[160] = "";
    schema_source_t source = { w->kind, w->text, schema_find_named, nullptr,
        why, sizeof(why) };
    schema_t s;

    if (!schema_parse(&source, &s)) {
        schema_free(&s);
        return (schema_say_failed(w->text, "was accepted"));
    }
    printf("%s: its descriptor %s\n", w->text, why);
    if (strstr(why, w->word))
        return (0);
    printf("FAIL: %s: refused without naming '%s': its descriptor %s\n",
        w->text, w->word, why);
    return (1);
}

int schema_run_refusals(void)
{
    constexpr size_t count = sizeof(SCHEMA_REFUSALS)
        / sizeof(*SCHEMA_REFUSALS);
    int failed = 0;
    size_t i = 0;

    while (i < count) {
        failed += check_refusal(&SCHEMA_REFUSALS[i]);
        i++;
    }
    return (failed);
}
