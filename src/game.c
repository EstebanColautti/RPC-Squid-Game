#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static long now_seconds(void) {
    return (long)time(NULL);
}

static void safe_copy(char *destination, size_t size, const char *source) {
    if (size == 0) {
        return;
    }
    if (source == NULL) {
        destination[0] = '\0';
        return;
    }
    snprintf(destination, size, "%s", source);
}

static void append_text(char *buffer, size_t size, const char *text) {
    size_t used = strlen(buffer);
    if (used < size - 1) {
        snprintf(buffer + used, size - used, "%s", text);
    }
}

static int remaining_time_unlocked(GameState *game) {
    long elapsed = now_seconds() - game->start_time;
    int left = game->duration_seconds - (int)elapsed;
    return left > 0 ? left : 0;
}

static Player *find_player_unlocked(GameState *game, uint32_t player_id) {
    int i;
    for (i = 0; i < GAME_MAX_PLAYERS; i++) {
        if (game->players[i].slot == PLAYER_ACTIVE && game->players[i].id == player_id) {
            return &game->players[i];
        }
    }
    return NULL;
}

static void expire_if_needed_unlocked(GameState *game) {
    int i;
    if (game->expired) {
        return;
    }
    if (remaining_time_unlocked(game) > 0) {
        return;
    }
    game->expired = 1;
    for (i = 0; i < GAME_MAX_PLAYERS; i++) {
        Player *player = &game->players[i];
        if (player->slot == PLAYER_ACTIVE && player->alive && !player->finished) {
            player->alive = 0;
        }
    }
}

static void fill_common_response_unlocked(GameState *game, Player *player, RpcResponse *response) {
    response->time_left = remaining_time_unlocked(game);
    response->survivors = game->survivors;
    response->move_sequence = game->move_sequence;
    if (player != NULL) {
        response->player_id = player->id;
        response->bridge = player->bridge;
        response->position = player->position;
        response->alive = player->alive;
        response->finished = player->finished;
    }
}

static void build_view_unlocked(GameState *game, char *output, size_t output_size) {
    char line[256];
    int bridge;
    int step;
    int i;
    output[0] = '\0';

    snprintf(line, sizeof(line), "Clock: %ds | Survivors: %d | Move sequence: %u\n", remaining_time_unlocked(game), game->survivors, game->move_sequence);
    append_text(output, output_size, line);

    for (bridge = 0; bridge < GAME_BRIDGES; bridge++) {
        snprintf(line, sizeof(line), "Bridge %d: ", bridge + 1);
        append_text(output, output_size, line);
        for (step = 0; step < game->steps; step++) {
            const char *token = "[?]";
            if (game->visible[bridge][step] == STEP_VISIBLE_STRONG) {
                token = "[S]";
            } else if (game->visible[bridge][step] == STEP_BROKEN) {
                token = "[X]";
            }
            append_text(output, output_size, token);
        }
        append_text(output, output_size, "\n");
    }

    append_text(output, output_size, "Players:\n");
    for (i = 0; i < GAME_MAX_PLAYERS; i++) {
        Player *player = &game->players[i];
        if (player->slot == PLAYER_ACTIVE) {
            const char *state = "dead";
            if (player->finished) {
                state = "finished";
            } else if (player->alive) {
                state = "alive";
            }
            snprintf(line, sizeof(line), "  #%u %s | bridge=%d | position=%d | %s\n", player->id, player->name, player->bridge, player->position, state);
            append_text(output, output_size, line);
        }
    }
}

static void finish_response_unlocked(GameState *game, Player *player, RpcResponse *response) {
    fill_common_response_unlocked(game, player, response);
    build_view_unlocked(game, response->view, RPC_VIEW_SIZE);
}

static int any_player_ahead_unlocked(GameState *game, const Player *player) {
    int i;
    if (player->bridge < 1) {
        return 0;
    }
    for (i = 0; i < GAME_MAX_PLAYERS; i++) {
        const Player *other = &game->players[i];
        if (other->slot == PLAYER_ACTIVE && other->id != player->id && other->alive && !other->finished && other->bridge == player->bridge && other->position > player->position) {
            return 1;
        }
    }
    return 0;
}

