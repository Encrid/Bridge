/* The schema two test modules share: what a game would publish beside its wasm. */
#ifndef TEST_GAME_H
#define TEST_GAME_H

ECS_STRUCT(Position, { float x; float y; });
ECS_STRUCT(Marked, { uint32_t by; });

#endif
