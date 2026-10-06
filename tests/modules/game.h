/* The schema two test modules share: what a game would publish beside its wasm. */
#ifndef TEST_GAME_H
#define TEST_GAME_H

#define POSITION_FIELDS { float x; float y; }
BRIDGE_SCHEMA_STRUCT_TYPE(Position, POSITION_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(Position, POSITION_FIELDS);
#define MARKED_FIELDS { uint32_t by; }
BRIDGE_SCHEMA_STRUCT_TYPE(Marked, MARKED_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(Marked, MARKED_FIELDS);

#endif
