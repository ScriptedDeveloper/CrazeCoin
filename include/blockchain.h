#pragma once
#ifndef __unix__
	#error "UNIX ONLY, WINDOWS NOT SUPPORTED!"
#endif
#include "block.h"

namespace blockchain {
	extern const std::string path;
	extern const std::string peer_path;
	extern const std::string peer_tracker;
	block generate_genesis_block();	
	std::string retrieve_addr();
	bool is_blockchain_empty();
	bool verify_transaction(nlohmann::json j);
	int block_number();
	int check_chain();
	int add_transaction(nlohmann::json jtransaction);
	int check_balances(std::string addr);
	void init_blockchain();
	void create_json(std::string name);
	bool is_empty(std::ifstream &ifS);
	void check_files();
	nlohmann::json blockchain_json();
};
