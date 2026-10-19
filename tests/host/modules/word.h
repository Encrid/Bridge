/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_HOST_WORD_H
    #define BRIDGE_HOST_WORD_H

    #include "bridge_wasm.h"

    #define WORD_FIELDS { char word[16]; }

BRIDGE_SCHEMA_STRUCT_TYPE(word_t, WORD_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(word_t, WORD_FIELDS);

#endif
