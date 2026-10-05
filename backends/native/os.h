/* The operating system, as far as the native backend needs it: a shared library, opened, looked
 * into and closed. It is this backend's own: nothing of Bridge-ASM700's inside is compiled into
 * a backend. */
#ifndef BRIDGE_NATIVE_OS_H
#define BRIDGE_NATIVE_OS_H

[[nodiscard]] void *os_lib_open(const char *path);                /* nullptr on failure */
[[nodiscard]] void *os_lib_sym(void *lib, const char *name);
void  os_lib_close(void *lib);

#endif
