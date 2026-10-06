/* The operating system, as far as the native backend needs it: a shared library, opened, looked
 * into and closed, and which thread is calling. It is this backend's own: nothing of
 * Bridge-ASM700's inside is compiled into a backend. */
#ifndef BRIDGE_NATIVE_OS_H
#define BRIDGE_NATIVE_OS_H

#include <stdint.h>

[[nodiscard]] void *os_lib_open(const char *path);                /* nullptr on failure */
[[nodiscard]] void *os_lib_sym(void *lib, const char *name);
void  os_lib_close(void *lib);

uint64_t os_thread_id();                    /* of the calling thread: never 0 */

#endif
