#pragma once
#include <fstream>
#include <nlohmann/json.hpp>

namespace broadcast {
	std::string retrieve_peer(int n);
	std::string retrieve_pending(int n);
	std::vector<std::string> get_local_ips();
	int check_block(nlohmann::json jblock);
	int connect_network();
	std::pair<int, nlohmann::json> get_peers();
	int send_chain(bool is_blockchain, bool is_transaction, std::string ip = "", std::string data = "");
	int recieve_chain(bool is_transaction);
	int send_transaction();
	int signup_peer();
	int check_local_ip(std::string target_ip);
	void recieve_chain_thread_handler();
	int unsign_pend_peer();
	void send_chain_thread_handler();
	int fail_emergency_mode();
	int unsign_miner();
	int pend_to_miner(std::string ip);
	int add_transaction(nlohmann::json jtrans);
	int broadcast_block(std::string block);
	int save_block(nlohmann::json jblock, bool is_transaction, bool is_recieved_block);
	int check_emergency_mode();
	int peers_empty();
	template<typename input>
	nlohmann::json raw_to_json(input raw) {
		nlohmann::json j_data;
		int retries = 0;
		if(!raw.empty()) {
			while(true) {
				try {
					try {	
						j_data = j_data.parse(raw);
						break;
					} catch(...) {
						raw.pop_back(); // trying to remove garbage character and try again
						if(retries == 400) {
							return 1; // parsing failed, char array is not valid JSON
						}
					}
				} catch(...) {
					return 1;
				}
			}
		}
		return j_data;
	}
	void clear_peers();
	std::string error_handler(std::string message);
}

