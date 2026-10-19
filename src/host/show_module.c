/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "host.h"

void show_module(void *ud, const char *path)
{
    host_t *h = ud;
    handed_t now = {};
    const stating_t s = { h, &now, "  refused: ", true };
    bridge_module_config_t cfg;
    const char *why = nullptr;

    now.bytes = read_whole(path, &now.len);
    if (!now.bytes)
        return;
    why = manifest_read((const unsigned char *)now.bytes, now.len, &now.who);
    if (why)
        printf("not a module: %s: %s\n", path, why);
    if (why)
        return;
    printf("module %s, %s, %s\n", now.who.identity, now.who.backend, path);
    if (config_load(h, now.who.identity, s.prefix) < 0
        || !state_found(&s, &cfg))
        h->counts.failed++;
}
