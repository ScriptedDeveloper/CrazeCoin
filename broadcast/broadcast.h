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
	int signup_peer();
	int save_block(nlohmann::json jblock);
	void clear_peers();
	void error_handler(std::string message);

}

