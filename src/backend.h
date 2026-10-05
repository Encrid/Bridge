#ifndef BRIDGE_SRC_BACKEND_H
#define BRIDGE_SRC_BACKEND_H

#include "bridge-backend.h"

/* Open a backend library and find its seventeen entries by name. Returns the library, or nullptr
 * with `why` saying which name was missing or that the library would not open. */
void *backend_open(const char *path, backend_api *api, char *why, size_t why_len);

#endif
