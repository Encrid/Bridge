/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/settings_room.c -- a config whose component is not there, is
** not a singleton, or has no room: for its row, or for its text.
**
** config.md, *A module's own settings are a component*
*/

#include "cases.h"

/*
** load_built
**
** Loads the settings module from the build of a file, with its config
** naming `config` and a share of the arena of `persistent` bytes.
*/
static void load_built(host_t *h, const char *file, const char *config,
    int64_t persistent)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_configured(h, &a, "game:settings@0.1.0");
    a.bytes = host_read_module(file, &a.len);
    a.cfg.config = config;
    a.cfg.persistent = persistent;
    host_load(h, &a);
    test_say_modules(h);
    host_close(h);
}

/*
** test_name_missing_config
**
** A config that names a component the module's init did not declare.
*/
void test_name_missing_config(host_t *h)
{
    load_built(h, "settings", "game:Nope", 64);
}

/*
** test_state_small_arena
**
** A share of the arena the settings' text does not fit in.
*/
void test_state_small_arena(host_t *h)
{
    load_built(h, "settings", "game:Settings", 8);
}

/*
** test_leave_no_room
**
** A column region with no room for the settings' row.
*/
void test_leave_no_room(host_t *h)
{
    h->cfg.budget.columns = 16;
    load_built(h, "settings", "game:Settings", 64);
}

/*
** test_configure_table
**
** The component a config names is a singleton, and one that is kept in a
** table is refused.
*/
void test_configure_table(host_t *h)
{
    load_built(h, "settings-table", "game:Settings", 64);
}
