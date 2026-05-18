CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -O2 -Iinclude
LDFLAGS ?=
UNAME_S := $(shell uname -s 2>/dev/null)
ifeq ($(OS),Windows_NT)
    LDLIBS += -lws2_32
else
    LDLIBS += -pthread
endif

BUILD_DIR = build
COMMON = $(BUILD_DIR)/platform.o $(BUILD_DIR)/rpc_protocol.o $(BUILD_DIR)/game.o $(BUILD_DIR)/rpc_client.o $(BUILD_DIR)/rpc_server.o

all: $(BUILD_DIR) $(BUILD_DIR)/server $(BUILD_DIR)/client $(BUILD_DIR)/demo $(BUILD_DIR)/concurrent_demo

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/platform.o: src/platform.c include/platform.h
	$(CC) $(CFLAGS) -c src/platform.c -o $@

$(BUILD_DIR)/rpc_protocol.o: src/rpc_protocol.c include/rpc_protocol.h include/platform.h
	$(CC) $(CFLAGS) -c src/rpc_protocol.c -o $@

$(BUILD_DIR)/game.o: src/game.c include/game.h include/rpc_protocol.h include/platform.h
	$(CC) $(CFLAGS) -c src/game.c -o $@

$(BUILD_DIR)/rpc_client.o: src/rpc_client.c include/rpc_client.h include/rpc_protocol.h include/platform.h
	$(CC) $(CFLAGS) -c src/rpc_client.c -o $@

$(BUILD_DIR)/rpc_server.o: src/rpc_server.c include/rpc_server.h include/game.h include/rpc_protocol.h include/platform.h
	$(CC) $(CFLAGS) -c src/rpc_server.c -o $@

$(BUILD_DIR)/server_main.o: src/server_main.c
	$(CC) $(CFLAGS) -c src/server_main.c -o $@

$(BUILD_DIR)/client_main.o: src/client_main.c
	$(CC) $(CFLAGS) -c src/client_main.c -o $@

$(BUILD_DIR)/demo_main.o: src/demo_main.c
	$(CC) $(CFLAGS) -c src/demo_main.c -o $@

$(BUILD_DIR)/concurrent_demo_main.o: src/concurrent_demo_main.c
	$(CC) $(CFLAGS) -c src/concurrent_demo_main.c -o $@

$(BUILD_DIR)/server: $(COMMON) $(BUILD_DIR)/server_main.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD_DIR)/client: $(COMMON) $(BUILD_DIR)/client_main.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD_DIR)/demo: $(COMMON) $(BUILD_DIR)/demo_main.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD_DIR)/concurrent_demo: $(COMMON) $(BUILD_DIR)/concurrent_demo_main.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