void game_initlize(GameState *game, int steps, int duration_seconds, unsigned int seed) {
    int bridge;
    int step;

    if (!game->mutex_ready) {
        mutex_init(&game->mutex);
        game->mutex_ready = 1;
    }

    mutex_lock(&game->mutex);

    if (steps < 3) {
        steps = 3;
    }
    if (steps > GAME_MAX_STEPS) {
        steps = GAME_MAX_STEPS;
    }
    if (duration_seconds < 10) {
        duration_seconds = 10;
    }

    game->steps = steps;
    game->duration_seconds = duration_seconds;
    game->start_time = now_seconds();
    game->expired = 0;
    game->survivors = 0;
    game->move_sequence = 0;
    game->next_player_id = 1;
    memset(game->players, 0, sizeof(game->players));

    srand(seed);
    for (bridge = 0; bridge < GAME_BRIDGES; bridge++) {
        for (step = 0; step < GAME_MAX_STEPS; step++) {
            int pattern = (int)((seed + (unsigned int)(bridge + 1) * 17u + (unsigned int)(step + 1) * 31u) % 5u);
            game->step_type[bridge][step] = pattern == 0 ? STEP_WEAK : STEP_STRONG;
            game->visible[bridge][step] = STEP_HIDDEN;
        }
    }

    if (steps >= 10) {
        StepType b1[10] = {STEP_STRONG, STEP_WEAK, STEP_STRONG, STEP_STRONG, STEP_WEAK, STEP_STRONG, STEP_STRONG, STEP_WEAK, STEP_STRONG, STEP_STRONG};
        StepType b2[10] = {STEP_STRONG, STEP_STRONG, STEP_WEAK, STEP_STRONG, STEP_STRONG, STEP_WEAK, STEP_STRONG, STEP_STRONG, STEP_WEAK, STEP_STRONG};
        StepType b3[10] = {STEP_WEAK, STEP_STRONG, STEP_STRONG, STEP_WEAK, STEP_STRONG, STEP_STRONG, STEP_WEAK, STEP_STRONG, STEP_STRONG, STEP_STRONG};
        for (step = 0; step < 10; step++) {
            game->step_type[0][step] = b1[step];
            game->step_type[1][step] = b2[step];
            game->step_type[2][step] = b3[step];
        }
    }

    mutex_unlock(&game->mutex);
}

void game_destroy(GameState *game) {
    if (game->mutex_ready) {
        mutex_destroy(&game->mutex);
        game->mutex_ready = 0;
    }
}

void game_join(GameState *game, const char *name, RpcResponse *response) {
    int i;
    Player *created = NULL;

    rpc_response_clear(response);
    mutex_lock(&game->mutex);
    expire_if_needed_unlocked(game);

    if (game->expired) {
        response->status = RPC_STATUS_ERROR;
        safe_copy(response->message, RPC_MESSAGE_SIZE, "The global clock has already run out. No new players can join.");
        finish_response_unlocked(game, NULL, response);
        mutex_unlock(&game->mutex);
        return;
    }

    for (i = 0; i < GAME_MAX_PLAYERS; i++) {
        if (game->players[i].slot == PLAYER_UNUSED) {
            created = &game->players[i];
            break;
        }
    }

    if (created == NULL) {
        response->status = RPC_STATUS_ERROR;
        safe_copy(response->message, RPC_MESSAGE_SIZE, "The server reached the maximum number of players.");
        finish_response_unlocked(game, NULL, response);
        mutex_unlock(&game->mutex);
        return;
    }

    created->slot = PLAYER_ACTIVE;
    created->id = game->next_player_id++;
    safe_copy(created->name, RPC_NAME_SIZE, name != NULL && name[0] != '\0' ? name : "Player");
    created->bridge = 0;
    created->position = 0;
    created->alive = 1;
    created->finished = 0;

    response->status = RPC_STATUS_OK;
    snprintf(response->message, RPC_MESSAGE_SIZE, "%s joined the game with id #%u.", created->name, created->id);
    finish_response_unlocked(game, created, response);
    mutex_unlock(&game->mutex);
}

void game_choose_bridge(GameState *game, uint32_t player_id, int bridge, RpcResponse *response) {
    Player *player;

    rpc_response_clear(response);
    mutex_lock(&game->mutex);
    expire_if_needed_unlocked(game);
    player = find_player_unlocked(game, player_id);

    if (player == NULL) {
        response->status = RPC_STATUS_ERROR;
        safe_copy(response->message, RPC_MESSAGE_SIZE, "Unknown player id.");
        finish_response_unlocked(game, NULL, response);
        mutex_unlock(&game->mutex);
        return;
    }

    if (!player->alive || player->finished || game->expired) {
        response->status = RPC_STATUS_ERROR;
        safe_copy(response->message, RPC_MESSAGE_SIZE, "This player cannot choose a bridge anymore.");
        finish_response_unlocked(game, player, response);
        mutex_unlock(&game->mutex);
        return;
    }

    if (bridge < 1 || bridge > GAME_BRIDGES) {
        response->status = RPC_STATUS_ERROR;
        safe_copy(response->message, RPC_MESSAGE_SIZE, "Invalid bridge. Choose bridge 1, 2, or 3.");
        finish_response_unlocked(game, player, response);
        mutex_unlock(&game->mutex);
        return;
    }

    if (player->bridge != 0) {
        response->status = RPC_STATUS_ERROR;
        safe_copy(response->message, RPC_MESSAGE_SIZE, "The player already chose a bridge and cannot switch.");
        finish_response_unlocked(game, player, response);
        mutex_unlock(&game->mutex);
        return;
    }

    player->bridge = bridge;
    response->status = RPC_STATUS_OK;
    snprintf(response->message, RPC_MESSAGE_SIZE, "%s entered bridge %d.", player->name, bridge);
    finish_response_unlocked(game, player, response);
    mutex_unlock(&game->mutex);
}

