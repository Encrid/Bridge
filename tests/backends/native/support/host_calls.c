/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "host.h"

void host_load(host_t *h, const bridge_artifact_t *a)
{
    const char *ended = bridge_load(h->b, a) ? "not loaded" : "loaded";

    printf("%s: %s\n", a->cfg.identity, ended);
}
