#include "rpc_client.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>

int rpc_call(const char *host, int port, uint16_t procedure, const RpcRequest *request, RpcResponse *response) {
    socket_t socket_fd;
    RpcHeader header;
    RpcHeader response_header;
    unsigned char header_buffer[RPC_HEADER_SIZE];
    unsigned char request_buffer[RPC_REQUEST_SIZE];
    unsigned char response_buffer[RPC_RESPONSE_SIZE];

    rpc_response_clear(response);
    if (net_startup() != 0) {
        snprintf(response->message, RPC_MESSAGE_SIZE, "Network startup failed.");
        response->status = RPC_STATUS_ERROR;
        return -1;
    }

    socket_fd = net_connect_to(host, port);
    if (socket_fd == INVALID_SOCKET_VALUE) {
        snprintf(response->message, RPC_MESSAGE_SIZE, "Could not connect to %s:%d.", host, port);
        response->status = RPC_STATUS_ERROR;
        return -1;
    }

    header.magic = RPC_MAGIC;
    header.version = RPC_VERSION;
    header.procedure = procedure;
    header.xid = rpc_next_xid();
    header.length = RPC_REQUEST_SIZE;

    rpc_encode_header(&header, header_buffer);
    rpc_encode_request(request, request_buffer);

    if (net_send_all(socket_fd, header_buffer, RPC_HEADER_SIZE) != 0 || net_send_all(socket_fd, request_buffer, RPC_REQUEST_SIZE) != 0) {
        snprintf(response->message, RPC_MESSAGE_SIZE, "Failed to send RPC request.");
        response->status = RPC_STATUS_ERROR;
        net_close(socket_fd);
        return -1;
    }

    if (net_recv_all(socket_fd, header_buffer, RPC_HEADER_SIZE) != 0) {
        snprintf(response->message, RPC_MESSAGE_SIZE, "Failed to receive RPC response header.");
        response->status = RPC_STATUS_ERROR;
        net_close(socket_fd);
        return -1;
    }

    rpc_decode_header(header_buffer, &response_header);
    if (response_header.magic != RPC_MAGIC || response_header.version != RPC_VERSION || response_header.xid != header.xid || response_header.length != RPC_RESPONSE_SIZE) {
        snprintf(response->message, RPC_MESSAGE_SIZE, "Invalid RPC response or mismatched transaction id.");
        response->status = RPC_STATUS_ERROR;
        net_close(socket_fd);
        return -1;
    }

    if (net_recv_all(socket_fd, response_buffer, RPC_RESPONSE_SIZE) != 0) {
        snprintf(response->message, RPC_MESSAGE_SIZE, "Failed to receive RPC response body.");
        response->status = RPC_STATUS_ERROR;
        net_close(socket_fd);
        return -1;
    }

    rpc_decode_response(response_buffer, response);
    net_close(socket_fd);
    return response->status == RPC_STATUS_OK ? 0 : 1;
}

int rpc_join_player(const char *host, int port, const char *name, RpcResponse *response) {
    RpcRequest request;
    rpc_request_clear(&request);
    snprintf(request.name, RPC_NAME_SIZE, "%s", name != NULL ? name : "Player");
    return rpc_call(host, port, RPC_PROC_JOIN, &request, response);
}

int rpc_choose_bridge(const char *host, int port, uint32_t player_id, int bridge, RpcResponse *response) {
    RpcRequest request;
    rpc_request_clear(&request);
    request.player_id = player_id;
    request.value1 = bridge;
    return rpc_call(host, port, RPC_PROC_CHOOSE_BRIDGE, &request, response);
}

int rpc_move_player(const char *host, int port, uint32_t player_id, int jump, RpcResponse *response) {
    RpcRequest request;
    rpc_request_clear(&request);
    request.player_id = player_id;
    request.value1 = jump;
    return rpc_call(host, port, RPC_PROC_MOVE, &request, response);
}

int rpc_get_state(const char *host, int port, uint32_t player_id, RpcResponse *response) {
    RpcRequest request;
    rpc_request_clear(&request);
    request.player_id = player_id;
    return rpc_call(host, port, RPC_PROC_GET_STATE, &request, response);
}

int rpc_reset_game(const char *host, int port, int steps, int duration, RpcResponse *response) {
    RpcRequest request;
    rpc_request_clear(&request);
    request.value1 = steps;
    request.value2 = duration;
    return rpc_call(host, port, RPC_PROC_RESET_GAME, &request, response);
}

int rpc_shutdown_server(const char *host, int port, RpcResponse *response) {
    RpcRequest request;
    rpc_request_clear(&request);
    return rpc_call(host, port, RPC_PROC_SHUTDOWN, &request, response);
}

void print_rpc_response(const char *title, const RpcResponse *response) {
    if (title != NULL && title[0] != '\0') {
        printf("\n=== %s ===\n", title);
    }
    printf("%s\n", response->message);
    printf("player=%u bridge=%d position=%d alive=%d finished=%d survivors=%d clock=%d move_seq=%u\n", response->player_id, response->bridge, response->position, response->alive, response->finished, response->survivors, response->time_left, response->move_sequence);
    if (response->view[0] != '\0') {
        printf("%s", response->view);
    }
}
