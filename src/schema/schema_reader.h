/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_SCHEMA_READER_H
    #define BRIDGE_SRC_SCHEMA_READER_H

    #include "schema.h"

typedef struct {
    const char *name;
    enum schema_kind kind;
    uint32_t size;
} schema_given_t;

constexpr uint32_t SCHEMA_GIVEN_COUNT = 12;
static const schema_given_t SCHEMA_GIVEN[SCHEMA_GIVEN_COUNT] = {
    { "bool", SCHEMA_BOOL, 1 },
    { "char", SCHEMA_CHAR, 1 },
    { "int8_t", SCHEMA_I8, 1 },
    { "uint8_t", SCHEMA_U8, 1 },
    { "int16_t", SCHEMA_I16, 2 },
    { "uint16_t", SCHEMA_U16, 2 },
    { "int32_t", SCHEMA_I32, 4 },
    { "uint32_t", SCHEMA_U32, 4 },
    { "int64_t", SCHEMA_I64, 8 },
    { "uint64_t", SCHEMA_U64, 8 },
    { "float", SCHEMA_F32, 4 },
    { "double", SCHEMA_F64, 8 }
};

typedef struct {
    const char *at;
    char word[64];
    bool cut;
    const schema_source_t *source;
    schema_t *out;
    int64_t value;
    bool below;
} schema_reader_t;

typedef struct {
    schema_field_t field;
    uint32_t size;
    uint32_t align;
} schema_laid_t;

void schema_read_next(schema_reader_t *r);

int schema_refuse(schema_reader_t *r, const char *what);

bool schema_is(const schema_reader_t *r, const char *word);

bool schema_is_name(const schema_reader_t *r);

bool schema_get_integer(const schema_reader_t *r, int64_t *value, bool *below);

int schema_find_given(const schema_reader_t *r);

char *schema_copy_text(const char *text);

void *schema_take_room(const schema_reader_t *r, char mark, size_t size);

bool schema_fits(enum schema_kind base, int64_t value, bool below);

int schema_set_value(schema_reader_t *r, bool bits);

int schema_set_type(schema_reader_t *r, schema_laid_t *laid);

int schema_parse_struct(schema_reader_t *r);

int schema_parse_constants(schema_reader_t *r, bool bits);

#endif
