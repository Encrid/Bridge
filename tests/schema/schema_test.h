/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/schema/schema_test.h -- what a descriptor may say, word by word.
** Every spelling the grammar accepts is read and its layout checked against
** what this compiler lays the same struct out as; every spelling it does
** not accept is refused, and the refusal has to name the word. From inside:
** it calls the parser and nothing else.
**
** schema.md, *The type set*
*/

#ifndef TEST_SCHEMA_TEST_H
    #define TEST_SCHEMA_TEST_H

    #include <stddef.h>

    #include "schema.h"

/*
** A field as this compiler lays it out, by its name in a struct of type T.
*/
    #define SCHEMA_FIELD(T, f, k, n) { #f, (uint32_t)offsetof(T, f), n, k }

/*
** The structs the descriptors describe, as this compiler lays them out.
*/
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

/*
** What a field of a struct is to be read as.
*/
typedef struct {
    const char *name;
    uint32_t offset;
    uint32_t count;
    enum schema_kind kind;
} want_field_t;

/*
** A struct's descriptor, and what it is to be read as.
*/
typedef struct {
    const char *text;
    uint32_t size;
    uint32_t align;
    const want_field_t *want;
    uint32_t count;
} want_struct_t;

/*
** An enum's or a bitmask's descriptor, and what it is to be read as.
*/
typedef struct {
    enum bridge_kind kind;
    const char *text;
    uint32_t size;
    int64_t want[4];
    uint32_t count;
} want_constants_t;

/*
** A descriptor that is to be refused, and the word its refusal names.
*/
typedef struct {
    enum bridge_kind kind;
    const char *text;
    const char *word;
} want_refusal_t;

/*
** schema_find_named
**
** What the reader calls for a type a field names: a namespace that has
** declared three types already, Vec2, Team and Flags.
*/
bool schema_find_named(void *ctx, const char *name, schema_named_t *named);

/*
** schema_say_failed
**
** Says that a descriptor did not read as wanted, and how. Answers 1.
*/
int schema_say_failed(const char *text, const char *what);

/*
** schema_check_struct, schema_check_constants
**
** Read one descriptor and say each way it was not read as wanted. Answer
** how many.
*/
int schema_check_struct(const want_struct_t *w);
int schema_check_constants(const want_constants_t *w);

/*
** schema_run_structs, schema_run_constants, schema_run_refusals
**
** Check every descriptor of a kind there is a row for. Answer how many did
** not read as wanted.
*/
int schema_run_structs(void);
int schema_run_constants(void);
int schema_run_refusals(void);

#endif
