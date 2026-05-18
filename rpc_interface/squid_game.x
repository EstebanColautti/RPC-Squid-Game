const NAME_SIZE = 32;
const MESSAGE_SIZE = 256;
const VIEW_SIZE = 1024;

struct join_in {
    char name[NAME_SIZE];
};

struct choose_bridge_in {
    unsigned int player_id;
    int bridge;
};

struct move_in {
    unsigned int player_id;
    int jump;
};

struct state_in {
    unsigned int player_id;
};

struct game_out {
    int status;
    unsigned int player_id;
    int bridge;
    int position;
    int alive;
    int finished;
    int survivors;
    int time_left;
    unsigned int move_sequence;
    char message[MESSAGE_SIZE];
    char view[VIEW_SIZE];
};

program SQUID_GAME_PROG {
    version SQUID_GAME_VERS {
        game_out JOIN_PLAYER(join_in) = 1;
        game_out CHOOSE_BRIDGE(choose_bridge_in) = 2;
        game_out MOVE_PLAYER(move_in) = 3;
        game_out GET_STATE(state_in) = 4;
    } = 1;
} = 0x31234567;
