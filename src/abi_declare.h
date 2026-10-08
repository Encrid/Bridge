/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/abi_declare.h -- a type declared by a module: what the files that
** check it, lay it out again at a reload and make it share.
**
** lifetime.md, *When a component layout changed*
*/

#ifndef BRIDGE_SRC_ABI_DECLARE_H
    #define BRIDGE_SRC_ABI_DECLARE_H

    #include "abi.h"

/*
** The other loaded modules that declare a type, as the end of a refusal:
** who they are is what a host needs to know, since unloading them is what
** lets a component's layout change.
*/
typedef struct {
    char text[160];
} abi_others_t;

/*
** A type being declared.
**
**     d         what the module stated
**     desc      its descriptor, in the scratch arena
**     name      its name, and `bare` where its namespace ends in it
**     print     the fingerprint of its descriptor and its kind
**     c         the type of that name there is, and then the one declared
**     was       at a reload, the type of that name this one replaces
**     s         its layout, read from its descriptor
*/
typedef struct {
    module_t *m;
    bridge_declaration_t d;
    const char *desc;
    char name[128];
    char *bare;
    uint64_t print;
    declared_type_t *c;
    declared_type_t *was;
    schema_t s;
} declaring_t;

/*
** A struct being laid out again at a reload.
**
**     rows      how many hold a value of the type that is changing
**     lost      what goes that this reload does not accept, by name
**     short_of  there was no memory for a run
*/
typedef struct {
    module_t *m;
    uint32_t rows;
    storage_run_t *runs;
    uint32_t run_count;
    char lost[240];
    size_t lost_len;
    bool short_of;
} relaying_t;

/*
** Where in a value a struct is being matched: the fields of `was`, at `from`
** in a value as it was, against those of `is`, at `to` in the value as it
** will be. `path` names it.
*/
typedef struct {
    const schema_t *was;
    const schema_t *is;
    uint32_t from;
    uint32_t to;
    const char *path;
} relay_at_t;

static const char *const ABI_STORAGE_NAMES[] = {
    "",
    "a table",
    "a sparse set",
    "a toggle",
    "direct-indexed",
    "a singleton"
};

/*
** abi_hash_text
**
** Answers the fingerprint of `n` bytes of text.
*/
uint64_t abi_hash_text(const char *s, size_t n);

/*
** abi_is_listed
**
** Answers whether a list of names holds a name.
*/
bool abi_is_listed(char **list, uint32_t count, const char *name);

/*
** abi_is_foreign
**
** Answers whether a name is of another namespace than the module's. A
** module's own namespace is its own, and another's needs a grant.
**
** grants.md, *Whose component it is*
*/
bool abi_is_foreign(const module_t *m, const char *name);

/*
** abi_is_writable
**
** Answers whether a module may write a type: one of its own namespace, or
** one a grant lets it write.
*/
bool abi_is_writable(const module_t *m, const declared_type_t *c);

/*
** abi_find_type
**
** What the descriptor reader calls to find a type a field names bare: it is
** looked for in the namespace of the component it is a field of. `ctx` is a
** declaring_t.
*/
bool abi_find_type(void *ctx, const char *bare, schema_named_t *named);

/*
** abi_say_others
**
** Writes who else declares a type, as the end of a refusal: nothing if
** nobody does.
*/
void abi_say_others(const module_t *m, const declared_type_t *c,
    abi_others_t *others);

/*
** abi_accepts
**
** Answers whether a reload accepts the loss named `name`, which it is now
** known to make, of something `rows` rows hold.
*/
bool abi_accepts(module_t *m, const char *name, uint32_t rows);

/*
** abi_get_element_size
**
** Answers how many bytes one of a field's elements is.
*/
uint32_t abi_get_element_size(const schema_field_t *f);

/*
** abi_add_run
**
** Records that `len` bytes at `from` of a value as it was are the bytes at
** `to` of the value as it will be.
*/
void abi_add_run(relaying_t *l, uint32_t from, uint32_t to, uint32_t len);

/*
** abi_say_lost
**
** Records, by name, a field or the end of an array that has no place in
** what the type becomes, unless no row holds one or the reload accepts it.
*/
void abi_say_lost(relaying_t *l, const char *name);

/*
** abi_names_superseded
**
** Answers whether a field of a type names a type this reload has made anew.
*/
bool abi_names_superseded(const declared_type_t *c);

/*
** abi_find_holder
**
** Answers a type with a field of type `c` that a loaded module other than
** the one reloading declares, and writes that module: its rows hold values
** of `c` as it is, which no reload of somebody else may change. nullptr if
** there is none.
*/
const declared_type_t *abi_find_holder(const module_t *m,
    const declared_type_t *c, const module_t **by);

/*
** abi_check_move
**
** Refuses a move of a type to another storage layout that cannot be made,
** or that loses what was not accepted. 1 with a refusal.
**
** lifetime.md, *When a component is moved*
*/
int abi_check_move(module_t *m, const declared_type_t *c,
    enum bridge_storage kept);

/*
** abi_relay_fields
**
** Matches the fields of a struct as it was to those of what it becomes, by
** name: each goes to its new offset, and what is new stays zero. 1 and a
** refusal for a field that is of another type than it was.
*/
int abi_relay_fields(relaying_t *l, const relay_at_t *at);

/*
** abi_relay
**
** Works out how the values of a type are carried over to what it becomes,
** and leaves it on the type, for the reload to use if it holds. 1 and a
** refusal if it cannot be, or would lose what this reload does not accept.
*/
int abi_relay(module_t *m, declared_type_t *was, enum bridge_kind kind,
    const schema_t *is);

/*
** abi_check_stated
**
** Refuses a declaration that states what it may not: a name with no
** namespace, a kind, a policy or a storage layout left unset, a tag with
** bytes, another namespace's name with no grant. Works out the fingerprint
** of what it states. 1 with a refusal.
*/
int abi_check_stated(declaring_t *g);

/*
** abi_read_layout
**
** Reads and lays out the descriptor of the type being declared, and checks
** that it is as many bytes, aligned as much, as the module stated. 1 with a
** refusal, and nothing left allocated.
*/
int abi_read_layout(declaring_t *g);

/*
** abi_replace_type
**
** Decides what becomes of the type of that name there already is: it is the
** one declared, or, at a reload of the one module that declares it, the one
** this declaration replaces. 1 with a refusal if it is neither.
*/
int abi_replace_type(declaring_t *g);

/*
** abi_make_type
**
** Makes the type a declaration states: its descriptor read and laid out, a
** place for it in storage, and the types its fields name held. 1 with a
** refusal.
*/
int abi_make_type(declaring_t *g);

#endif
