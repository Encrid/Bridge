/* What the keeper and the counter share: a position, and a list of ids to ask after. */
#ifndef TEST_ROSTER_H
#define TEST_ROSTER_H

BRIDGE_SCHEMA_STRUCT(Position, { float x; float y; });
BRIDGE_SCHEMA_STRUCT(Private, { uint32_t secret; });
BRIDGE_SCHEMA_STRUCT(Roster, { uint64_t ids[5]; });

#endif
