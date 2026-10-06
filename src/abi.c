/* The ABI: the eight functions a module imports. docs/modules/abi.md.
 *
 * Every pointer argument is an offset into the caller's scratch arena. An entry that cannot be
 * honoured says why in the module's record and answers non-zero, which abandons the
 * invocation that made it - docs/bridge/memory/budgets.md, *What running out does*. */
#include "inside.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int refuse(module_t *m, const char *fmt, ...)
{
    va_list args;
    /* the first refusal of a call is the reason: a backend that cannot unwind lets the
     * module run on, and what it meets afterwards is consequence */
    if (m->refused) return 1;
    va_start(args, fmt);
    vsnprintf(m->why, sizeof m->why, fmt, args);
    va_end(args);
    m->refused = 1;
    return 1;
}

/* the one memory a module can write that Bridge-ASM700 can read and bounds-check */
static const void *scratch(module_t *m, uint64_t at, uint64_t len)
{
    if (at < m->scratch || len > m->scratch_len || at - m->scratch > m->scratch_len - len) {
        return nullptr;
    }
    return m->priv + at;
}

static uint64_t fingerprint(const char *s, size_t n)
{
    uint64_t h = 1'469'598'103'934'665'603ull;       /* FNV-1a */
    size_t i;
    for (i = 0; i < n; i ++) h = (h ^ (uint8_t)s[i]) * 1'099'511'628'211ull;
    return h;
}

static int listed(char **list, uint32_t count, const char *name)
{
    uint32_t i;
    for (i = 0; i < count; i ++) {
        if (!strcmp(list[i], name)) return 1;
    }
    return 0;
}

/* grants.md: a module's own namespace is its own, and another's needs a grant */
static int foreign(module_t *m, const char *name)
{
    size_t n = strlen(m->ns);
    return strncmp(name, m->ns, n) || name[n] != ':';
}

static int may_write(module_t *m, const declared_type_t *c)
{
    return !foreign(m, c->name) || listed(m->write, m->write_count, c->name);
}

/* a type a field names bare is looked for in the namespace of the component it is a field of */
typedef struct { bridge_t *b; const char *ns; } looking_t;

static bool find(void *ctx, const char *bare, schema_named_t *named)
{
    const looking_t *l = ctx;
    size_t ns = strlen(l->ns);
    for (uint32_t i = 0; i < l->b->type_count; i ++) {
        declared_type_t *c = l->b->types[i];
        if (strncmp(c->name, l->ns, ns) || c->name[ns] != ':' || strcmp(c->name + ns + 1, bare)) continue;
        if (c->kind == BRIDGE_TAG) continue;        /* it has no bytes for a field to be */
        if (c->superseded) continue;                /* a reload has made the one that is meant */
        *named = (schema_named_t){ c->kind, c->size, c->align, c };
        return true;
    }
    return false;
}

/* The other loaded modules that declare `c`, as the end of a refusal: who they are is what a
 * host needs to know, since unloading them is what lets a component's layout change -
 * lifetime.md, *When a component layout changed*. */
static void also_declared(const bridge_t *b, const module_t *m, const declared_type_t *c, char *others,
                          size_t size)
{
    size_t used = 0;
    uint32_t i;
    others[0] = 0;
    for (module_t *o = b->modules; o; o = o->next) {
        if (o == m || o == m->reloads || !strcmp(o->identity, m->identity)) continue;
        for (i = 0; i < o->declared_count && o->declared[i] != c; i ++) {}
        if (i == o->declared_count || used >= size) continue;
        used += (size_t)snprintf(others + used, size - used, "%s'%s'",
            used ? ", " : ". It is also declared by ", o->identity);
    }
}

static const char *const storage_names[] = {
    "", "a table", "a sparse set", "a toggle", "direct-indexed", "a singleton"
};

/* Whether this reload accepts the loss named `name`, which it is now known to make, of
 * something `rows` rows hold. */
static bool accepted(module_t *m, const char *name, uint32_t rows)
{
    for (uint32_t i = 0; i < m->loss_count; i ++) {
        if (strcmp(m->losses[i], name)) continue;
        m->loss_rows[i] = rows + 1;
        return true;
    }
    return false;
}

/* ---- lifetime.md, *When a component layout changed*: what a value of a type becomes ---- */

/* how many bytes one of a field's elements is */
static uint32_t element(const schema_field_t *f)
{
    switch (f->kind) {
    case SCHEMA_BOOL: case SCHEMA_CHAR: case SCHEMA_I8: case SCHEMA_U8: return 1;
    case SCHEMA_I16: case SCHEMA_U16: return 2;
    case SCHEMA_I32: case SCHEMA_U32: case SCHEMA_F32: return 4;
    case SCHEMA_I64: case SCHEMA_U64: case SCHEMA_F64: case SCHEMA_TEXT: return 8;
    default: return ((const declared_type_t *)f->type)->size;
    }
}

typedef struct {
    module_t      *m;
    uint32_t     rows;              /* how many hold a value of the type that is changing */
    storage_run_t *runs;
    uint32_t     run_count;
    char         lost[240];         /* what goes that this reload does not accept, by name */
    size_t       lost_len;
    bool         short_of;          /* there was no memory for a run */
} relaying_t;

static void run_added(relaying_t *l, uint32_t from, uint32_t to, uint32_t len)
{
    storage_run_t *runs = realloc(l->runs, (l->run_count + 1) * sizeof *runs);
    if (!runs) {
        l->short_of = true;
        return;
    }
    l->runs = runs;
    l->runs[l->run_count ++] = (storage_run_t){ from, to, len };
}

