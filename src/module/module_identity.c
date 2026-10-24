/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "module.h"

static bool has_form(const char *id)
{
    const char *colon = strchr(id, ':');
    const char *at = strchr(id, '@');

    if (!colon || !at || colon == id || at <= colon + 1 || !at[1])
        return (false);
    return (!strchr(colon + 1, ':') && !strchr(at + 1, '@'));
}

int module_check_identity(bridge_world_t *w, const char *id)
{
    if (!id || !*id) {
        say_error(w, "a module with no identity is refused");
        return (1);
    }
    if (strlen(id) > 127) {
        say_error(w, "a module with an identity of %zu bytes is refused: "
            "an identity has at most 127 bytes", strlen(id));
        return (1);
    }
    if (has_form(id))
        return (0);
    say_error(w, "a module with the identity '%s' is refused: an identity "
        "has the form namespace:name@version", id);
    return (1);
}
