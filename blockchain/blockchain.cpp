#include <iostream>
#include <fstream>
#include <ios>
#include <chrono>
#include <string>
#include <cstdio>
#include <experimental/filesystem>
#include <nlohmann/json.hpp>
#include "blockchain.h"
#include <libtorrent/torrent_handle.hpp>
#include "../block/block.h"
#include "../broadcast/broadcast.h"

namespace blockchain {
	std::string path = std::experimental::filesystem::current_path().u8string() + "/blockchain.json";
	std::string torrent_file = std::experimental::filesystem::current_path().u8string() + "/discovery.torrent";
	std::string peer_path = std::experimental::filesystem::current_path().u8string() + "/peers.json";
}

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
	return (bool)is_empty(ifChain);
}

void blockchain::init_blockchain() {
	clear_peers();
	get_peers(); // Connecting to other peers
	if(blockchain::is_blockchain_empty()) {
		recieve_chain();
	}
	else {
		send_chain();
	}
	
}

void check_files () {
	if(!std::experimental::filesystem::exists(blockchain::path) || !std::experimental::filesystem::exists(blockchain::peer_path)) {
		std::cout << "\nRequired files not found, creating..." << std::endl;
		std::ofstream ofsBlock(blockchain::path);
		std::ofstream ofsPeer(blockchain::peer_path);
		ofsPeer << "{}";
		ofsBlock.close();
		ofsPeer.close();
		std::cout << "Done!" << std::endl;
	}
	try {	
		std::ifstream ifsBlock(blockchain::path);
		std::ifstream ifsPeer(blockchain::peer_path);
		nlohmann::json j = j.parse(ifsBlock);
		nlohmann::json j1 = j1.parse(ifsPeer);
		ifsBlock.close();
		ifsPeer.close();
	} 
	catch (...){
		std::cout << "Corrupt files found! Cleaning up..." << std::endl;
		std::remove(blockchain::path.c_str());
		std::remove(blockchain::peer_path.c_str());
		check_files();
	}
}

int main() {
	check_files();
	blockchain::init_blockchain();	// Calling start of blockchain
	return 0;
}