/* a field, or the end of an array, has no place in what the type becomes */
static void lost(relaying_t *l, const char *name)
{
    if (!l->rows || accepted(l->m, name, l->rows) || l->lost_len >= sizeof l->lost) return;
    l->lost_len += (size_t)snprintf(l->lost + l->lost_len, sizeof l->lost - l->lost_len, "%s'%s'",
        l->lost_len ? ", " : "", name);
}

/* The fields of `was`, at `from` in a value, matched by name to those of `is`, at `to`: each
 * goes to its new offset, and what is new stays zero. 1 and a refusal for a field that is of
 * another type than it was. */
static int relaid(relaying_t *l, const schema_t *was, const schema_t *is, uint32_t from, uint32_t to,
                  const char *path)
{
    for (uint32_t i = 0; i < was->field_count; i ++) {
        const schema_field_t *old = &was->fields[i], *new = nullptr;
        const declared_type_t *held = old->type, *holds;
        char name[200];
        uint32_t count, k;
        snprintf(name, sizeof name, "%s.%s", path, old->name);
        for (k = 0; k < is->field_count; k ++) {
            if (!strcmp(is->fields[k].name, old->name)) new = &is->fields[k];
        }
        if (!new) {
            lost(l, name);
            continue;
        }
        holds = new->type;
        /* the same kind; and where it names a declared type, that one, or what this reload
         * has made of it */
        if (old->kind != new->kind || (held && holds != held && holds->replaces != held)
            || (held && holds->size != held->size && old->kind != SCHEMA_STRUCT)) {
            return refuse(l->m, "field '%s' is of another type than it was, and bytes of one type "
                "are not a value of another", name);
        }
        count = old->count < new->count ? old->count : new->count;
        if (new->count < old->count) lost(l, name);
        if (old->kind == SCHEMA_STRUCT && holds != held) {
            /* a struct a field names, changed: the same rules, one level down */
            for (k = 0; k < count; k ++) {
                if (relaid(l, &held->s, &holds->s, from + old->offset + k * held->size,
                        to + new->offset + k * holds->size, name)) return 1;
            }
        } else {
            run_added(l, from + old->offset, to + new->offset, count * element(new));
        }
    }
    return 0;
}

/* An enum or a bitmask whose constants changed: every value a row may hold must still mean
 * what it meant. A constant added is nothing. */
static int renumbered(module_t *m, const declared_type_t *was, const schema_t *is)
{
    if (was->s.size != is->size) {
        return refuse(m, "type '%s' is an integer of %u bytes here and was one of %u: every value "
            "kept of it would be read as another", was->name, is->size, was->s.size);
    }
    for (uint32_t i = 0; i < was->s.constant_count; i ++) {
        const schema_constant_t *old = &was->s.constants[i], *new = nullptr;
        for (uint32_t k = 0; k < is->constant_count; k ++) {
            if (!strcmp(is->constants[k].name, old->name)) new = &is->constants[k];
        }
        if (!new) {
            return refuse(m, "type '%s' has no constant '%s' here, which it had: a value kept of it "
                "would mean nothing", was->name, old->name);
        }
        if (new->value != old->value) {
            return refuse(m, "type '%s': its constant '%s' is %lld here and was %lld, and every "
                "value kept of it would mean something else", was->name, old->name,
                (long long)new->value, (long long)old->value);
        }
    }
    return 0;
}

/* `was` becomes a type laid out as `is`: how its values are carried over is left on `was`, for
 * the reload to use if it holds. 1 and a refusal if it cannot be, or would lose what this
 * reload does not accept. */
static int relay(module_t *m, declared_type_t *was, enum bridge_kind kind, const schema_t *is)
{
    static const char *const kinds[] = { "", "a struct", "an enum", "a bitmask", "a tag" };
    relaying_t l = { .m = m, .rows = storage_count(m->b->storage, was->type) };
    if (was->kind != kind) {
        return refuse(m, "type '%s' is %s here and was %s: a reload does not change what kind of "
            "type one is", was->name, kinds[kind], kinds[was->kind]);
    }
    if (kind != BRIDGE_STRUCT) return kind == BRIDGE_TAG ? 0 : renumbered(m, was, is);
    if (relaid(&l, &was->s, is, 0, 0, was->name)) {
        free(l.runs);
        return 1;
    }
    if (l.short_of) {
        free(l.runs);
        return refuse(m, "out of memory: there was none to work out how '%s' is laid out again",
            was->name);
    }
    if (l.lost_len) {
        free(l.runs);
        return refuse(m, "type '%s' has another component layout here, and %u rows would lose %s. "
            "A reload that accepts that states each among its losses", was->name, l.rows, l.lost);
    }
    was->runs = l.runs ? l.runs : calloc(1, sizeof *was->runs);
    was->run_count = l.run_count;
    return was->runs ? 0 : refuse(m, "out of memory: there was none to work out how '%s' is laid "
        "out again", was->name);
}

/* whether a field of `c` names a type this reload has made anew */
static bool names_superseded(const declared_type_t *c)
{
    for (uint32_t k = 0; k < c->dep_count; k ++) if (c->deps[k]->superseded) return true;
    return false;
}

/* A loaded module other than the one reloading that declares a type with a field of type
 * `c`: its rows hold values of `c` as it is, which no reload of somebody else may change. */
