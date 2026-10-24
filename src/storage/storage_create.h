/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_STORAGE_CREATE_H
    #define BRIDGE_SRC_STORAGE_CREATE_H

    #include "storage_inside.h"

[[nodiscard]] bool storage_make_tables(storage_t *r);

[[nodiscard]] bool storage_make_fold(storage_t *r);

[[nodiscard]] bool storage_make_apart(storage_t *r);

[[nodiscard]] bool storage_make_terms(storage_t *r);

#endif
