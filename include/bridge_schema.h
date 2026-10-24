/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SCHEMA_H
    #define BRIDGE_SCHEMA_H

    #define BRIDGE_SCHEMA_TEXT(...) #__VA_ARGS__

    #define BRIDGE_SCHEMA_STRUCT_TYPE(name, ...) \
        typedef struct name __VA_ARGS__ name
    #define BRIDGE_SCHEMA_STRUCT_DESC(name, ...) \
        static const char name##_desc[] = BRIDGE_SCHEMA_TEXT(__VA_ARGS__)

    #define BRIDGE_SCHEMA_ENUM_TYPE(name, type, ...) \
        typedef enum name : type __VA_ARGS__ name
    #define BRIDGE_SCHEMA_ENUM_DESC(name, type, ...) \
        static const char name##_desc[] = \
            #type " " BRIDGE_SCHEMA_TEXT(__VA_ARGS__)
    #define BRIDGE_SCHEMA_BITMASK_TYPE(name, type, ...) \
        typedef enum name : type __VA_ARGS__ name
    #define BRIDGE_SCHEMA_BITMASK_DESC(name, type, ...) \
        static const char name##_desc[] = \
            #type " " BRIDGE_SCHEMA_TEXT(__VA_ARGS__)

#endif