static const declared_type_t *held_elsewhere(const bridge_t *b, const module_t *m, const declared_type_t *c,
                                           const module_t **by)
{
    for (uint32_t i = 0; i < b->type_count; i ++) {
        const declared_type_t *other = b->types[i];
        uint32_t k, d;
        for (k = 0; k < other->dep_count && other->deps[k] != c; k ++) {}
        if (k == other->dep_count) continue;
        for (const module_t *o = b->modules; o; o = o->next) {
            if (o == m || o == m->reloads) continue;
            for (d = 0; d < o->declared_count && o->declared[d] != other; d ++) {}
            if (d < o->declared_count) {
                *by = o;
                return other;
            }
        }
    }
    return nullptr;
}

/* lifetime.md, *When a component is moved*: `c` is to be kept as `kept` from this reload on.
 * 1 and a refusal for a move that cannot be made, or that loses what was not accepted. */
static int moved_storage(module_t *m, const declared_type_t *c, enum bridge_storage kept)
{
    uint32_t carriers = storage_count(m->b->storage, c->type), off = storage_off(m->b->storage, c->type);
    if (c->storage == BRIDGE_SINGLETON) {
        return refuse(m, "type '%s' is a singleton, and is not moved to %s: there is no entity to "
            "carry it", c->name, storage_names[kept]);
    }
    if (kept == BRIDGE_SINGLETON && carriers > 1) {
        return refuse(m, "type '%s' is carried by %u entities, and is not moved to a singleton: "
            "nothing says which of them has its value", c->name, carriers);
    }
    if (off && !accepted(m, c->name, off)) {
        return refuse(m, "type '%s' is a toggle that is off for %u rows, and as %s they are "
            "carried like the rest. That is a loss this reload does not accept: '%s'", c->name, off,
            storage_names[kept], c->name);
    }
    return 0;
}

