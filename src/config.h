/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_CONFIG_H
    #define BRIDGE_SRC_CONFIG_H

    #include "inside.h"

typedef struct {
    bridge_t *b;
    module_t *m;
    const bridge_module_config_t *cfg;
    declared_type_t *c;
    uint8_t *row;
    uint32_t text_used;
    bool writing;
    char *why;
    size_t why_len;
} config_seed_t;

typedef int (*config_seeder_fn_t)(config_seed_t *seed,
    const schema_field_t *field, const bridge_setting_t *s);

typedef struct {
    int64_t least;
    int64_t most;
    uint32_t size;
} config_integer_t;

[[nodiscard]] int config_seed(module_t *m, const bridge_module_config_t *cfg,
    backend_err_t *err, bool writing);

void config_keep(bridge_t *b, module_t *m);

void config_give_back_free(bridge_t *b);

kept_text_t *config_find_kept(bridge_t *b, const char *stem);

[[gnu::format(printf, 2, 3)]] int config_refuse(config_seed_t *seed,
    const char *format, ...);

int config_say_misfit(config_seed_t *seed, const bridge_setting_t *s,
    const char *field);

int config_place_text(config_seed_t *seed);

int config_seed_setting(config_seed_t *seed, const bridge_setting_t *s);

int config_seed_bool(config_seed_t *seed, const schema_field_t *field,
    const bridge_setting_t *s);
int config_seed_float(config_seed_t *seed, const schema_field_t *field,
    const bridge_setting_t *s);
int config_seed_integer(config_seed_t *seed, const schema_field_t *field,
    const bridge_setting_t *s);
int config_seed_enum(config_seed_t *seed, const schema_field_t *field,
    const bridge_setting_t *s);
int config_seed_bitmask(config_seed_t *seed, const schema_field_t *field,
    const bridge_setting_t *s);
int config_seed_text(config_seed_t *seed, const schema_field_t *field,
    const bridge_setting_t *s);
int config_seed_struct(config_seed_t *seed, const schema_field_t *field,
    const bridge_setting_t *s);
int config_seed_array(config_seed_t *seed, const schema_field_t *field,
    const bridge_setting_t *s);

#endif
