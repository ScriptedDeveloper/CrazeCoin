#pragma once
#include <iostream>
#include <fstream>
#include <experimental/filesystem>
#include "../block/block.h"

namespace blockchain {
	static std::string path = std::experimental::filesystem::current_path().u8string() + "/blockchain.json";
	static block generate_genesis_block(std::string data);	
	bool is_blockchain_empty();
	void init_blockchain();
}