/* declare(name, name_len, kind, size, align, desc, desc_len, on_unload, storage) -> slot */
static int declare_stated(void *ud, const uint64_t *a, uint64_t *r)
{
    module_t *m = ud;
    bridge_t *b = m->b;
    const char *name_at = scratch(m, a[0], a[1]), *desc_at = scratch(m, a[5], a[6]);
    enum bridge_kind kind = (enum bridge_kind)a[2];
    enum bridge_on_unload on_unload = (enum bridge_on_unload)a[7];
    enum bridge_storage kept = (enum bridge_storage)a[8];
    uint32_t size = (uint32_t)a[3], align = (uint32_t)a[4], i;
    declared_type_t *c = nullptr, *was = nullptr;
    char name[128], *bare;
    uint64_t print;

    if (!m->in_init) return refuse(m, "declare is called outside init");
    if (!name_at || (!desc_at && kind != BRIDGE_TAG) || !a[1] || a[1] >= sizeof name) {
        return refuse(m, "declare: a name or a descriptor lies outside the scratch arena");
    }
    memcpy(name, name_at, a[1]);
    name[a[1]] = 0;
    {
        /* room for one more of what it declares, and the set of what it may write, before
         * anything is made that would have to be unmade: failure.md, *Out of memory* */
        void *declared = realloc(m->declared, (m->declared_count + 1) * sizeof *m->declared);
        void *policy = realloc(m->policy, (m->declared_count + 1) * sizeof *m->policy);
        void *writes = realloc(m->may_write, (m->declared_count + 1) * sizeof *m->may_write);
        if (declared) m->declared = declared;
        if (policy) m->policy = policy;
        if (writes) m->may_write = writes;
        if (!declared || !policy || !writes || !storage_writable(b->storage, m)) {
            return refuse(m, "out of memory: there was none for what it declares");
        }
    }
    bare = strchr(name, ':');
    if (!bare) return refuse(m, "type '%s' has no namespace", name);
    if (kind < BRIDGE_STRUCT || kind > BRIDGE_TAG) {
        return refuse(m, "type '%s': its kind is unset", name);
    }
    if (kind == BRIDGE_TAG && (size || a[6])) {
        return refuse(m, "type '%s' is a tag, and a tag has no bytes and no descriptor", name);
    }
    if (foreign(m, name) && !listed(m->read, m->read_count, name)
        && !listed(m->write, m->write_count, name)) {
        return refuse(m, "it may not declare '%s': that is another namespace's, and no grant "
            "names it", name);
    }
    if (on_unload < BRIDGE_STRIP || on_unload > BRIDGE_DELETE) {
        return refuse(m, "type '%s': on_unload is unset", name);
    }
    if (kept < BRIDGE_TABLE || kept > BRIDGE_SINGLETON) {
        return refuse(m, "type '%s': its storage layout is unset", name);
    }
    print = fingerprint(kind == BRIDGE_TAG ? "" : desc_at, a[6]) ^ kind;

    for (i = 0; i < b->type_count; i ++) {
        if (!strcmp(b->types[i]->name, name) && !b->types[i]->superseded) c = b->types[i];
    }
    if (c && (c->fingerprint != print || names_superseded(c))) {
        /* lifetime.md, *When a component layout changed*: shared, it is refused, naming who
         * else declares it. Exclusive, and at a reload of the module that declares it, a type
         * is made for what it becomes, which takes the rows if the reload holds */
        char others[160];
        const module_t *by = nullptr;
        const declared_type_t *holder;
        also_declared(b, m, c, others, sizeof others);
        for (i = 0; m->reloads && i < m->reloads->declared_count && m->reloads->declared[i] != c; i ++) {}
        if (others[0] || !m->reloads || i == m->reloads->declared_count) {
            return refuse(m, "type '%s' has another component layout here: fingerprint %016llx, and it "
                "was declared with %016llx%s", name, (unsigned long long)print,
                (unsigned long long)c->fingerprint, others);
        }
        holder = held_elsewhere(b, m, c, &by);
        if (holder) {
            return refuse(m, "type '%s' has another component layout here, and '%s', which module "
                "'%s' declares, has a field of it", name, holder->name, by->identity);
        }
        if (c->storage != kept && moved_storage(m, c, kept)) return 1;
        was = c;
        c = nullptr;
    } else if (c && c->storage != kept) {
        /* a module reads a component by code compiled for one storage layout, so every
         * declarer states the same - storage.md, *Storage layouts* */
        char others[160];
        also_declared(b, m, c, others, sizeof others);
        /* lifetime.md, *When a component is moved*: it is, at a reload of the one module that
         * declares it. A type is made for what it becomes, and takes the rows if the reload
         * holds */
        for (i = 0; m->reloads && i < m->reloads->declared_count && m->reloads->declared[i] != c; i ++) {}
        if (others[0] || !m->reloads || i == m->reloads->declared_count) {
            return refuse(m, "type '%s' is stated as %s here, and it was declared as %s%s", name,
                storage_names[kept], storage_names[c->storage], others);
        }
        if (moved_storage(m, c, kept)) return 1;
        was = c;
        c = nullptr;
    }
    if (was) {
        /* whatever an earlier reload that failed worked out for it is not this one's */
        free(was->runs);
        was->runs = nullptr;
        was->run_count = 0;
    }
    if (!c) {
        /* The descriptor is read and laid out. A field's type is a bare C name, looked for in
         * the declarer's namespace: `Vec3` in game:Body is game:Vec3 - schema.md. */
        char *desc = kind == BRIDGE_TAG ? nullptr : copy(desc_at, a[6]);
        char why[200];
        looking_t l = { b, name };
        schema_t s;
        int refused;
        uint32_t k, held_count = 0;
        declared_type_t **types, **deps;
        char *named;
        void *zero;

        if (kind == BRIDGE_TAG) {
            /* nothing to read: it is what it was stated to be, which is nothing */
            memset(&s, 0, sizeof s);
            s.align = align;
        } else {
            if (!desc) return refuse(m, "out of memory: there was none for the descriptor of '%s'", name);
            *bare = 0;
            refused = schema_parse(kind, desc, find, &l, &s, why, sizeof why);
            *bare = ':';
            free(desc);
            if (refused) return refuse(m, "type '%s': its descriptor %s", name, why);
        }
        if (s.size != size || s.align != align) {
            uint32_t laid = s.size, aligned = s.align;
            schema_free(&s);
            return refuse(m, "type '%s' is %u bytes aligned to %u in the module and %u "
                "aligned to %u by its descriptor", name, size, align, laid, aligned);
        }
        if (was && (was->fingerprint != print || names_superseded(was)) && relay(m, was, kind, &s)) {
            schema_free(&s);
            return 1;
        }
        if (b->type_count >= b->max_types) {
            schema_free(&s);
            return refuse(m, "type '%s' is one more than [budget] types of %u", name,
                b->max_types);
        }

        if (kept == BRIDGE_SPARSE_SET && !storage_sets_free(b->storage)) {
            schema_free(&s);
            return refuse(m, "type '%s' is one more than [budget] sparse_sets of %u", name,
                b->sparse_sets);
        }

        if (kept == BRIDGE_DIRECT_INDEXED && !storage_direct_free(b->storage)) {
            schema_free(&s);
            return refuse(m, "type '%s' is one more than [budget] direct_indexed of %u", name,
                b->direct_indexed);
        }

        /* everything it needs is had before any of it is kept: failure.md, *Out of memory* */
        for (k = 0; k < s.field_count; k ++) held_count += s.fields[k].type != nullptr;
        c = calloc(1, sizeof *c);
        named = copy(name, strlen(name));
        zero = calloc(1, size ? size : 1);
        deps = calloc(held_count ? held_count : 1, sizeof *deps);
        types = realloc(b->types, (b->type_count + 1) * sizeof *b->types);
        if (types) b->types = types;
        if (!c || !named || !zero || !deps || !types) {
            free(c);
            free(named);
            free(zero);
            free(deps);
            schema_free(&s);
            c = nullptr;
            return refuse(m, "out of memory: there was none for type '%s'", name);
        }
        c->type = storage_declare(b->storage, size, align, kept);
        if (!c->type) {
            free(c);
            free(named);
            free(zero);
            free(deps);
            schema_free(&s);
            c = nullptr;
            if (kept == BRIDGE_SINGLETON) {
                return refuse(m, "type '%s' is a singleton, and [budget] columns of %u bytes has "
                    "no room for its %u bytes", name, b->columns_len, size);
            }
            return refuse(m, "type '%s' is direct-indexed, and [budget] columns of %u bytes has no "
                "room for a cell of %u bytes for each of %u entities", name, b->columns_len, size,
                b->entities);
        }
        c->name = named;
        c->s = s;
        c->kind = kind;
        c->storage = kept;
        c->size = size;
        c->align = align;
        c->fingerprint = print;
        c->zero = zero;
        c->deps = deps;
        c->replaces = was;
        if (was) was->superseded = true;
        b->types[b->type_count ++] = c;
        /* a declared type a field of this one names is held for as long as this one is */
        for (k = 0; k < s.field_count; k ++) {
            declared_type_t *held = s.fields[k].type;
            if (!held) continue;
            c->deps[c->dep_count ++] = held;
            held->users ++;
        }
    }

    for (i = 0; i < m->declared_count; i ++) {
        if (m->declared[i] == c) {              /* declare is idempotent */
            r[0] = i;
            return 0;
        }
    }
    m->policy[m->declared_count] = on_unload;
    m->declared[m->declared_count] = c;
    m->may_write[m->declared_count] = may_write(m, c);
    if (m->may_write[m->declared_count]) storage_writes(b->storage, m, c->type);
    c->declarers ++;
    b->layout_moved = 1;
    r[0] = m->declared_count ++;
    return 0;
}

