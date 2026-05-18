#include "rpc_server.h"
#include "rpc_protocol.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    RpcServerConfig config;
    config.port = RPC_DEFAULT_PORT;
    config.steps = 10;
    config.duration_seconds = 90;
    config.seed = 42u;

    if (argc > 1) {
        config.port = atoi(argv[1]);
    }
    if (argc > 2) {
        config.steps = atoi(argv[2]);
    }
    if (argc > 3) {
        config.duration_seconds = atoi(argv[3]);
    }

    return rpc_server_start(&config);
}
