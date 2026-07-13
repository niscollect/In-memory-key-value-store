void append_to_wal(ServerState& state, string command, string key, string value);
void load_wal(ServerState &state);