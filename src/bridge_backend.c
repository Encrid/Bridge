/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/bridge_backend.c -- one backend of a deployment: its library opened,
** and the backend started with its settings.
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

/*
** check_backend
**
** Refuses a backend with no name or no library, or named as one before it.
**
** create.md, *Create*
*/
static int check_backend(bridge_t *b, const bridge_config_t *cfg, uint32_t i)
{
    const bridge_backend_t *want = &cfg->backends[i];
    uint32_t k = 0;

    if (!want->name || !want->library) {
        say_error(b, "backend %u has no %s", i,
            want->name ? "library" : "name");
        return (1);
    }
    while (k < i && strcmp(b->backends[k].name, want->name))
        k++;
    if (k < i) {
        say_error(b, "backend '%s' is named twice", want->name);
        return (1);
    }
    return (0);
}

/*
** convert_settings
**
** Answers a backend's settings as the backend seam states them. There are
** two structs for one fact, one per seam, so that neither header needs the
** other: they are held to one shape above, where one is copied to the other,
** the same four kinds under the two seams' names and the same numbers.
*/
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

/*
** open_library
**
** Opens a backend's library and finds its entries. The backend is counted
** once it has its name, so that a deck that is refused frees it.
**
** create.md, *Create*
*/
static int open_library(bridge_t *b, const bridge_backend_t *want,
    open_backend_t *o)
{
    backend_err_t opened;

    o->name = copy_text(want->name, strlen(want->name));
    if (bridge_lacks(b, o->name, "a backend's name"))
        return (1);
    b->backend_count++;
    o->lib = backend_open(want->library, &o->api, &opened);
    if (o->lib)
        return (0);
    say_error(b, "backend '%s': %s", want->name, opened.text);
    return (1);
}

/*
** start_backend
**
** Creates the backend, with its settings passed to it unread.
**
** host_api.md, *The data*
*/
static int start_backend(bridge_t *b, const bridge_backend_t *want,
    open_backend_t *o)
{
    backend_err_t err = {};
    backend_setting_t *settings = convert_settings(want);

    if (bridge_lacks(b, settings, "a backend's settings"))
        return (1);
    o->be = o->api.backend_create(settings, want->setting_count, &err);
    free(settings);
    if (o->be)
        return (0);
    say_error(b, "backend '%s' would not start: %s", want->name, err.text);
    return (1);
}

/*
** bridge_open_backend
**
** Opens the backend `i` of the deployment and starts it.
*/
int bridge_open_backend(bridge_t *b, const bridge_config_t *cfg, uint32_t i)
{
    const bridge_backend_t *want = &cfg->backends[i];
    open_backend_t *o = &b->backends[i];

    if (check_backend(b, cfg, i))
        return (1);
    if (open_library(b, want, o))
        return (1);
    return (start_backend(b, want, o));
}
