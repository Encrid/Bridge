/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_SCHEMA_H
    #define BRIDGE_SRC_SCHEMA_H

    #include <stddef.h>

    #include "bridge_abi.h"

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

typedef struct {
    const char *name;
    uint32_t offset;
    uint32_t count;
    enum schema_kind kind;
    void *type;
} schema_field_t;

typedef struct {
    enum bridge_kind kind;
    uint32_t size;
    uint32_t align;
    void *type;
} schema_named_t;

typedef bool (*schema_find_t)(void *ctx, const char *name,
    schema_named_t *named);

typedef struct {
    char *name;
    int64_t value;
} schema_constant_t;

typedef struct {
    uint32_t size;
    uint32_t align;
    enum schema_kind base;
    schema_field_t *fields;
    uint32_t field_count;
    schema_constant_t *constants;
    uint32_t constant_count;
} schema_t;

typedef struct {
    enum bridge_kind kind;
    const char *text;
    schema_find_t find;
    void *ctx;
    char *why;
    size_t why_len;
} schema_source_t;

[[nodiscard]] int schema_parse(const schema_source_t *source, schema_t *out);

void schema_free(schema_t *s);

#endif
