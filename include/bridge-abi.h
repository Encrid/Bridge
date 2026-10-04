/* The module seam: what Bridge-ASM700 and every module both compile.
 * docs/modules/impl/wasm.md, *The published layouts*. It begins with what phase 2 needs. */
#ifndef BRIDGE_ABI_H
#define BRIDGE_ABI_H

/* A module states the version it was built against; one built against another is refused. */
#define BRIDGE_ABI 1

/* Everything a module imports comes from one namespace; the two memories go by name. */
#define BRIDGE_NAMESPACE "bridge"
#define BRIDGE_SHARED    "shared"
#define BRIDGE_PRIVATE   "private"

#endif
