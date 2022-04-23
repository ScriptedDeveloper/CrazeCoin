#include <iostream>
#include <ios>
#include <chrono>
#include <string>
#include <experimental/filesystem>
#include <nlohmann/json.hpp>
#include "blockchain.h"
#include <libtorrent/torrent_handle.hpp>
#include "../block/block.h"
#include "../broadcast/broadcast.h"

bool is_empty(std::ifstream &ifS) {
	return ifS.peek() == std::ifstream::traits_type::eof();
}

block blockchain::generate_genesis_block(std::string data) {
	block b("0", data);
	b.add_block(b);
	return b;
}

bool blockchain::is_blockchain_empty() {
	std::ifstream ifChain(blockchain::path);
	if(is_empty(ifChain)) {
		return true;
	}
	return false;
}

void blockchain::init_blockchain() {
	if(blockchain::is_blockchain_empty()) {
		blockchain::generate_genesis_block("GENESIS");
	}

	
}

int main() {
	// Calling start of blockchain
	//blockchain::init_blockchain();
	lt::torrent_handle t = connect_network();
	print_peers(t);
	return 0;
}
