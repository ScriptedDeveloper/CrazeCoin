#pragma once
#include <nlohmann/json.hpp>

namespace broadcast {
	std::string retrieve_peer(int n);
	std::string retrieve_pending(int n);
	int check_block(nlohmann::json jblock);
	int connect_network();
	int get_peers();
	int send_chain(bool is_blockchain, bool is_transaction, std::string ip = "");
	int recieve_chain(bool is_transaction);
	int send_transaction();
	int signup_peer();
	int add_transaction(nlohmann::json jtrans);
	int broadcast_block(std::string block);
	int save_block(nlohmann::json jblock, bool is_transaction);
	int check_emergency_mode();
	int peers_empty();
	nlohmann::json raw_to_json(std::string raw);
	void clear_peers();
	void error_handler(std::string message);
	void fail_emergency_mode();
}

