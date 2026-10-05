/* A type from its text: what a descriptor may say, and what it lays out to.
 * docs/modules/schema.md, *The type set*. Storage of our own reads a descriptor with this. */
#ifndef BRIDGE_SCHEMA_H
#define BRIDGE_SCHEMA_H

#include "rows.h"

#include <stddef.h>

/* a type declared before this one, in the same namespace, as a field may name it */
typedef struct {
    enum bridge_kind kind;
    uint32_t         size, align;
    rows_type       *type;
} schema_named;

/* false if the namespace has declared nothing of that name */
typedef bool (*schema_find)(void *ctx, const char *name, schema_named *named);

typedef struct { char *name; int32_t value; } schema_constant;

typedef struct {
    uint32_t         size, align;
    rows_field      *fields;            /* a struct's, in the order it states them */
    uint32_t         field_count;
    schema_constant *constants;         /* an enum's or a bitmask's */
    uint32_t         constant_count;
} schema;

/* 0 and `out`, or 1 and `why`: what the text says that a descriptor may not, as the rest of a
 * sentence that begins "its descriptor". Nothing is left allocated when it is refused. */
[[nodiscard]] int schema_parse(enum bridge_kind kind, const char *text, schema_find find, void *ctx,
                               schema *out, char *why, size_t why_len);
void schema_free(schema *s);

#endif
