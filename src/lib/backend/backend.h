/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_BACKEND_H
    #define BRIDGE_SRC_BACKEND_H

    #include "bridge_backend.h"

[[nodiscard]] void *backend_open(const char *path, backend_api_t *api,
    backend_err_t *err);

#endif
