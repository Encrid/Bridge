/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/schema.h -- a type from its text: what a descriptor may say, and what
** it lays out to.
**
** It reads text and answers a layout: who keeps the rows is not its business,
** and storage never sees a field.
**
** schema.md, *The type set*
*/

#ifndef BRIDGE_SRC_SCHEMA_H
    #define BRIDGE_SRC_SCHEMA_H

    #include <stddef.h>

    #include "bridge_abi.h"

/*
** What one field of a struct holds. Zero is none of them.
*/
enum schema_kind : uint8_t {
    SCHEMA_BOOL = 1,
    SCHEMA_CHAR,
    SCHEMA_I8,
    SCHEMA_U8,
    SCHEMA_I16,
    SCHEMA_U16,
    SCHEMA_I32,
    SCHEMA_U32,
    SCHEMA_I64,
    SCHEMA_U64,
    SCHEMA_F32,
    SCHEMA_F64,
    SCHEMA_ENUM,
    SCHEMA_BITMASK,
    SCHEMA_TEXT,
    SCHEMA_STRUCT
};

/*
** A field of a struct. `count` is 1, or how many of them an inline array
** holds. `type` is the declared type an enum, a bitmask or a struct field
** names: whatever `find` answered for it.
*/
typedef struct {
    const char *name;
    uint32_t offset;
    uint32_t count;
    enum schema_kind kind;
    void *type;
} schema_field_t;

/*
** A type declared before this one, in the same namespace, as a field may name
** it.
*/
typedef struct {
    enum bridge_kind kind;
    uint32_t size;
    uint32_t align;
    void *type;
} schema_named_t;

/*
** Answers false if the namespace has declared nothing of that name.
*/
typedef bool (*schema_find_t)(void *ctx, const char *name,
    schema_named_t *named);

/*
** A constant of an enum or a bitmask. A bitmask of 64 bits has a constant
** past INT64_MAX: its bits are kept, as they are.
*/
typedef struct {
    char *name;
    int64_t value;
} schema_constant_t;

/*
** A type, laid out. `base` is the integer an enum or a bitmask is: its
** author's. `fields` are a struct's, in the order it states them, and
** `constants` an enum's or a bitmask's.
*/
typedef struct {
    uint32_t size;
    uint32_t align;
    enum schema_kind base;
    schema_field_t *fields;
    uint32_t field_count;
    schema_constant_t *constants;
    uint32_t constant_count;
} schema_t;

/*
** What schema_parse reads: the text, what kind of type it describes, how a
** name its fields use is found, and where a refusal is written.
*/
typedef struct {
    enum bridge_kind kind;
    const char *text;
    schema_find_t find;
    void *ctx;
    char *why;
    size_t why_len;
} schema_source_t;

/*
** schema_parse
**
** Lays a type out from its text. 0 and `out`, or 1 and the source's `why`:
** what the text says that a descriptor may not, as the rest of a sentence
** that begins "its descriptor". Nothing is left allocated when it is refused.
*/
[[nodiscard]] int schema_parse(const schema_source_t *source, schema_t *out);

/*
** schema_free
**
** Frees what a parsed type holds, and leaves it empty.
*/
void schema_free(schema_t *s);

#endif
