/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_HOST_CONFIG_H
    #define BRIDGE_HOST_CONFIG_H

    #include <stddef.h>
    #include <stdint.h>

    #include "toml.h"

typedef struct {
    char *path;
    char *folder;
    char *whole;
    toml_t toml;
    uint32_t *named;
    uint32_t named_count;
    int32_t by;
    uint32_t by_line;
    bool reading;
} cfile_t;

typedef struct {
    cfile_t *files;
    uint32_t count;
    int32_t local;
    uint32_t refused;
} config_t;

typedef struct {
    const value_t *value;
    uint32_t file;
    uint32_t line;
} said_t;

typedef struct {
    said_t at;
    said_t other;
} found_t;

typedef struct {
    const char *const *names;
    uint32_t depth;
} keyed_t;

typedef struct {
    const char *tables[3];
    keyed_t key;
} module_key_t;

typedef struct {
    const char **names;
    uint32_t count;
} names_t;

typedef bool states_fn(const cfile_t *f, const void *asked, said_t *said);

typedef struct {
    const char *prefix;
    states_fn *states;
    const void *asked;
} shown_t;

enum check : uint8_t {
    CHECK_FLAG = 1,
    CHECK_COUNT,
    CHECK_LIMIT,
    CHECK_LEVEL,
    CHECK_NAMES,
    CHECK_NAMES_NONE,
    CHECK_ERROR,
    CHECK_ARRIVE,
    CHECK_TEXT,
    CHECK_ROUNDS,
    CHECK_SETTING,
    CHECK_PAIRS,
    CHECK_AUTHOR
};

enum allow : uint8_t {
    ALLOW_FLAG = 1,
    ALLOW_WHOLE = 2,
    ALLOW_REAL = 4,
    ALLOW_WORD = 8,
    ALLOW_TEXT = 16,
    ALLOW_EMPTY = 32,
    ALLOW_NAMES = 64,
    ALLOW_PAIRS = 128
};

typedef struct {
    uint8_t allow;
    int64_t least;
    int64_t most;
    const char *words;
    const char *why;
} value_rule_t;

typedef struct {
    const char *name;
    enum check check;
    uint8_t width;
    size_t at;
    int64_t none;
} module_rule_t;

extern const module_rule_t MODULE_KEYS[];
extern const uint32_t MODULE_KEY_COUNT;

[[nodiscard]] const char *config_go_beside(char *name);

[[nodiscard]] char *config_path(const config_t *c, uint32_t file,
    const char *path);

[[nodiscard]] char *config_get_folder(const char *path);

[[nodiscard]] int32_t config_add_file(config_t *c, char *path, int32_t by,
    uint32_t line);

[[nodiscard]] int32_t config_find_whole(const config_t *c, const char *whole);

void config_say_chain(const config_t *c, int32_t from, int32_t to);

[[nodiscard]] int32_t config_take_file(config_t *c, char *path, int32_t by,
    uint32_t line);

[[nodiscard]] bool config_read(config_t *c, const char *name);

[[nodiscard]] uint32_t config_check(config_t *c);

[[nodiscard]] const char *config_check_world(const entry_t *e, bool local);

[[nodiscard]] const char *config_check_budget(const entry_t *e, bool local);

[[nodiscard]] const char *config_check_checks(const entry_t *e, bool local);

[[nodiscard]] const char *config_check_backend(const entry_t *e, bool local);

[[nodiscard]] const char *config_check_content(const entry_t *e);

[[nodiscard]] const char *config_check_module(const config_t *c,
    const entry_t *e);

[[nodiscard]] const char *config_check_value(const value_t *v,
    enum check check);

[[nodiscard]] found_t config_find(const config_t *c, states_fn *states,
    const void *asked);

[[nodiscard]] said_t config_find_key(const config_t *c,
    const char *const *names, uint32_t depth);

[[nodiscard]] bool config_states_key(const cfile_t *f, const void *asked,
    said_t *said);

[[nodiscard]] bool config_states_module(const cfile_t *f, const void *asked,
    said_t *said);

void names_add(names_t *into, const char *name);

[[nodiscard]] names_t config_names(const config_t *c, const keyed_t *under);

const char *config_say_key(const char *const *names, uint32_t depth,
    char *out, size_t size);

[[nodiscard]] bool config_say_found(const config_t *c, const char *key,
    const found_t *f, const char *prefix);

[[nodiscard]] const char *config_say_where(const config_t *c,
    const said_t *said);

void config_say_over(const config_t *c, states_fn *states,
    const void *asked, const said_t *at);

void config_say_value(const config_t *c, const char *key,
    const found_t *f, const shown_t *how);

#endif
