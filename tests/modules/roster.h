/* What the keeper and the counter share: a position, and a list of ids to ask after. */
#ifndef TEST_ROSTER_H
#define TEST_ROSTER_H

#define POSITION_FIELDS { float x; float y; }
BRIDGE_SCHEMA_STRUCT_TYPE(Position, POSITION_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(Position, POSITION_FIELDS);
#define PRIVATE_FIELDS { uint32_t secret; }
BRIDGE_SCHEMA_STRUCT_TYPE(Private, PRIVATE_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(Private, PRIVATE_FIELDS);
#define ROSTER_FIELDS { uint64_t ids[5]; }
BRIDGE_SCHEMA_STRUCT_TYPE(Roster, ROSTER_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(Roster, ROSTER_FIELDS);

#endif