/* storage says who and what; the observer itself runs at the barrier, after the republish */
static void heard(void *ctx, uint64_t id, enum bridge_event event)
{
    bridge_system_t *s = ctx;
    /* its list is as long as what its module's spans could publish of it, and one past
     * that is all that needs knowing: storage_publish finds it does not fit, and says so */
    if (s->pending_count > s->pending_room) return;
    if (s->pending_count && s->pending[s->pending_count - 1].id == id) {
        s->pending[s->pending_count - 1].events |= event;
        return;
    }
    if (s->pending_count == s->pending_room) {
        s->pending_count ++;
        return;
    }
    s->pending[s->pending_count].id = id;
    s->pending[s->pending_count ++].events = event;
}

/* A system or an observer is a query plus a function, so one registration serves both.
 * `events` is zero for a system, and then `phase` is the phase it named. */
static int subscribe(module_t *m, uint64_t name_off, uint64_t name_len, uint32_t phase,
                     enum bridge_event events, uint64_t terms_off, uint64_t term_count,
                     uint64_t changes_off, uint64_t change_count, uint64_t *r)
{
    bridge_t *b = m->b;
    const char *what = events ? "observer" : "system";
    const char *name_at = scratch(m, name_off, name_len);
    const bridge_term_t *terms = scratch(m, terms_off, term_count * sizeof(bridge_term_t));
    const void *changes = change_count ? scratch(m, changes_off, change_count * sizeof(uint32_t))
                                       : "";
    uint32_t count = (uint32_t)term_count, i;
    storage_term_t *asked;
    bridge_system_t *s;
    char name[128], full[256];

    if (b->system_count - b->parked >= b->max_systems) {
        return refuse(m, "it registers one more than [budget] systems of %u", b->max_systems);
    }
    if (term_count > b->max_terms) {
        return refuse(m, "a%s %s asks for %llu terms, which is more than [budget] terms of %u",
            events ? "n" : "", what, (unsigned long long)term_count, b->max_terms);
    }
    if (!name_at || !terms || !changes || !name_len || name_len >= sizeof name) {
        return refuse(m, "%s: a name, a term or a change lies outside the scratch arena", what);
    }
    if (change_count > m->declared_count) {
        return refuse(m, "a%s %s states %llu changes, and the module declared %u types",
            events ? "n" : "", what, (unsigned long long)change_count, m->declared_count);
    }
    memcpy(name, name_at, name_len);
    name[name_len] = 0;
    if (!strcmp(name, "init") || !strcmp(name, "shutdown") || !strcmp(name, "reload")) {
        return refuse(m, "'%s' is a reserved name, and no %s may take it", name, what);
    }
    /* a system and an observer are both exports, and an export has one name */
    for (i = 0; i < m->system_count + m->observer_count; i ++) {
        bridge_system_t *other = i < m->system_count ? m->systems[i]
                                                   : m->observers[i - m->system_count];
        if (!strcmp(other->name, name)) return refuse(m, "'%s' is registered twice", name);
    }
    if (events && !count) {
        return refuse(m, "observer '%s' has no term, and so nothing to observe", name);
    }

    /* everything a system needs is had before any of it is kept: failure.md, *Out of memory* */
    snprintf(full, sizeof full, "%s:%s", m->stem, name);
    s = calloc(1, sizeof *s + count * sizeof *s->term + change_count * sizeof *s->changes);
    asked = calloc(count + 1, sizeof *asked);
    {
        bridge_system_t **all = realloc(b->systems, (b->system_count + 1) * sizeof *b->systems);
        bridge_system_t ***mine = events ? &m->observers : &m->systems;
        uint32_t have = events ? m->observer_count : m->system_count;
        bridge_system_t **own = realloc(*mine, (have + 1) * sizeof **mine);
        if (all) b->systems = all;
        if (own) *mine = own;
        if (s) {
            s->name = copy(name, strlen(name));
            s->full = copy(full, strlen(full));
            if (events) {
                s->pending_room = m->spans_len / (uint32_t)sizeof(bridge_trigger_t);
                s->pending = calloc(s->pending_room ? s->pending_room : 1, sizeof *s->pending);
            }
        }
        if (!s || !asked || !all || !own || !s->name || !s->full || (events && !s->pending)
            || !abi_marks(m, 1)) {
            if (s) {
                free(s->name);
                free(s->full);
                free(s->pending);
            }
            free(s);
            free(asked);
            return refuse(m, "out of memory: there was none for %s '%s'", what, name);
        }
    }
    s->changes = (declared_type_t **)(void *)&s->term[count];
    /* what it attaches or detaches: a write, so it wants the grant a write wants */
    for (i = 0; i < change_count; i ++) {
        uint32_t slot;
        memcpy(&slot, (const char *)changes + i * sizeof slot, sizeof slot);
        if (slot >= m->declared_count) {
            refuse(m, "change %u of %s '%s' names slot %u, which it never declared", i, what, name,
                slot);
        } else if (!may_write(m, m->declared[slot])) {
            refuse(m, "%s '%s' may not attach or detach '%s': no grant says so", what, name,
                m->declared[slot]->name);
        }
        if (m->refused) {
            free(asked);
            free(s);
            return 1;
        }
        s->changes[s->change_count ++] = m->declared[slot];
    }
    /* query.md, *What a query may not ask for* */
    for (i = 0; i < count; i ++) {
        bridge_term_t t;
        declared_type_t *c;
        memcpy(&t, &terms[i], sizeof t);
        if (t.slot >= m->declared_count) {
            refuse(m, "term %u of %s '%s' names slot %u, which it never declared", i, what, name,
                t.slot);
        } else if (!t.access) {
            refuse(m, "term %u of %s '%s': its access is unset", i, what, name);
        } else if (t.access > BRIDGE_WRITE) {
            refuse(m, "term %u of %s '%s': its access is %u, and there is read and write", i, what,
                name, t.access);
        } else if (!t.oper) {
            refuse(m, "term %u of %s '%s': its operator is unset", i, what, name);
        } else if (t.oper > BRIDGE_OPTIONAL) {
            refuse(m, "term %u of %s '%s': its operator is %u, and there are three", i, what, name,
                t.oper);
        } else if (t.access == BRIDGE_WRITE && !may_write(m, m->declared[t.slot])) {
            refuse(m, "%s '%s' may not write '%s': no grant says so", what, name,
                m->declared[t.slot]->name);
        }
        if (m->refused) {
            free(asked);
            free(s);
            return 1;
        }
        c = m->declared[t.slot];
        /* storage.md, *What a query walks*: a system walks tables, or one sparse set alone,
         * and what is kept elsewhere it reaches from the row. An observer walks nothing */
        if (!events && c->storage == BRIDGE_SINGLETON && t.oper != BRIDGE_OPTIONAL) {
            refuse(m, "term %u of %s '%s': '%s' is a singleton, and is on no entity a system "
                "walks. It is stated optional and read where it is", i, what, name, c->name);
            free(asked);
            free(s);
            return 1;
        }
        if (!events && c->storage == BRIDGE_DIRECT_INDEXED && t.oper != BRIDGE_OPTIONAL) {
            refuse(m, "term %u of %s '%s': '%s' is direct-indexed, and does not select what a "
                "system walks. It is stated optional and reached by the entity's id", i, what, name,
                c->name);
            free(asked);
            free(s);
            return 1;
        }
        if (!events && c->storage == BRIDGE_SPARSE_SET && t.oper != BRIDGE_OPTIONAL) {
            for (uint32_t k = 0; k < count; k ++) {
                bridge_term_t other;
                memcpy(&other, &terms[k], sizeof other);
                if (k == i || other.oper == BRIDGE_OPTIONAL) continue;
                refuse(m, "term %u of %s '%s': '%s' is kept in a sparse set, and does not "
                    "select the rows of a table. It is stated optional and looked for at each "
                    "row, or it is the one term that selects", i, what, name, c->name);
                break;
            }
            if (!m->refused && t.oper == BRIDGE_NOT) {
                refuse(m, "term %u of %s '%s': '%s' is kept in a sparse set, and a system "
                    "cannot walk the entities that are not in it", i, what, name, c->name);
            }
            if (m->refused) {
                free(asked);
                free(s);
                return 1;
            }
        }
        asked[i].type = c->type;
        asked[i].oper = t.oper;
        /* a term that must not match touches nothing */
        s->term[i].touches = t.oper == BRIDGE_NOT ? nullptr : c;
        s->term[i].writes = t.access == BRIDGE_WRITE;
    }

    s->matched = events ? storage_observer(b->storage, asked, count, events, heard, s)
               : count ? storage_system(b->storage, asked, count) : nullptr;
    s->asked = asked;
    s->events = events;

    s->m = m;
    s->phase = phase;
    s->terms = count;
    s->observer = events != 0;
    b->systems[b->system_count ++] = s;
    if (events) {
        m->observers[m->observer_count] = s;
        r[0] = m->observer_count ++;
    } else {
        m->systems[m->system_count] = s;
        r[0] = m->system_count ++;
    }
    return 0;
}

