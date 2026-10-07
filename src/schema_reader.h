/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/schema_reader.h -- a descriptor being read: the token it is at, and
** what the files that read one share.
**
** The text is what a module's compiler made of the body of a type, through a
** macro of bridge_schema.h: tokens with one space between them. What it may
** say is written out here in full, and nothing else is accepted:
**
**     a struct      { field field ... }
**     a field       type name ;   or   type name [ count ] ;
**     a type        bool char int8_t int16_t int32_t int64_t uint8_t
**                   uint16_t uint32_t uint64_t float double bridge_text_t,
**                   or a type its namespace declared
**     an enum       integer { name , name = integer , ... }
**                   counted from zero, or from the last
**     a bitmask     unsigned { name = integer , ... }
**                   every bit stated
**
** A struct is laid out as C does on every target a module is built for: each
** field at the next multiple of its alignment, the whole a multiple of its
** widest. An enum and a bitmask are the integer their author stated before
** the brace: int8_t to uint32_t for an enum, an unsigned one to uint64_t for
** a bitmask. Every constant is checked to fit it.
**
** schema.md, *The type set*
*/

#ifndef BRIDGE_SRC_SCHEMA_READER_H
    #define BRIDGE_SRC_SCHEMA_READER_H

    #include "schema.h"

/*
** A type a field can be without its namespace declaring it.
*/
typedef struct {
    const char *name;
    enum schema_kind kind;
    uint32_t size;
} schema_given_t;

constexpr uint32_t SCHEMA_GIVEN_COUNT = 12;
static const schema_given_t SCHEMA_GIVEN[SCHEMA_GIVEN_COUNT] = {
    { "bool", SCHEMA_BOOL, 1 },
    { "char", SCHEMA_CHAR, 1 },
    { "int8_t", SCHEMA_I8, 1 },
    { "uint8_t", SCHEMA_U8, 1 },
    { "int16_t", SCHEMA_I16, 2 },
    { "uint16_t", SCHEMA_U16, 2 },
    { "int32_t", SCHEMA_I32, 4 },
    { "uint32_t", SCHEMA_U32, 4 },
    { "int64_t", SCHEMA_I64, 8 },
    { "uint64_t", SCHEMA_U64, 8 },
    { "float", SCHEMA_F32, 4 },
    { "double", SCHEMA_F64, 8 }
};

/*
** The reading of one descriptor. `word` is the token just read: a name, a
** number, or one mark; `cut` says it was longer than any name may be. `room`
** is how many fields or constants `out` has memory for. `value` and `below`
** are the last constant's, which the next counts up from.
*/
typedef struct {
    const char *at;
    char word[64];
    bool cut;
    const schema_source_t *source;
    schema_t *out;
    uint32_t room;
    int64_t value;
    bool below;
} schema_reader_t;

/*
** A field with what it takes in a row, before it is placed.
*/
typedef struct {
    schema_field_t field;
    uint32_t size;
    uint32_t align;
} schema_laid_t;

/*
** schema_read_next
**
** Reads the next token into `word`: a run of name characters, or a single
** other character. Empty at the end of the text. A run too long for any name
** is cut, and will be refused as a name.
*/
void schema_read_next(schema_reader_t *r);

/*
** schema_refuse
**
** Writes why the descriptor is refused: `what`, with the token it stopped at
** where `what` says %s. Answers 1.
*/
int schema_refuse(schema_reader_t *r, const char *what);

/*
** schema_is
**
** Answers whether the token is `word`.
*/
bool schema_is(const schema_reader_t *r, const char *word);

/*
** schema_is_name
**
** Answers whether the token is a name a field or a constant may have: it
** begins with a letter or an underscore.
*/
bool schema_is_name(const schema_reader_t *r);

/*
** schema_get_integer
**
** Reads the token as an integer, decimal or 0x, as it is written: below
** zero, or its bits, which reach past INT64_MAX. false if it is not one.
*/
bool schema_get_integer(const schema_reader_t *r, int64_t *value, bool *below);

/*
** schema_find_given
**
** Answers which of SCHEMA_GIVEN the token names, or -1.
*/
int schema_find_given(const schema_reader_t *r);

/*
** schema_copy_text
**
** Answers a copy of a text that free gives back, or nullptr.
*/
char *schema_copy_text(const char *text);

/*
** schema_fits
**
** Answers whether the integer `base` is holds a value.
*/
bool schema_fits(enum schema_kind base, int64_t value, bool below);

/*
** schema_set_value
**
** Gives the constant just named its value: the one stated after an equals
** sign, or one more than the last. A bitmask states every one. 1 if refused.
*/
int schema_set_value(schema_reader_t *r, bool bits);

/*
** schema_set_type
**
** Reads the token as a field's type: what it is, and what it takes in a row.
** 1 if it is neither a type a field can be nor one its namespace declared.
*/
int schema_set_type(schema_reader_t *r, schema_laid_t *laid);

/*
** schema_parse_struct
**
** Reads a struct's fields, from its opening brace to its closing one, and
** lays them out. 1 if refused.
*/
int schema_parse_struct(schema_reader_t *r);

/*
** schema_parse_constants
**
** Reads an enum's or a bitmask's integer and its constants, to its closing
** brace. 1 if refused.
*/
int schema_parse_constants(schema_reader_t *r, bool bits);

#endif
