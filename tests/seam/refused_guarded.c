/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include <string.h>

#include "cases.h"

void test_create_no_sink(host_t *h)
{
    bridge_t *b = bridge_create(&h->cfg, nullptr, nullptr);

    printf("%s\n", b ? "created" : "refused: no sink");
    printf("alive\n");
}

void test_state_other_abi(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_small(&a, identity, "ok");
    a.cfg.abi = BRIDGE_ABI + 1;
    host_load(h, &a);
    host_close(h);
}

void test_name_no_backend(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_small(&a, identity, "ok");
    a.backend = "plain";
    host_load(h, &a);
    host_close(h);
}

static size_t get_most(const char *what)
{
    if (!strcmp(what, "long"))
        return (40'000);
    return (strcmp(what, "multibyte") ? 260 : 300);
}

static const char *get_library(const char *what)
{
    static char path[40'001];
    const char *unit = strcmp(what, "multibyte") ? "a" : "\xc3\xa9";
    size_t step = strlen(unit);
    size_t most = get_most(what);
    size_t len = 3;

    if (!strcmp(what, "nolib"))
        return ("no-such-backend-library");
    if (!strcmp(what, "backslash"))
        return ("no\\such-backend-library");
    memcpy(path, "C:/", 3);
    while (len + step <= most) {
        memcpy(path + len, unit, step);
        len += step;
    }
    path[len] = '\0';
    return (path);
}

void test_open_no_library(host_t *h)
{
    h->backends[0].library = get_library(h->arg);
    if (host_open(h))
        return;
    test_load_small(h, h->arg);
    host_close(h);
}

void test_state_other_host_abi(host_t *h)
{
    h->cfg.abi = BRIDGE_HOST_ABI + 1;
    if (host_open(h))
        return;
    test_load_small(h, h->arg);
    host_close(h);
}
