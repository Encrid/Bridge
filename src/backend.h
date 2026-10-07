/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** src/backend.h -- a backend's library, opened.
*/

#ifndef BRIDGE_SRC_BACKEND_H
    #define BRIDGE_SRC_BACKEND_H

    #include "bridge_backend.h"

/*
** backend_open
**
** Opens a backend's library and finds its eighteen entries by name. Answers
** the library, or nullptr with `err` saying that it would not open, which
** entry it lacks, or that it is of another backend seam.
*/
[[nodiscard]] void *backend_open(const char *path, backend_api_t *api,
    backend_err_t *err);

#endif
