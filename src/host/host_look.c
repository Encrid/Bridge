/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "host.h"

static int order_files(const void *left, const void *right)
{
    const file_t *l = *(file_t *const *)left;
    const file_t *r = *(file_t *const *)right;
    int order = strcmp(l->of->who.identity, r->of->who.identity);

    return (order ? order : strcmp(l->path, r->path));
}

static void pass_file(host_t *h, look_t *lk, file_t *f, handed_t *now)
{
    if (f)
        f->seen = true;
    if (!f && now->bytes && !h->full) {
        printf("too many files: %d are looked at\n", FILES);
        lk->did++;
    }
    h->full = h->full || (now->bytes && !f);
    free(now->bytes);
    free(now);
}

static void see_file(void *ud, const char *path)
{
    seeing_t *s = ud;
    handed_t *now = calloc(1, sizeof(*now));
    file_t *f = nullptr;
    uint64_t mark = 0;

    now->bytes = read_whole(path, &now->len);
    if (now->bytes)
        f = find_file(s->h, path);
    if (now->bytes)
        mark = hash_bytes(now->bytes, now->len);
    if (!f || (f->path && f->mark == mark)) {
        pass_file(s->h, s->lk, f, now);
        return;
    }
    if (meet(s->h, f, path, now))
        s->lk->found[s->lk->count++] = f;
    f->mark = mark;
    f->seen = true;
    s->lk->did++;
}

static void drop_gone(host_t *h, look_t *lk)
{
    file_t *f = nullptr;

    for (int i = 0; i < FILES; i++) {
        f = &h->files[i];
        if (!f->path || f->seen)
            continue;
        if (f->loaded && !bridge_unload(h->w, f->of->who.identity))
            printf("unloaded: %s: %s is gone\n", f->of->who.identity,
                f->path);
        lk->did += f->loaded;
        free(f->path);
        *f = (file_t){};
    }
}

int look(host_t *h, bool first)
{
    look_t lk = {};
    seeing_t s = { h, &lk };

    for (int i = 0; i < FILES; i++)
        h->files[i].seen = false;
    if (walk_content(h, see_file, &s) < 0)
        return (-1);
    drop_gone(h, &lk);
    qsort(lk.found, (size_t)lk.count, sizeof(lk.found[0]), order_files);
    for (int i = 0; i < lk.count; i++)
        load_found(h, lk.found[i], first);
    return (lk.did);
}
