#pragma once
#ifndef __unix__
	#error "UNIX ONLY, WINDOWS NOT SUPPORTED!"
#endif
#include <iostream>
#include <fstream>
#include <experimental/filesystem>
#include "../block/block.h"

namespace blockchain {
	extern std::string torrent_file; // The BitTorrent Protocol is used for Peer Discovery
	extern std::string path;
	extern std::string peer_path;
	extern const std::string peer_tracker;
	static block generate_genesis_block(std::string data);	
	bool is_blockchain_empty();
	int block_number();
	int check_chain();
	void init_blockchain();
	void create_json(std::string name);
	bool is_empty(std::ifstream &ifS);
	void check_files();
	nlohmann::json blockchain_json();
};
