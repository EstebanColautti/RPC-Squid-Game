#include "rpc_protocol.h"
#include "platform.h"
#include <string.h>
#include <time.h>

static uint32_t g_xid_counter = 1;

static void put_u16(unsigned char *buffer, size_t *offset, uint16_t value) {
    buffer[(*offset)++] = (unsigned char)((value >> 8) & 0xffu);
    buffer[(*offset)++] = (unsigned char)(value & 0xffu);
}

static void put_u32(unsigned char *buffer, size_t *offset, uint32_t value) {
    buffer[(*offset)++] = (unsigned char)((value >> 24) & 0xffu);
    buffer[(*offset)++] = (unsigned char)((value >> 16) & 0xffu);
    buffer[(*offset)++] = (unsigned char)((value >> 8) & 0xffu);
    buffer[(*offset)++] = (unsigned char)(value & 0xffu);
}

static uint16_t get_u16(const unsigned char *buffer, size_t *offset) {
    uint16_t value = (uint16_t)(((uint16_t)buffer[*offset] << 8) | buffer[*offset + 1]);
    *offset += 2;
    return value;
}

static uint32_t get_u32(const unsigned char *buffer, size_t *offset) {
    uint32_t value = ((uint32_t)buffer[*offset] << 24) |
                     ((uint32_t)buffer[*offset + 1] << 16) |
                     ((uint32_t)buffer[*offset + 2] << 8) |
                     (uint32_t)buffer[*offset + 3];
    *offset += 4;
    return value;
}

void rpc_request_clear(RpcRequest *request) {
    memset(request, 0, sizeof(*request));
}

void rpc_response_clear(RpcResponse *response) {
    memset(response, 0, sizeof(*response));
}

uint32_t rpc_next_xid(void) {
    uint32_t value = (uint32_t)time(NULL) ^ platform_process_id() ^ g_xid_counter++;
    if (value == 0) {
        value = g_xid_counter++;
    }
    return value;
}

void rpc_encode_header(const RpcHeader *header, unsigned char *buffer) {
    size_t offset = 0;
    put_u32(buffer, &offset, header->magic);
    put_u16(buffer, &offset, header->version);
    put_u16(buffer, &offset, header->procedure);
    put_u32(buffer, &offset, header->xid);
    put_u32(buffer, &offset, header->length);
}

int rpc_decode_header(const unsigned char *buffer, RpcHeader *header) {
    size_t offset = 0;
    header->magic = get_u32(buffer, &offset);
    header->version = get_u16(buffer, &offset);
    header->procedure = get_u16(buffer, &offset);
    header->xid = get_u32(buffer, &offset);
    header->length = get_u32(buffer, &offset);
    return 0;
}

void rpc_encode_request(const RpcRequest *request, unsigned char *buffer) {
    size_t offset = 0;
    put_u32(buffer, &offset, request->player_id);
    put_u32(buffer, &offset, (uint32_t)request->value1);
    put_u32(buffer, &offset, (uint32_t)request->value2);
    memcpy(buffer + offset, request->name, RPC_NAME_SIZE);
}

int rpc_decode_request(const unsigned char *buffer, RpcRequest *request) {
    size_t offset = 0;
    request->player_id = get_u32(buffer, &offset);
    request->value1 = (int32_t)get_u32(buffer, &offset);
    request->value2 = (int32_t)get_u32(buffer, &offset);
    memcpy(request->name, buffer + offset, RPC_NAME_SIZE);
    request->name[RPC_NAME_SIZE - 1] = '\0';
    return 0;
}

void rpc_encode_response(const RpcResponse *response, unsigned char *buffer) {
    size_t offset = 0;
    put_u32(buffer, &offset, (uint32_t)response->status);
    put_u32(buffer, &offset, response->player_id);
    put_u32(buffer, &offset, (uint32_t)response->bridge);
    put_u32(buffer, &offset, (uint32_t)response->position);
    put_u32(buffer, &offset, (uint32_t)response->alive);
    put_u32(buffer, &offset, (uint32_t)response->finished);
    put_u32(buffer, &offset, (uint32_t)response->survivors);
    put_u32(buffer, &offset, (uint32_t)response->time_left);
    put_u32(buffer, &offset, response->move_sequence);
    put_u32(buffer, &offset, (uint32_t)response->result_code);
    memcpy(buffer + offset, response->message, RPC_MESSAGE_SIZE);
    offset += RPC_MESSAGE_SIZE;
    memcpy(buffer + offset, response->view, RPC_VIEW_SIZE);
}

int rpc_decode_response(const unsigned char *buffer, RpcResponse *response) {
    size_t offset = 0;
    response->status = (int32_t)get_u32(buffer, &offset);
    response->player_id = get_u32(buffer, &offset);
    response->bridge = (int32_t)get_u32(buffer, &offset);
    response->position = (int32_t)get_u32(buffer, &offset);
    response->alive = (int32_t)get_u32(buffer, &offset);
    response->finished = (int32_t)get_u32(buffer, &offset);
    response->survivors = (int32_t)get_u32(buffer, &offset);
    response->time_left = (int32_t)get_u32(buffer, &offset);
    response->move_sequence = get_u32(buffer, &offset);
    response->result_code = (int32_t)get_u32(buffer, &offset);
    memcpy(response->message, buffer + offset, RPC_MESSAGE_SIZE);
    response->message[RPC_MESSAGE_SIZE - 1] = '\0';
    offset += RPC_MESSAGE_SIZE;
    memcpy(response->view, buffer + offset, RPC_VIEW_SIZE);
    response->view[RPC_VIEW_SIZE - 1] = '\0';
    return 0;
}
