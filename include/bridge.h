/* The host seam: the only header a host includes.
 * docs/seam/host-api/host-api.md is the contract. This is as much of it as exists so far:
 * create, destroy and load. Phases, unload, reload and the pulls arrive with what they act on. */
#ifndef BRIDGE_H
#define BRIDGE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct bridge bridge;

/* A backend the deployment configures: Bridge-ASM700 opens the library, and a module names
 * the backend that runs it. */
typedef struct {
    const char *name;
    const char *library;
} bridge_backend;

typedef struct {
    const bridge_backend *backends; uint32_t backend_count;
} bridge_config;

enum { BRIDGE_FAIL = 1, BRIDGE_SKIP = 2 };      /* on_error. zero is unset */
#define BRIDGE_NO_LIMIT (-1)                    /* a limit stated as none */

typedef struct {
    const char *identity;        /* "tri:triangle@0.1.0" */
    uint32_t    abi;

    bool     trusted;
    uint8_t  on_error;           /* BRIDGE_FAIL | BRIDGE_SKIP */
    /* each of the next two: zero is unset, BRIDGE_NO_LIMIT is "no limit" */
    int64_t  memory_limit;       /* bytes. a ceiling, never re-set */
    int64_t  init_timeout;       /* nanoseconds */
} bridge_module_config;

typedef struct {
    bridge_module_config cfg;
    const char *backend;             /* which backend runs it, by name */
    const void *bytes; size_t len;   /* the image, or null */
    const char *path;                /* where it is, or null. both go to the backend */
} bridge_artifact;

/* what comes back out */
typedef struct {
    const char *identity;            /* null for Bridge-ASM700's own records */
    const char *system;              /* null outside a system */
    const char *text;                /* THIS CALL ONLY */
    uint32_t    len;
    uint32_t    when;
    uint32_t    arg[2];
    int8_t      level;               /* negative is worse - flecs' convention */
} bridge_record;

enum { BRIDGE_WARN = -2, BRIDGE_ERR = -3 };

typedef void (*bridge_sink_fn)(void *ud, const bridge_record *r);

/* NULL if the deployment cannot start; which reason, the sink was told. */
bridge *bridge_create(const bridge_config *cfg, bridge_sink_fn sink, void *ud);
void    bridge_destroy(bridge *b);

/* 0 if the module is loaded and its init answered 0. Otherwise the load was refused or
 * failed, and a record says why. */
int bridge_load(bridge *b, const bridge_artifact *a);

#endif
