#include <iostream>
#include <ios>
#include <chrono>
#include <string>
#include <experimental/filesystem>
#include <nlohmann/json.hpp>
#include "../block/block.h"
#include "blockchain.h"

bool is_empty(std::ifstream &ifS) {
	return ifS.peek() == std::ifstream::traits_type::eof();
}

static block blockchain::generate_genesis_block(std::string data) {
	static block b("0", data);
	std::cout << b.add_block(b);
	return b;
}

bool blockchain::is_blockchain_empty() {
	std::ifstream ifChain(blockchain::path);
	if(is_empty(ifChain)) {
		return true;
	}
	return false;
}

void init_blockchain() {
	if(blockchain::is_blockchain_empty()) {
		//writing l8ter
	}
}

int main() {
	// Calling start of blockchain
	init_blockchain();
	return 0;
}
