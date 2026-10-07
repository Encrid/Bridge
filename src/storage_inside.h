/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/storage_inside.h -- storage, kept by Bridge itself: what it
** holds.
**
** It is made for what the deck promises and a general store cannot assume:
** rows are touched only at a barrier, by one thread, with every change of
** that barrier already queued; systems are fixed at load; nobody iterates
** through it; and no order of rows is anybody's to keep.
**
**   - A barrier is worked an entity at a time. The queues are folded, in
**     their stated order, into what each changed entity will be, and it is
**     then placed once. No table is made that nothing stays in.
**   - A table is one set of types, found by that set, and lives while it has
**     rows. Its rows are one block of the column region: the entity ids, then
**     a column for each type.
**   - Nothing in the shared block is read back. Where an entity is, and which
**     entity is in which row, are kept here; what the block holds of either
**     is a copy for modules to read.
**   - Everything it holds is sized by a budget and allocated when the deck
**     is created: entities, tables, columns, types, systems. Nothing is
**     allocated after.
**   - What a module reads is rewritten where a table was touched, and nowhere
**     else.
**
** storage.md, *A barrier is worked an entity at a time*
*/

#ifndef BRIDGE_SRC_STORAGE_INSIDE_H
    #define BRIDGE_SRC_STORAGE_INSIDE_H

    #include "inside.h"

/*
** A place whose entity is gone.
*/
constexpr uint32_t DEAD = 0xffffffffu;

/*
** A column that begins at a multiple of sixteen is read by a compiler's loop
** of 16-byte loads with none of them across two cache lines. Measured: one
** that begins eight past it takes 8.3 us for 204,800 rows where this takes
** 5.6 to 6.4.
**
** storage.md, *A table*
*/
constexpr uint64_t COLUMN = 16;

/*
** What the fold has decided of an entity, for the length of one barrier.
** SHORT: dropped for want of columns, and not of a table. MOVES: it is to
** have a row it has not, and room is found for it.
*/
enum folded : uint8_t {
    TOUCHED = 1,
    DESTROYED = 2,
    DROPPED = 4,
    SHORT = 8,
    MOVES = 16
};

/*
** A declared type.
**
**     id        its bit in a set, and its column in the layout
**     align     where a column of it may begin: a power of two
**     kept      the storage layout it is kept in
**     slot      which sparse set it is, when it is one; or which of the
**               direct-indexed
**     cells, cells_len
**               direct-indexed: its block, the sixteen bytes that say where
**               its bits are and then a cell an entity. A singleton: its
**               block, which is its one value
**     set       a singleton: it has a value
*/
struct storage_type {
    uint32_t id;
    uint32_t size;
    uint32_t align;
    enum bridge_storage kept;
    uint32_t slot;
    uint32_t cells;
    uint32_t cells_len;
    bool set;
};

/*
** A system's terms, or an observer's.
**
**     type, oper    [budget] terms of each, of which `count` are its own
**     list, pos     a system's: the tables it matches, and where each is in
**                   that list, less one
**     changed       which tables it matches is not what was last written
**     used          of the [budget] systems there are, this one is taken
**     events, heard, ctx
**                   an observer's
*/
struct storage_terms {
    uint32_t count;
    uint32_t *type;
    enum bridge_oper *oper;
    uint32_t *list;
    uint32_t *pos;
    uint32_t matched;
    bool changed;
    bool used;
    enum bridge_event events;
    storage_heard_t heard;
    void *ctx;
};

/*
** Where an entity is. For a place that is free, `row` is the next free index.
*/
typedef struct {
    uint32_t table;
    uint32_t row;
    uint32_t generation;
} place_t;

/*
** A table.
**
**     block, block_len
**               its block of the column region, from the shared base
**     incoming  rows this barrier will add: room is found for them first
**     sized     and it has been: `incoming` is how many it has room for
**     named     it is the table its set is found as
**     set       the sparse set this is the table of, plus one: 0 for the
**               table of an archetype
**
** lifetime.md, *What unload leaves behind*
*/
typedef struct {
    uint32_t rows;
    uint32_t room;
    uint32_t block;
    uint32_t block_len;
    uint32_t incoming;
    bool live;
    bool touched;
    bool sized;
    bool named;
    uint32_t set;
} table_t;

