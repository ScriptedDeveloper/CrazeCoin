#pragma once
#include <nlohmann/json.hpp>

namespace broadcast {
	std::string retrieve_peer(int n);
	std::string retrieve_pending(int n);
	int check_block(nlohmann::json jblock);
	int connect_network();
	int get_peers();
	int send_chain(bool is_blockchain);
	int recieve_chain();
	int send_transaction();
	int signup_peer();
	int save_block(nlohmann::json jblock);
	int check_emergency_mode();
	int peers_empty();
	void clear_peers();
	void error_handler(std::string message);
	void fail_emergency_mode();
}

