/* A type from its text: what a descriptor may say, and what it lays out to.
 * docs/modules/schema.md, *The type set*. It reads text and answers a layout: who keeps the rows
 * is not its business, and storage never sees a field. */
#ifndef BRIDGE_INSIDE_SCHEMA_H
#define BRIDGE_INSIDE_SCHEMA_H

#include "bridge-abi.h"

#include <stddef.h>

/* what one field of a struct holds. Zero is none of them. */
enum schema_kind : uint8_t {
    SCHEMA_BOOL = 1, SCHEMA_CHAR, SCHEMA_I8, SCHEMA_U8, SCHEMA_I16, SCHEMA_U16, SCHEMA_I32,
    SCHEMA_U32, SCHEMA_I64, SCHEMA_U64, SCHEMA_F32, SCHEMA_F64, SCHEMA_ENUM, SCHEMA_BITMASK,
    SCHEMA_TEXT, SCHEMA_STRUCT
};

typedef struct {
    const char      *name;
    uint32_t         offset;
    uint32_t         count;     /* 1, or how many of them an inline array holds */
    enum schema_kind kind;
    void            *type;      /* the declared type an enum, a bitmask or a struct field names:
                                 * whatever `find` answered for it */
} schema_field_t;

/* a type declared before this one, in the same namespace, as a field may name it */
typedef struct {
    enum bridge_kind kind;
    uint32_t         size, align;
    void            *type;
} schema_named_t;

/* false if the namespace has declared nothing of that name */
typedef bool (*schema_find_t)(void *ctx, const char *name, schema_named_t *named);

/* a bitmask of 64 bits has a constant past INT64_MAX: its bits are kept, as they are */
typedef struct { char *name; int64_t value; } schema_constant_t;

typedef struct {
    uint32_t         size, align;
    enum schema_kind base;              /* the integer an enum or a bitmask is: its author's */
    schema_field_t    *fields;            /* a struct's, in the order it states them */
    uint32_t         field_count;
    schema_constant_t *constants;         /* an enum's or a bitmask's */
    uint32_t         constant_count;
} schema_t;

/* 0 and `out`, or 1 and `why`: what the text says that a descriptor may not, as the rest of a
 * sentence that begins "its descriptor". Nothing is left allocated when it is refused. */
[[nodiscard]] int schema_parse(enum bridge_kind kind, const char *text, schema_find_t find, void *ctx,
                               schema_t *out, char *why, size_t why_len);
void schema_free(schema_t *s);

#endif