/*
** One deck's storage.
**
**     places        entities: where each is. Never the copy in the block
**     high, free_head
**                   indices never used begin at `high`; freed ones are a list
**     tables        0 is no table
**     table_next    the free list of table ids
**     hash          a set to its table: open addressing, twice the room
**     sets          [table][words]: which types a table's entities carry
**     columns       [table][types]: where a type's column is, 0 if absent
**     words, type_room
**                   how wide a set is, in 64-bit words and in types
**     who           which entity is in which row: the column region's twin
**     dirty         tables touched since what modules read was last written
**     types         by id: nullptr where none is declared. [budget] types
**                   and [budget] systems of them, and no more
**     lists         every system's `list` and `pos`, one after another
**     term_types, term_opers
**                   and every one's terms
**
** The fold.
**
**     work          [entity][words]: what a changed entity is becoming
**     changed       the entities this barrier changes, in the order met
**     dest          the table each will be in
**     at            where lay_out last put each type's column: a type each
**     attached      and of that, what an attach wrote a value of: kept only
**                   while there is an observer to be told
**     scratch, before, masked
**                   one set each
**
** Sparse sets: [budget] sparse_sets of them. Each is kept as a table of its
** one type, whose rows are found by entity, here, and not by where the
** entity is.
**
**     set_type      by slot: nullptr where none is declared
**     set_table     by slot: its table, 0 while no entity is in it
**     where         [slot][entity]: its row there plus one, 0 for none
**     in_table      one set: the types that are kept in tables
**     table_count   how many tables of archetypes there are
**     tags          how many declared types have no bytes
**     apart         how many declared types are not kept in a table. While
**                   there is none, an entity carries what its table holds and
**                   nothing else, and a barrier looks no further
**
** Direct-indexed: [budget] direct_indexed of them. A cell for every entity,
** in a block of the column region taken at declare; and a bit for every
** entity that says it has a value there, kept here, with a copy for modules.
**
**     direct_type   by slot: nullptr where none is declared
**     given         [slot][bit_words]: which entities have one
**
** storage.md, *A sparse set*
** storage.md, *Direct-indexed*
*/
struct storage {
    bridge_t *b;
    place_t *places;
    uint32_t high;
    uint32_t free_head;
    table_t *tables;
    uint32_t table_room;
    uint32_t table_free;
    uint32_t table_high;
    uint32_t *table_next;
    uint32_t *hash;
    uint32_t hash_room;
    uint64_t *sets;
    uint32_t *columns;
    uint32_t words;
    uint32_t type_room;
    uint32_t terms_room;
    uint64_t *who;
    uint32_t *dirty;
    uint32_t dirty_count;
    storage_type_t **types;
    storage_type_t *type_pool;
    storage_terms_t *terms_pool;
    uint32_t *lists;
    uint32_t *term_types;
    enum bridge_oper *term_opers;
    storage_terms_t **systems;
    storage_terms_t **observers;
    uint32_t system_count;
    uint32_t observer_count;
    uint64_t *work;
    uint32_t *changed;
    uint32_t changed_count;
    uint8_t *folded;
    uint32_t *dest;
    uint64_t *at;
    uint64_t *attached;
    uint64_t *scratch;
    uint32_t set_room;
    storage_type_t **set_type;
    uint32_t *set_table;
    uint32_t *where;
    uint64_t *in_table;
    uint64_t *before;
    uint64_t *masked;
    uint32_t table_count;
    uint32_t tags;
    uint32_t apart;
    uint32_t direct_room;
    uint32_t bit_words;
    storage_type_t **direct_type;
    uint64_t *given;
};

/*
** A row on its way from one table to another: the entity at `index` leaves
** row `row` of `from` for row `at` of `to`. Either table may be 0, which is
** no table.
*/
typedef struct {
    uint32_t index;
    uint32_t from;
    uint32_t row;
    uint32_t to;
    uint32_t at;
} row_move_t;

/*
** What an observer is told of: an entity, what it carried and what it
** carries now, and of that, what an attach wrote a value of. `before` and
** `after` are null for an entity that carried nothing, or is gone.
*/
typedef struct {
    uint64_t id;
    const uint64_t *before;
    const uint64_t *after;
    const uint64_t *attached;
} seen_t;

/*
** A question asked of a row's bytes, and what it is handed with them.
*/
typedef struct {
    storage_is_fn_t is;
    void *ctx;
} question_t;

/*
** The rows of components being written again as other types: the moves,
** and a copy of one entity's values as they were, each after the other.
*/
typedef struct {
    storage_t *r;
    const storage_move_t *moves;
    uint32_t count;
    uint8_t *was;
} convert_t;

#endif
