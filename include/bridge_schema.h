/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** include/bridge_schema.h -- a type, as a module in C states it: the typedef
** its compiler lays out, and the descriptor Bridge reads to lay it out
** the same way.
**
** It is Bridge's and not a backend's: the descriptor is a text
** Bridge parses, so how it is spelled in C is stated once, here, for a
** module on any backend. A schema header is written with these and included
** by whoever shares its components. A module in another language does not use
** this file: it writes the same text its own way.
**
** schema.md, *There is no schema format*
*/

#ifndef BRIDGE_SCHEMA_H
    #define BRIDGE_SCHEMA_H

/*
** A type and its descriptor are made of one text, so that they cannot differ.
** The text is named once, by a macro of the author's, and handed to both:
**
**     #define POSITION_FIELDS { float x; float y; }
**     BRIDGE_SCHEMA_STRUCT_TYPE(position_t, POSITION_FIELDS);
**     BRIDGE_SCHEMA_STRUCT_DESC(position_t, POSITION_FIELDS);
**
** They are two macros because a macro is one statement, and no declaration is
** both a type and the text that describes it.
**
** schema.md, *There is no schema format*
*/
    #define BRIDGE_SCHEMA_TEXT(...) #__VA_ARGS__

/*
** A struct: its body is its descriptor.
**
** schema.md, *There is no schema format*
*/
    #define BRIDGE_SCHEMA_STRUCT_TYPE(name, ...) \
        typedef struct name __VA_ARGS__ name
    #define BRIDGE_SCHEMA_STRUCT_DESC(name, ...) \
        static const char name##_desc[] = BRIDGE_SCHEMA_TEXT(__VA_ARGS__)

/*
** An enum and a bitmask are the integer their author states: the width a row
** gives one. It goes into the descriptor ahead of the brace, where
** Bridge reads it and checks that every constant fits.
**
** schema.md, *The type set*
*/
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