/* system(name, name_len, phase, phase_len, terms, term_count, changes, change_count) -> slot.
 * `changes` is the slots of what it attaches or detaches, each a u32. */
static int system_stated(void *ud, const uint64_t *a, uint64_t *r)
{
    module_t *m = ud;
    bridge_t *b = m->b;
    const char *phase_at = scratch(m, a[2], a[3]);
    uint32_t phase;
    if (!m->in_init) return refuse(m, "add_system is called outside init");
    if (!phase_at) return refuse(m, "add_system: a phase lies outside the scratch arena");
    /* what the deployment says this module's name for a phase is, and otherwise the name */
    for (phase = 0; phase < m->phase_names; phase ++) {
        if (strlen(m->phase_from[phase]) == a[3] && !memcmp(m->phase_from[phase], phase_at, a[3])) break;
    }
    if (phase < m->phase_names) return subscribe(m, a[0], a[1], m->phase_to[phase], 0, a[4], a[5], a[6], a[7], r);
    for (phase = 0; phase < b->phase_count; phase ++) {
        if (strlen(b->phases[phase]) == a[3] && !memcmp(b->phases[phase], phase_at, a[3])) break;
    }
    if (phase == b->phase_count) {
        return refuse(m, "a system names phase '%.*s', which the deployment did not declare",
            (int)a[3], phase_at);
    }
    return subscribe(m, a[0], a[1], phase, 0, a[4], a[5], a[6], a[7], r);
}

/* observe(name, name_len, events, terms, term_count, changes, change_count) -> slot. It takes
 * no phase: it has one already, the barrier that applied its trigger. */
