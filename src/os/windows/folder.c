/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <direct.h>
#include <sys/stat.h>

#include "os.h"

int os_folder_set(const char *folder)
{
    return (_chdir(folder));
}

int os_folder_make(const char *folder)
{
    struct _stat there;

    if (!_mkdir(folder))
        return (0);
    if (!_stat(folder, &there) && (there.st_mode & _S_IFDIR))
        return (0);
    return (-1);
}
