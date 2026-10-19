/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_HOST_TOML_H
    #define BRIDGE_HOST_TOML_H

    #include <stddef.h>
    #include <stdint.h>

enum value_kind : uint8_t {
    VALUE_TEXT = 1,
    VALUE_WHOLE,
    VALUE_REAL,
    VALUE_FLAG,
    VALUE_LIST
};

typedef struct value value_t;

struct value {
    enum value_kind kind;
    bool flag;
    int64_t whole;
    double real;
    char *text;
    value_t *items;
    uint32_t count;
};

typedef struct {
    char **key;
    uint32_t depth;
    uint32_t line;
    value_t value;
} entry_t;

typedef struct {
    entry_t *entries;
    uint32_t count;
    uint32_t line;
    const char *why;
} toml_t;

typedef struct {
    const char *at;
    uint32_t line;
    uint32_t start;
    char **table;
    uint32_t table_depth;
    toml_t *into;
} reading_t;

[[nodiscard]] bool toml_read(const char *text, toml_t *t);

bool toml_refuse(reading_t *r, const char *why);

void toml_skip(reading_t *r);

[[nodiscard]] bool toml_read_pair(reading_t *r);

[[nodiscard]] bool toml_read_inline(reading_t *r, char **key,
    uint32_t depth);

[[nodiscard]] bool toml_read_key(reading_t *r, char ***key, uint32_t *depth);

[[nodiscard]] bool toml_read_value(reading_t *r, value_t *v);

[[nodiscard]] char *toml_read_text(reading_t *r);

[[nodiscard]] bool toml_read_number(reading_t *r, value_t *v);

[[nodiscard]] bool toml_add(reading_t *r, char **key, uint32_t depth,
    const value_t *v);

[[nodiscard]] bool toml_has_prefix(const entry_t *e, const char *const *names,
    uint32_t depth);

[[nodiscard]] bool toml_is_same(const value_t *a, const value_t *b);

void toml_say(const value_t *v, char *out, size_t size);

#endif
