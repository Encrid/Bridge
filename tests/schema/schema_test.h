/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_SCHEMA_TEST_H
    #define TEST_SCHEMA_TEST_H

    #include <stddef.h>

    #include "schema.h"

    #define SCHEMA_FIELD(T, f, k, n) { #f, (uint32_t)offsetof(T, f), n, k }

typedef struct {
    float x;
    float y;
} vec2_t;

typedef enum team : int32_t {
    RED
} team_t;

typedef enum flags : uint32_t {
    SOLID = 1
} flags_t;

typedef struct {
    bool vsync;
    int32_t width;
    float scale;
    team_t mode;
    char title[16];
    bridge_text_t path;
    uint8_t small;
    flags_t flags;
    double ratio;
    int64_t big;
} settings_t;

typedef struct {
    uint64_t ids[5];
} roster_t;

typedef struct {
    vec2_t at;
    team_t team;
    flags_t flags;
    float mass;
} body_t;

typedef struct {
    uint8_t a;
    uint16_t b;
    uint8_t c;
    uint64_t d;
    int8_t e;
    int16_t f;
    uint32_t g;
} mixed_t;

typedef struct {
    const char *name;
    uint32_t offset;
    uint32_t count;
    enum schema_kind kind;
} want_field_t;

typedef struct {
    const char *text;
    uint32_t size;
    uint32_t align;
    const want_field_t *want;
    uint32_t count;
} want_struct_t;

typedef struct {
    enum bridge_kind kind;
    const char *text;
    uint32_t size;
    int64_t want[4];
    uint32_t count;
} want_constants_t;

typedef struct {
    enum bridge_kind kind;
    const char *text;
    const char *word;
} want_refusal_t;

bool schema_find_named(void *ctx, const char *name, schema_named_t *named);

int schema_say_failed(const char *text, const char *what);

int schema_check_struct(const want_struct_t *w);
int schema_check_constants(const want_constants_t *w);

int schema_run_structs(void);
int schema_run_constants(void);
int schema_run_refusals(void);

#endif
