/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <dirent.h>

#include "host.h"
#include "os.h"

static bool is_ignored(const host_t *h, const char *path)
{
    char *whole = os_path_whole(path);
    char *ignored = nullptr;
    size_t len = 0;
    bool is = false;

    for (uint32_t i = 0; whole && !is && i < h->ignore_count; i++) {
        ignored = os_path_whole(h->ignore[i]);
        len = ignored ? strlen(ignored) : 0;
        is = ignored && !strncmp(whole, ignored, len)
            && (!whole[len] || whole[len] == '/' || whole[len] == '\\');
        free(ignored);
    }
    free(whole);
    return (is);
}

static void walk_folder(const host_t *h, const char *path, see_fn *see,
    void *ud)
{
    DIR *d = opendir(path);
    char full[1024];

    for (struct dirent *e = d ? readdir(d) : nullptr; e; e = readdir(d)) {
        snprintf(full, sizeof(full), "%s/%s", path, e->d_name);
        if (strcmp(e->d_name, ".") && strcmp(e->d_name, "..")
            && !is_ignored(h, full))
            see(ud, full);
    }
    if (d)
        closedir(d);
}

static int walk_path(const host_t *h, const char *path, see_fn *see,
    void *ud)
{
    DIR *d = opendir(path);
    FILE *file = d ? nullptr : fopen(path, "rb");

    if (d)
        closedir(d);
    if (!d && !file) {
        printf("cannot look in '%s'\n", path);
        return (-1);
    }
    if (file) {
        fclose(file);
        see(ud, path);
        return (0);
    }
    walk_folder(h, path, see, ud);
    return (0);
}

int walk_content(const host_t *h, see_fn *see, void *ud)
{
    for (uint32_t i = 0; i < h->path_count; i++)
        if (walk_path(h, h->paths[i], see, ud) < 0)
            return (-1);
    return (0);
}
