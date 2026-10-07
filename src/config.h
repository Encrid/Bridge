/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/config.h -- a module's own settings: one value of the component its
** author named, written after init by what each field is.
**
** config.md, *A module's own settings are a component*
*/

#ifndef BRIDGE_SRC_CONFIG_H
    #define BRIDGE_SRC_CONFIG_H

    #include "inside.h"

/*
** A module's settings being written. `c` is the component they are a value
** of and `row` that value, `text_used` how much of the module's block of
** text the settings before this one took, and `why` where a refusal goes.
*/
typedef struct {
    bridge_t *b;
    module_t *m;
    const bridge_module_config_t *cfg;
    declared_type_t *c;
    uint8_t *row;
    uint32_t text_used;
    char *why;
    size_t why_len;
} config_seed_t;

/*
** What writes one setting into a field of one kind. 1 if it is refused.
*/
typedef int (*config_seeder_fn_t)(config_seed_t *seed,
    const schema_field_t *field, const bridge_setting_t *s);

/*
** The integers a field of an integer kind holds, and how many bytes it is.
*/
typedef struct {
    int64_t least;
    int64_t most;
    uint32_t size;
} config_integer_t;

/*
** config_seed
**
** Writes a module's settings into the component its config names. 0, or 1
** with `err` saying which setting or which name was refused.
*/
[[nodiscard]] int config_seed(bridge_t *b, module_t *m,
    const bridge_module_config_t *cfg, backend_err_t *err);

/*
** config_keep
**
** Remembers where an unloaded module's settings text is: the block outlives
** the module, and its next load writes there.
**
** strings.md, *Changing one, and the cost of growing*
*/
void config_keep(bridge_t *b, module_t *m);

/*
** config_forget
**
** Forgets where a module's settings text was: its share has gone back to the
** arena, and the block with it.
*/
void config_forget(bridge_t *b, const module_t *m);

/*
** config_find_kept
**
** Answers what is remembered of the settings text of the module `stem`, or
** nullptr.
*/
kept_text_t *config_find_kept(bridge_t *b, const char *stem);

/*
** config_refuse
**
** Writes why the settings are refused, as printf would. Answers 1.
*/
[[gnu::format(printf, 2, 3)]] int config_refuse(config_seed_t *seed,
    const char *format, ...);

/*
** config_say_misfit
**
** Refuses a setting whose kind is not one the field it names takes: `field`
** says what the field is. Answers 1.
*/
int config_say_misfit(config_seed_t *seed, const bridge_setting_t *s,
    const char *field);

/*
** config_place_text
**
** Finds where the text of the settings goes, before any is written: where it
** went the last time the module was loaded, when it still fits there, or the
** module's persistent share. 1 if there is no room.
*/
int config_place_text(config_seed_t *seed);

/*
** config_seed_setting
**
** Writes one setting into the field its key names. 1 if no field has that
** name, or the field does not take it.
*/
int config_seed_setting(config_seed_t *seed, const bridge_setting_t *s);

/*
** The seeders, one for each kind a field can be, and one for an inline array.
*/
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