void game_move(GameState *game, uint32_t player_id, int jump, RpcResponse *response) {
    Player *player;
    int target;
    int bridge_index;
    int step_index;

    rpc_response_clear(response);
    mutex_lock(&game->mutex);
    expire_if_needed_unlocked(game);
    player = find_player_unlocked(game, player_id);

    if (player == NULL) {
        response->status = RPC_STATUS_ERROR;
        safe_copy(response->message, RPC_MESSAGE_SIZE, "Unknown player id.");
        finish_response_unlocked(game, NULL, response);
        mutex_unlock(&game->mutex);
        return;
    }

    if (game->expired) {
        response->status = RPC_STATUS_ERROR;
        safe_copy(response->message, RPC_MESSAGE_SIZE, "The clock ran out. Remaining players died.");
        finish_response_unlocked(game, player, response);
        mutex_unlock(&game->mutex);
        return;
    }

    if (!player->alive) {
        response->status = RPC_STATUS_ERROR;
        safe_copy(response->message, RPC_MESSAGE_SIZE, "Dead players cannot move.");
        finish_response_unlocked(game, player, response);
        mutex_unlock(&game->mutex);
        return;
    }

    if (player->finished) {
        response->status = RPC_STATUS_ERROR;
        safe_copy(response->message, RPC_MESSAGE_SIZE, "This player already reached Side B.");
        finish_response_unlocked(game, player, response);
        mutex_unlock(&game->mutex);
        return;
    }

    if (player->bridge < 1) {
        response->status = RPC_STATUS_ERROR;
        safe_copy(response->message, RPC_MESSAGE_SIZE, "Choose a bridge before moving.");
        finish_response_unlocked(game, player, response);
        mutex_unlock(&game->mutex);
        return;
    }

    if (jump != 1 && jump != 2) {
        response->status = RPC_STATUS_ERROR;
        safe_copy(response->message, RPC_MESSAGE_SIZE, "Invalid jump. A player can jump only 1 or 2 steps.");
        finish_response_unlocked(game, player, response);
        mutex_unlock(&game->mutex);
        return;
    }

    if (any_player_ahead_unlocked(game, player)) {
        response->status = RPC_STATUS_ERROR;
        safe_copy(response->message, RPC_MESSAGE_SIZE, "A player is ahead on this bridge, so this player must wait.");
        finish_response_unlocked(game, player, response);
        mutex_unlock(&game->mutex);
        return;
    }

    target = player->position + jump;
    game->move_sequence++;

    if (target > game->steps) {
        player->position = game->steps + 1;
        player->finished = 1;
        game->survivors++;
        response->status = RPC_STATUS_OK;
        snprintf(response->message, RPC_MESSAGE_SIZE, "%s reached Side B and survived.", player->name);
        finish_response_unlocked(game, player, response);
        mutex_unlock(&game->mutex);
        return;
    }

    bridge_index = player->bridge - 1;
    step_index = target - 1;
    player->position = target;

    if (game->step_type[bridge_index][step_index] == STEP_WEAK) {
        game->visible[bridge_index][step_index] = STEP_BROKEN;
        player->alive = 0;
        response->status = RPC_STATUS_OK;
        snprintf(response->message, RPC_MESSAGE_SIZE, "%s jumped to bridge %d step %d. The step was weak and broke. The player died.", player->name, player->bridge, target);
    } else {
        game->visible[bridge_index][step_index] = STEP_VISIBLE_STRONG;
        response->status = RPC_STATUS_OK;
        snprintf(response->message, RPC_MESSAGE_SIZE, "%s jumped to bridge %d step %d. The step was strong.", player->name, player->bridge, target);
    }

    finish_response_unlocked(game, player, response);
    mutex_unlock(&game->mutex);
}

void game_get_state(GameState *game, uint32_t player_id, RpcResponse *response) {
    Player *player;

    rpc_response_clear(response);
    mutex_lock(&game->mutex);
    expire_if_needed_unlocked(game);
    player = find_player_unlocked(game, player_id);
    response->status = RPC_STATUS_OK;
    safe_copy(response->message, RPC_MESSAGE_SIZE, "Current visible game state.");
    finish_response_unlocked(game, player, response);
    mutex_unlock(&game->mutex);
}

void game_reset(GameState *game, int steps, int duration_seconds, unsigned int seed, RpcResponse *response) {
    rpc_response_clear(response);
    game_initlize(game, steps, duration_seconds, seed);
    mutex_lock(&game->mutex);
    response->status = RPC_STATUS_OK;
    safe_copy(response->message, RPC_MESSAGE_SIZE, "The game was reset.");
    finish_response_unlocked(game, NULL, response);
    mutex_unlock(&game->mutex);
}
