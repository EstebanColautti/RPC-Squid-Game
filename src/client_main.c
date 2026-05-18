#include "rpc_client.h"
#include "rpc_protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void trim_newline(char *text) {
    size_t length = strlen(text);
    if (length > 0 && text[length - 1] == '\n') {
        text[length - 1] = '\0';
    }
}

int main(int argc, char **argv) {
    const char *host = argc > 1 ? argv[1] : "127.0.0.1";
    int port = argc > 2 ? atoi(argv[2]) : RPC_DEFAULT_PORT;
    char name[RPC_NAME_SIZE];
    char line[64];
    int bridge;
    uint32_t player_id;
    RpcResponse response;

    printf("RPC Squid Game Client\n");
    printf("Server: %s:%d\n", host, port);
    printf("Player name: ");
    if (fgets(name, sizeof(name), stdin) == NULL) {
        return 1;
    }
    trim_newline(name);
    if (name[0] == '\0') {
        snprintf(name, sizeof(name), "Player");
    }

    if (rpc_join_player(host, port, name, &response) < 0) {
        print_rpc_response("JOIN FAILED", &response);
        return 1;
    }
    player_id = response.player_id;
    print_rpc_response("JOIN", &response);

    printf("Choose bridge 1, 2, or 3: ");
    if (fgets(line, sizeof(line), stdin) == NULL) {
        return 1;
    }
    bridge = atoi(line);
    rpc_choose_bridge(host, port, player_id, bridge, &response);
    print_rpc_response("CHOOSE BRIDGE", &response);

    while (1) {
        printf("\nMenu: 1=jump one step, 2=jump two steps, s=state, q=quit\n> ");
        if (fgets(line, sizeof(line), stdin) == NULL) {
            break;
        }
        trim_newline(line);
        if (strcmp(line, "q") == 0 || strcmp(line, "Q") == 0) {
            break;
        }
        if (strcmp(line, "s") == 0 || strcmp(line, "S") == 0) {
            rpc_get_state(host, port, player_id, &response);
            print_rpc_response("STATE", &response);
            continue;
        }
        if (strcmp(line, "1") == 0 || strcmp(line, "2") == 0) {
            rpc_move_player(host, port, player_id, atoi(line), &response);
            print_rpc_response("MOVE", &response);
            if (!response.alive || response.finished || response.time_left == 0) {
                printf("Round ended for this player.\n");
                break;
            }
            continue;
        }
        printf("Invalid option.\n");
    }

    return 0;
}
