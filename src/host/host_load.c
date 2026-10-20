/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "host.h"

static const file_t *get_same(const host_t *h, const char *who)
{
    const file_t *same = nullptr;

    for (int i = 0; i < FILES; i++)
        if (h->files[i].loaded && !strncmp(h->files[i].of->who.identity, who,
            strcspn(who, "@") + 1))
            same = &h->files[i];
    return (same);
}

bool make_artifact(const stating_t *s, const file_t *f, bridge_artifact_t *a)
{
    *a = (bridge_artifact_t){ .backend = s->of->who.backend,
        .bytes = s->of->bytes, .len = s->of->len };
    if (!strcmp(s->of->who.backend, "native"))
        *a = (bridge_artifact_t){ .backend = s->of->who.backend,
            .path = f->path };
    return (state_found(s, &a->cfg));
}

static bool is_left(const host_t *h, const file_t *f)
{
    const char *who = f->of->who.identity;
    const file_t *same = get_same(h, who);
    const held_t *held = get_held(h, f->of->who.backend);

    if (same)
        printf("not loaded: %s: %s is %s\n", f->path, who, same->path);
    else if (f->of->who.abi != BRIDGE_ABI)
        printf("not loaded: %s: %s was built against another ABI\n",
            f->path, who);
    else if (!held)
        printf("not loaded: %s: %s asks for %s, which the host was not "
            "given\n", f->path, who, f->of->who.backend);
    return (same || f->of->who.abi != BRIDGE_ABI || !held);
}

static bool is_stated(host_t *h, const file_t *f, bridge_artifact_t *a)
{
    char prefix[1200];
    const stating_t s = { h, f->of, prefix, false };
    int load = 0;

    snprintf(prefix, sizeof(prefix), "not loaded: %s: %s: ", f->path,
        f->of->who.identity);
    load = config_load(h, f->of->who.identity, prefix);
    if (!load)
        printf("not loaded: %s: %s, as content.load says\n", f->path,
            f->of->who.identity);
    if (load > 0 && make_artifact(&s, f, a))
        return (true);
    h->counts.failed += load != 0;
    return (false);
}

void load_found(host_t *h, file_t *f, bool first)
{
    bridge_artifact_t a;
    const char *who = f->of->who.identity;

    if (is_left(h, f) || !is_stated(h, f, &a))
        return;
    if (!first && !may_arrive(f, get_held(h, f->of->who.backend)))
        return;
    if (bridge_load(h->w, &a)) {
        printf("not loaded: %s: %s was refused\n", f->path, who);
        return;
    }
    printf("loaded: %s, %s, %s\n", who, a.backend, f->path);
    f->loaded = true;
}
