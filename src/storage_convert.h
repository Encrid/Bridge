/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/storage_convert.h -- a component's rows written again as another
** type, at a reload: what the two files that do it share.
**
** lifetime.md, *When a component is moved*
*/

#ifndef BRIDGE_SRC_STORAGE_CONVERT_H
    #define BRIDGE_SRC_STORAGE_CONVERT_H

    #include "storage_inside.h"

/*
** storage_fold_moves
**
** Folds what each entity that carries one of the moved types will carry,
** and finds a table and room for each, with nothing moved. false if one
** cannot be had: the fold is forgotten, and nothing has changed.
*/
[[nodiscard]] bool storage_fold_moves(const convert_t *c);

#endif
