#include "rpc_server.h"
#include "rpc_client.h"
#include "rpc_protocol.h"
#include "platform.h"
#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONCURRENT_PLAYERS 8

typedef struct {
    int index;
    const char *host;
    int port;
} DemoClientArgs;

static mutex_t print_mutex;

static void print_line_safe(const char *player, const char *message) {
    mutex_lock(&print_mutex);
    printf("[%s] %s\n", player, message);
    mutex_unlock(&print_mutex);
}

static THREAD_RETURN scripted_client(THREAD_ARG argument) {
    DemoClientArgs *args = (DemoClientArgs *)argument;
    char name[RPC_NAME_SIZE];
    RpcResponse response;
    uint32_t player_id;
    int bridge;
    int i;

    snprintf(name, sizeof(name), "P%d", args->index + 1);
    bridge = (args->index % GAME_BRIDGES) + 1;

    rpc_join_player(args->host, args->port, name, &response);
    player_id = response.player_id;
    print_line_safe(name, response.message);

    rpc_choose_bridge(args->host, args->port, player_id, bridge, &response);
    print_line_safe(name, response.message);

    for (i = 0; i < 5; i++) {
        int jump = ((args->index + i) % 2) + 1;
        sleep_ms(70 + args->index * 25);
        rpc_move_player(args->host, args->port, player_id, jump, &response);
        print_line_safe(name, response.message);
        if (!response.alive || response.finished || response.time_left == 0) {
            break;
        }
    }

#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

int main(void) {
    const char *host = "127.0.0.1";
    int port = 5052;
    RpcServerConfig config;
    RpcResponse response;
    thread_t server_thread;
    thread_t clients[CONCURRENT_PLAYERS];
    DemoClientArgs args[CONCURRENT_PLAYERS];
    int i;

    mutex_init(&print_mutex);
    config.port = port;
    config.steps = 10;
    config.duration_seconds = 60;
    config.seed = 42u;

    printf("Starting local RPC server for concurrent demo...\n");
    if (rpc_server_start_background(&config, &server_thread) != 0) {
        printf("Could not start background server.\n");
        mutex_destroy(&print_mutex);
        return 1;
    }

    sleep_ms(500);

    for (i = 0; i < CONCURRENT_PLAYERS; i++) {
        args[i].index = i;
        args[i].host = host;
        args[i].port = port;
        thread_start(&clients[i], scripted_client, &args[i]);
    }

    for (i = 0; i < CONCURRENT_PLAYERS; i++) {
        thread_join(clients[i]);
    }

    rpc_get_state(host, port, 0, &response);
    print_rpc_response("FINAL CONCURRENT STATE", &response);
    rpc_shutdown_server(host, port, &response);
    thread_join(server_thread);
    mutex_destroy(&print_mutex);
    return 0;
}
