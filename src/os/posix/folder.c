/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <sys/stat.h>
#include <unistd.h>

#include "os.h"

int os_folder_set(const char *folder)
{
    return (chdir(folder));
}

int os_folder_make(const char *folder)
{
    struct stat there;

    if (!mkdir(folder, 0777))
        return (0);
    if (!stat(folder, &there) && S_ISDIR(there.st_mode))
        return (0);
    return (-1);
}
