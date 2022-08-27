#pragma once
#ifndef __unix__
	#error "UNIX ONLY, WINDOWS NOT SUPPORTED!"
#endif
#include "block.h"

namespace blockchain {
	extern const std::string path;
	extern const std::string peer_path;
	extern const std::string peer_tracker;
	extern const int max_transactions;
	block generate_genesis_block();	
	std::string retrieve_addr();
	bool is_blockchain_empty();
	std::pair<bool, nlohmann::json> verify_transaction(nlohmann::json j);
	int block_number();
	int check_chain();
	int get_transaction_num(std::string block_num);
	std::string get_previous_hash(bool last_block);
	int check_balances(std::string addr);
	int add_block(nlohmann::json jblock);
	void init_blockchain();
	void create_json(std::string name);
	bool is_empty(std::ifstream &ifS);
	void check_files();
	std::string get_hash();
	nlohmann::json blockchain_json();
};
