/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_EXPORT_H
    #define BRIDGE_EXPORT_H

    #if defined(BRIDGE_BUILDING) && defined(_WIN32)
        #define BRIDGE_API [[gnu::dllexport]]
    #elif defined(BRIDGE_BUILDING)
        #define BRIDGE_API [[gnu::visibility("default")]]
    #else
        #define BRIDGE_API
    #endif

#endif
