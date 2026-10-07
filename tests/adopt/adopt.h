/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/adopt/adopt.h -- one block, adopted by the wasm backend, backing a
** memory in two stores at once. This is the question stage 2 opens with:
** Bridge-ASM700 allocates the shared memory and every module imports it,
** each from a store of its own. wasmtime does not promise that one block
** may back a memory twice, so it is tried here before anything is built on
** it.
**
** wasmtime.md, *Memory*
*/

#ifndef TEST_ADOPT_H
    #define TEST_ADOPT_H

    #include "backend.h"
    #include "os.h"

constexpr uint32_t ADOPT_PAGE = 65'536;

/*
** What the checks share: the backend, the block it adopted, the module
** every store instantiates, and why the last call of adopt_run failed.
*/
typedef struct {
    backend_api_t api;
    backend_err_t err;
    backend_t *be;
    backend_module_t *module;
    backend_memory_t *memory;
    unsigned char *block;
    size_t reserved;
} adopt_t;

/*
** A thread's work: a function of the module called `rounds` times in a
** store of the thread's own, and how many calls failed.
*/
typedef struct {
    adopt_t *t;
    const char *name;
    int rounds;
    int failed;
} adopt_job_t;

/*
** adopt_read_file
**
** Answers the bytes of a file and how many. The program ends, saying so,
** if it cannot be read.
*/
void *adopt_read_file(const char *path, size_t *len);

/*
** adopt_make_instance
**
** Answers a store of the module with the block as its memory. The program
** ends, saying so, if it is refused.
*/
backend_instance_t *adopt_make_instance(adopt_t *t);

/*
** adopt_run
**
** Calls a function of an instance by name. 0 and the function's result, or
** -1 and why, in `t->err`.
*/
int adopt_run(adopt_t *t, backend_instance_t *i, const char *name,
    int32_t *status);

/*
** adopt_fail
**
** Says that a check failed, as printf would. Answers 1.
*/
[[gnu::format(printf, 1, 2)]] int adopt_fail(const char *format, ...);

/*
** The checks. Each answers 1, having said why, if it failed.
*/
int adopt_open(adopt_t *t);
int adopt_check_write(adopt_t *t, backend_instance_t *one);
int adopt_check_two_stores(adopt_t *t, backend_instance_t *one);
int adopt_check_stray(adopt_t *t, backend_instance_t *one);
int adopt_check_host_function(adopt_t *t);
int adopt_check_timed_calls(adopt_t *t);
int adopt_check_functions(adopt_t *t, backend_instance_t *one);
int adopt_check_refusals(adopt_t *t, backend_instance_t *one);
int adopt_check_native_entry(void);

/*
** adopt_say_refused
**
** Says why a call was refused. Answers 1, having said so, if it was
** accepted.
*/
int adopt_say_refused(const char *what, bool refused,
    const backend_err_t *err);

#endif
