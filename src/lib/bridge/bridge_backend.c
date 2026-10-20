/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "backend.h"
#include "bridge_inside.h"
#include "say.h"

static_assert((int)BRIDGE_BOOL == (int)BACKEND_BOOL);
static_assert((int)BRIDGE_INT == (int)BACKEND_INT);
static_assert((int)BRIDGE_FLOAT == (int)BACKEND_FLOAT);
static_assert((int)BRIDGE_STRING == (int)BACKEND_STRING);
static_assert(sizeof(bridge_setting_t) == sizeof(backend_setting_t));
static_assert(sizeof(bridge_value_t) == sizeof(backend_value_t));
static_assert(offsetof(bridge_setting_t, kind)
    == offsetof(backend_setting_t, kind));
static_assert(offsetof(bridge_setting_t, v) == offsetof(backend_setting_t, v));

static int check_backend(bridge_world_t *w, const bridge_config_t *cfg,
    uint32_t i)
{
    const bridge_backend_t *want = &cfg->backends[i];
    uint32_t k = 0;

    if (!want->name || !want->library) {
        say_error(w, "backend %u has no %s", i,
            want->name ? "library" : "name");
        return (1);
    }
    while (k < i && strcmp(w->backends[k].name, want->name))
        k++;
    if (k < i) {
        say_error(w, "backend '%s' is named twice", want->name);
        return (1);
    }
    return (0);
}

static backend_setting_t *convert_settings(const bridge_backend_t *want)
{
    uint32_t count = want->setting_count;
    backend_setting_t *settings = calloc(count ? count : 1, sizeof(*settings));
    uint32_t k = 0;

    while (settings && k < count) {
        settings[k].key = want->settings[k].key;
        settings[k].kind = (enum backend_setting_kind)want->settings[k].kind;
        memcpy(&settings[k].v, &want->settings[k].v, sizeof(settings[k].v));
        k++;
    }
    return (settings);
}

static int open_library(bridge_world_t *w, const bridge_backend_t *want,
    open_backend_t *o)
{
    backend_err_t opened;

    o->name = copy_text(want->name, strlen(want->name));
    if (bridge_lacks(w, o->name, "a backend's name"))
        return (1);
    w->backend_count++;
    o->lib = backend_open(want->library, &o->api, &opened);
    if (o->lib)
        return (0);
    say_error(w, "backend '%s': %s", want->name, opened.text);
    return (1);
}

static int start_backend(bridge_world_t *w, const bridge_backend_t *want,
    open_backend_t *o)
{
    backend_err_t err = {};
    backend_setting_t *settings = convert_settings(want);

    if (bridge_lacks(w, settings, "a backend's settings"))
        return (1);
    o->be = o->api.backend_create(settings, want->setting_count, &err);
    free(settings);
    if (o->be)
        return (0);
    say_error(w, "backend '%s' would not start: %s", want->name, err.text);
    return (1);
}

int bridge_open_backend(bridge_world_t *w, const bridge_config_t *cfg,
    uint32_t i)
{
    const bridge_backend_t *want = &cfg->backends[i];
    open_backend_t *o = &w->backends[i];

    if (check_backend(w, cfg, i))
        return (1);
    if (open_library(w, want, o))
        return (1);
    return (start_backend(w, want, o));
}
