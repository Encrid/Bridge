/* A type, as a module in C states it: the typedef its compiler lays out, and the descriptor
 * Bridge-ASM700 reads to lay it out the same way. docs/modules/schema.md.
 *
 * It is Bridge-ASM700's and not a backend's: the descriptor is a text Bridge-ASM700 parses,
 * so how it is spelled in C is stated once, here, for a module on any backend. A schema header
 * is written with these and included by whoever shares its components. A module in another
 * language does not use this file: it writes the same text its own way. */
#ifndef BRIDGE_SCHEMA_H
#define BRIDGE_SCHEMA_H

/* a struct: its body is its descriptor */
#define BRIDGE_SCHEMA_STRUCT(name, ...) \
    typedef struct name __VA_ARGS__ name; \
    static const char name##_desc[] = #__VA_ARGS__

/* An enum and a bitmask are the integer their author states: the width a row gives one. It
 * goes into the descriptor ahead of the brace, where Bridge-ASM700 reads it and checks that
 * every constant fits - docs/modules/schema.md, *The type set* */
#define BRIDGE_SCHEMA_ENUM(name, type, ...) \
    typedef enum name : type __VA_ARGS__ name; \
    static const char name##_desc[] = #type " " #__VA_ARGS__
#define BRIDGE_SCHEMA_BITMASK(name, type, ...) \
    typedef enum name : type __VA_ARGS__ name; \
    static const char name##_desc[] = #type " " #__VA_ARGS__

#endif
