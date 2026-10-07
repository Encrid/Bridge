/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/storage_create.h -- storage made: everything it holds is sized by a
** budget and allocated when the bridge is created, and nothing after.
**
** storage.md, *What bounds it*
*/

#ifndef BRIDGE_SRC_STORAGE_CREATE_H
    #define BRIDGE_SRC_STORAGE_CREATE_H

    #include "storage_inside.h"

/*
** storage_make_tables
**
** Allocates what is kept of entities and of tables. false if there was not
** the memory for all of it.
*/
[[nodiscard]] bool storage_make_tables(storage_t *r);

/*
** storage_make_fold
**
** Allocates what a barrier's fold works in. false if there was not the
** memory for all of it.
*/
[[nodiscard]] bool storage_make_fold(storage_t *r);

/*
** storage_make_apart
**
** Allocates what is kept of sparse sets and of direct-indexed components.
** false if there was not the memory for all of it.
*/
[[nodiscard]] bool storage_make_apart(storage_t *r);

/*
** storage_make_terms
**
** Allocates the types and the systems' terms there may be. false if there
** was not the memory for all of it.
*/
[[nodiscard]] bool storage_make_terms(storage_t *r);

#endif
