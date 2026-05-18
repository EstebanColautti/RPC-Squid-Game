#include "rpc_server.h"
#include "rpc_client.h"
#include "rpc_protocol.h"
#include "platform.h"
#include <stdio.h>

static uint32_t join_and_choose(const char *host, int port, const char *name, int bridge) {
    RpcResponse response;
    rpc_join_player(host, port, name, &response);
    print_rpc_response("JOIN", &response);
    uint32_t id = response.player_id;
    rpc_choose_bridge(host, port, id, bridge, &response);
    print_rpc_response("CHOOSE", &response);
    return id;
}

static void move_and_print(const char *host, int port, uint32_t player_id, int jump, const char *title) {
    RpcResponse response;
    rpc_move_player(host, port, player_id, jump, &response);
    print_rpc_response(title, &response);
}

int main(void) {
    const char *host = "127.0.0.1";
    int port = 5051;
    RpcServerConfig config;
    RpcResponse response;
    thread_t server_thread;

    config.port = port;
    config.steps = 10;
    config.duration_seconds = 90;
    config.seed = 42u;

    printf("Starting local RPC server for automatic demo...\n");
    if (rpc_server_start_background(&config, &server_thread) != 0) {
        printf("Could not start background server.\n");
        return 1;
    }

    sleep_ms(500);

    uint32_t gihun = join_and_choose(host, port, "Gi-hun", 2);
    uint32_t ali = join_and_choose(host, port, "Ali", 2);
    uint32_t saebyeok = join_and_choose(host, port, "Sae-byeok", 1);

    move_and_print(host, port, gihun, 1, "GI-HUN MOVE 1");
    move_and_print(host, port, gihun, 1, "GI-HUN MOVE 2");
    move_and_print(host, port, ali, 1, "ALI MUST WAIT");
    move_and_print(host, port, saebyeok, 2, "SAE-BYEOK TESTS WEAK STEP");
    move_and_print(host, port, gihun, 2, "GI-HUN JUMPS OVER WEAK STEP");
    move_and_print(host, port, gihun, 1, "GI-HUN MOVE 5");
    move_and_print(host, port, gihun, 2, "GI-HUN JUMPS OVER STEP 6");
    move_and_print(host, port, gihun, 1, "GI-HUN MOVE 8");
    move_and_print(host, port, gihun, 2, "GI-HUN MOVE 10");
    move_and_print(host, port, gihun, 1, "GI-HUN REACHES SIDE B");

    rpc_get_state(host, port, gihun, &response);
    print_rpc_response("FINAL STATE", &response);
    rpc_shutdown_server(host, port, &response);
    thread_join(server_thread);
    return 0;
}
