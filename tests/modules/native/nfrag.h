/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/nfrag.h -- the components of nfrag.c: the value
** every entity carries, and what a marker holds. Built with TAGS, the
** markers are tags and hold nothing.
*/

#ifndef TEST_NFRAG_H
    #define TEST_NFRAG_H

    #include "bridge_schema.h"

    #define V_FIELDS { uint32_t v; }

BRIDGE_SCHEMA_STRUCT_TYPE(v_t, V_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(v_t, V_FIELDS);

    #ifndef TAGS
        #define M_FIELDS { uint32_t on; }

BRIDGE_SCHEMA_STRUCT_TYPE(m_t, M_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(m_t, M_FIELDS);

    #endif

#endif
