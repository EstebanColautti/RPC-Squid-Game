#ifndef GAME_H
#define GAME_H

#include "platform.h"
#include "rpc_protocol.h"

#define GAME_BRIDGES 3
#define GAME_MAX_STEPS 32
#define GAME_MAX_PLAYERS 32

typedef enum {
    STEP_STRONG = 1,
    STEP_WEAK = 2
} StepType;

typedef enum {
    STEP_HIDDEN = 0,
    STEP_VISIBLE_STRONG = 1,
    STEP_BROKEN = 2
} StepVisible;

typedef enum {
    PLAYER_UNUSED = 0,
    PLAYER_ACTIVE = 1
} PlayerSlot;

typedef struct {
    uint32_t id;
    char name[RPC_NAME_SIZE];
    int bridge;
    int position;
    int alive;
    int finished;
    int slot;
} Player;

typedef struct {
    int steps;
    int duration_seconds;
    long start_time;
    int expired;
    int survivors;
    uint32_t move_sequence;
    uint32_t next_player_id;
    StepType step_type[GAME_BRIDGES][GAME_MAX_STEPS];
    StepVisible visible[GAME_BRIDGES][GAME_MAX_STEPS];
    Player players[GAME_MAX_PLAYERS];
    mutex_t mutex;
    int mutex_ready;
} GameState;

void game_initialize(GameState *game, int steps, int duration_seconds, unsigned int seed);
void game_destroy(GameState *game);
void game_join(GameState *game, const char *name, RpcResponse *response);
void game_choose_bridge(GameState *game, uint32_t player_id, int bridge, RpcResponse *response);
void game_move(GameState *game, uint32_t player_id, int jump, RpcResponse *response);
void game_get_state(GameState *game, uint32_t player_id, RpcResponse *response);
void game_reset(GameState *game, int steps, int duration_seconds, unsigned int seed, RpcResponse *response);

#endif
