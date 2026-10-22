/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "host.h"
#include "os.h"

bool take_lock(const char *name)
{
    size_t len = strlen(name) + sizeof(".d/lock");
    char *path = malloc(len);
    int got = -1;

    if (path)
        snprintf(path, len, "%s.d", name);
    if (!path || os_folder_make(path)) {
        printf("refused: %s.d is no folder\n", name);
        return (false);
    }
    snprintf(path, len, "%s.d/lock", name);
    got = os_lock(path);
    if (got > 0)
        printf("refused: %s is being run\n", name);
    if (got < 0)
        printf("cannot lock '%s'\n", path);
    return (!got);
}