static int observer_stated(void *ud, const uint64_t *a, uint64_t *r)
{
    module_t *m = ud;
    if (!m->in_init) return refuse(m, "observe is called outside init");
    if (!a[2] || a[2] & ~(uint64_t)(BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE)) {
        return refuse(m, "observe: its events are unset, or name one there is not");
    }
    return subscribe(m, a[0], a[1], m->b->phase_count, (uint32_t)a[2], a[3], a[4], a[5], a[6], r);
}

/* declare(at) -> slot, add_system(at) -> slot, observe(at) -> slot. Each is handed where one
 * struct is in the scratch arena - abi.md, *The ABI* - which is copied out before it is read,
 * since a module may have put it at any offset. */
static int abi_declare(void *ud, const uint64_t *at, uint64_t *r)
{
    bridge_declaration_t d;
    const void *stated = scratch(ud, at[0], sizeof d);
    if (!stated) return refuse(ud, "declare: what it states lies outside the scratch arena");
    memcpy(&d, stated, sizeof d);
    return declare_stated(ud, (uint64_t[9]){ d.name, d.name_len, d.kind, d.size, d.align, d.desc,
        d.desc_len, d.on_unload, d.storage }, r);
}

static int abi_system(void *ud, const uint64_t *at, uint64_t *r)
{
    bridge_system_statement_t s;
    const void *stated = scratch(ud, at[0], sizeof s);
    if (!stated) return refuse(ud, "add_system: what it states lies outside the scratch arena");
    memcpy(&s, stated, sizeof s);
    return system_stated(ud, (uint64_t[8]){ s.name, s.name_len, s.phase, s.phase_len, s.terms,
        s.term_count, s.changes, s.change_count }, r);
}

static int abi_observe(void *ud, const uint64_t *at, uint64_t *r)
{
    bridge_observer_statement_t o;
    const void *stated = scratch(ud, at[0], sizeof o);
    if (!stated) return refuse(ud, "observe: what it states lies outside the scratch arena");
    memcpy(&o, stated, sizeof o);
    return observer_stated(ud, (uint64_t[7]){ o.name, o.name_len, o.events, o.terms, o.term_count,
        o.changes, o.change_count }, r);
}

/* Nothing below touches storage. Modules run at once, and what one asks for must not depend on
 * who ran beside it - schedule.md, *What a pass depends on*. So an id comes from a block set
 * aside for the module, arena space from its own share, and a change goes on its own queue,
 * which the barrier applies in an order that never varies. No lock: a module's systems are
 * serialised, so each of the three has one writer. */

/* budgets.md, *Budgets*: a reload does not need room for a module's systems twice. The old
 * instance's leave storage before the new one's are registered, and count for nothing until
 * they are back or gone. */
void abi_park(bridge_t *b, bridge_system_t *s)
{
    if (s->parked) return;
    if (s->matched) storage_forget(b->storage, s->matched);
    s->matched = nullptr;
    s->parked = 1;
    b->parked ++;
}

/* As it was: the places it had are free again, since whatever took them has been dropped. */
void abi_unpark(bridge_t *b, bridge_system_t *s)
{
    if (!s->parked) return;
    s->matched = s->events ? storage_observer(b->storage, s->asked, s->terms, s->events, heard, s)
               : s->terms ? storage_system(b->storage, s->asked, s->terms) : nullptr;
    s->parked = 0;
    b->parked --;
}

/* Room for a mark from each of its systems and observers, and from the lifecycle call in
 * progress, beyond those it has: asked for where a module gains a system and before a
 * lifecycle call, so that `queue` never has to. */
bool abi_marks(module_t *m, uint32_t more)
{
    uint32_t want = m->mark_count + m->system_count + m->observer_count + 1 + more;
    void *marks;
    if (want <= m->mark_room) return true;
    marks = realloc(m->marks, want * sizeof *m->marks);
    if (!marks) return false;
    m->marks = marks;
    m->mark_room = want;
    return true;
}

static int queue(module_t *m, enum change_op op, uint64_t id, declared_type_t *c, const void *value)
{
    change_t ch = { id, c, value ? c->size : 0, op };
    uint32_t need = (uint32_t)sizeof ch + ((ch.size + 7u) & ~7u);
    /* budgets.md, *Nothing is allocated in a phase*: its queue is as long as it stated */
    if (!m->queue_room) return refuse(m, "it stated that it queues no change");
    if (need > m->queue_room - m->queue_len) {
        return refuse(m, "its changes of %u bytes are used up", m->queue_room);
    }
    if (!m->mark_count || m->marks[m->mark_count - 1].turn != m->turn) {
        if (m->mark_count == m->mark_room) return refuse(m, "out of memory: there was none for a mark in its queue");
        m->marks[m->mark_count].turn = m->turn;
        m->marks[m->mark_count ++].at = m->queue_len;
    }
    memcpy(m->queue + m->queue_len, &ch, sizeof ch);
    /* the value is copied now: the scratch arena it is in is reset before it is applied */
    if (value) memcpy(m->queue + m->queue_len + sizeof ch, value, ch.size);
    m->queue_len += need;
    return 0;
}

/* spawn() -> id */
static int abi_spawn(void *ud, [[maybe_unused]] const uint64_t *a, uint64_t *r)
{
    module_t *m = ud;
    if (m->id_next == m->id_count) {
        if (m->spawns <= 0) return refuse(m, "spawn: it stated that it spawns nothing");
        if (m->ids_short) {
            /* the index is twelve bytes times the highest id, and that is what was budgeted */
            return refuse(m, "spawn: [budget] entities of %u is reached", m->b->entities);
        }
        return refuse(m, "spawn: its spawns of %lld between two barriers are used up",
            (long long)m->spawns);
    }
    r[0] = m->ids[m->id_next ++];
    return 0;
}

