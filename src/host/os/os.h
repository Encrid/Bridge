/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_HOST_OS_H
    #define BRIDGE_HOST_OS_H

[[nodiscard]] int os_folder_set(const char *folder);

[[nodiscard]] int os_folder_make(const char *folder);

[[nodiscard]] char *os_path_whole(const char *path);

[[nodiscard]] int os_lock(const char *path);

#endif