/* destroy(id). An id that has already died is nobody's mistake: two modules may both want it
 * gone, and which of them is told at the barrier, where it is looked at. */
static int abi_destroy(void *ud, const uint64_t *a, [[maybe_unused]] uint64_t *r)
{
    return queue(ud, CHANGE_DESTROY, a[0], nullptr, nullptr);
}

static declared_type_t *written(module_t *m, uint64_t slot, const char *entry)
{
    const bridge_system_t *s = m->running;
    uint32_t i;
    if (slot >= m->declared_count) {
        refuse(m, "%s names slot %llu, which it never declared", entry, (unsigned long long)slot);
        return nullptr;
    }
    if (!m->may_write[slot]) {
        refuse(m, "it may not %s '%s': no grant says so", entry, m->declared[slot]->name);
        return nullptr;
    }
    /* schedule.md, *Order within a phase is never implicit*: what a system changes is what
     * it stated, so that two that change one component were ordered when they loaded. A
     * lifecycle call runs alone, and states nothing */
    for (i = 0; s && i < s->change_count && s->changes[i] != m->declared[slot]; i ++) {}
    if (s && i == s->change_count) {
        refuse(m, "%s '%s' may not %s '%s': it is not among the changes it stated",
            s->observer ? "observer" : "system", s->name, entry, m->declared[slot]->name);
        return nullptr;
    }
    return m->declared[slot];
}

/* attach(id, slot, ptr). The value travels with it, and is as long as the type was declared;
 * BRIDGE_ZEROED as `ptr` asks for a zeroed one. */
/* storage.md, *A singleton*: it is on no entity, so it is given and taken with the id 0,
 * which is no entity; and nothing else is. 1 and a refusal if `id` is the wrong one of the two. */
static int unowned(module_t *m, const char *entry, uint64_t id, const declared_type_t *c)
{
    if (c->storage == BRIDGE_SINGLETON && id) {
        return refuse(m, "%s: '%s' is a singleton, and its id is 0", entry, c->name);
    }
    if (c->storage != BRIDGE_SINGLETON && !id) {
        return refuse(m, "%s: the id 0 is no entity, and '%s' is not a singleton", entry, c->name);
    }
    return 0;
}

static int abi_attach(void *ud, const uint64_t *a, [[maybe_unused]] uint64_t *r)
{
    module_t *m = ud;
    declared_type_t *c = written(m, a[1], "attach");
    const void *value;
    if (!c) return 1;
    value = a[2] != BRIDGE_ZEROED ? scratch(m, a[2], c->size) : c->zero;
    if (!value) return refuse(m, "attach: the value lies outside the scratch arena");
    if (unowned(m, "attach", a[0], c)) return 1;
    return queue(m, CHANGE_ATTACH, a[0], c, value);
}

/* detach(id, slot) */
static int abi_detach(void *ud, const uint64_t *a, [[maybe_unused]] uint64_t *r)
{
    module_t *m = ud;
    declared_type_t *c = written(m, a[1], "detach");
    if (!c || unowned(m, "detach", a[0], c)) return 1;
    return queue(m, CHANGE_DETACH, a[0], c, nullptr);
}

/* alloc(len) -> offset into the persistent arena. Nothing frees it while its module is
 * loaded - strings.md. */
static int abi_alloc(void *ud, const uint64_t *a, uint64_t *r)
{
    module_t *m = ud;
    uint64_t len = (a[0] + 7u) & ~7ull;
    if (m->persistent <= 0) {
        return refuse(m, "alloc: it stated that it keeps nothing in the persistent arena");
    }
    /* what is left of a range too small for this is dead space, as a string that outgrew
     * its place leaves - strings.md */
    while (len > m->arena_left && m->own_next < m->own_count) {
        m->arena_at = m->owned[m->own_next].at;
        m->arena_left = m->owned[m->own_next ++].len;
    }
    if (len > m->arena_left) {
        return refuse(m, "alloc: its persistent of %lld bytes is exhausted", (long long)m->persistent);
    }
    r[0] = m->arena_at;
    m->arena_at += (uint32_t)len;
    m->arena_left -= (uint32_t)len;
    return 0;
}

int abi_define(module_t *m, backend_err_t *err)
{
    constexpr enum backend_type I = BACKEND_I32, L = BACKEND_I64;
    static const struct {
        const char *name;
        backend_fn_t  fn;
        enum backend_type params[9];
        uint32_t    param_count;
        enum backend_type results[1];
        uint32_t    result_count;
    } entries[] = {
        { "declare", abi_declare, { I },                   1, { I }, 1 },
        { "add_system", abi_system, { I },                 1, { I }, 1 },
        { "observe", abi_observe, { I },                   1, { I }, 1 },
        { "spawn",   abi_spawn,   {},                      0, { L }, 1 },
        { "destroy", abi_destroy, { L },                   1, {}, 0 },
        { "attach",  abi_attach,  { L, I, I },             3, {}, 0 },
        { "detach",  abi_detach,  { L, I },                2, {}, 0 },
        { "alloc",   abi_alloc,   { I },                   1, { I }, 1 },
    };
    size_t i;
    for (i = 0; i < sizeof entries / sizeof *entries; i ++) {
        backend_sig_t sig = { .params = entries[i].params, .param_count = entries[i].param_count,
                            .results = entries[i].results,
                            .result_count = entries[i].result_count };
        if (m->on->api.define_func(m->instance, BRIDGE_NAMESPACE, entries[i].name, entries[i].fn,
                                   m, &sig, err)) return 1;
    }
    return 0;
}
