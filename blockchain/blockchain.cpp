#include <iostream>
#include <fstream>
#include <ios>
#include <chrono>
#include <string>
#include <cstdio>
#include <experimental/filesystem>
#include <nlohmann/json.hpp>
#include "blockchain.h"
//#include <libtorrent/torrent_handle.hpp>
#include "../block/block.h"
#include "../broadcast/broadcast.h"

namespace blockchain {
	std::string path = std::experimental::filesystem::current_path().u8string() + "/blockchain.json";
	std::string torrent_file = std::experimental::filesystem::current_path().u8string() + "/discovery.torrent";
	std::string peer_path = std::experimental::filesystem::current_path().u8string() + "/peers.json";
	const std::string peer_tracker = "192.168.178.111:8086";
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
	auto ss = std::stringstream();
	ss << ifChain.rdbuf();
	if(ss.str() != "{}") {
		return false;
	}
	return (bool)is_empty(ifChain);
}

void blockchain::init_blockchain() {
	//clear_peers();
	//get_peers(); // Connecting to other peers
	if(blockchain::is_blockchain_empty()) {
		recieve_chain();
	} else {
		send_chain();
	}
}

void check_files () {
	std::ifstream ifsBlock(blockchain::path);
	std::ifstream ifsPeer(blockchain::peer_path);
	if(is_empty(ifsBlock)) {
		std::cout << blockchain::path;
		std::cout << "\n Blockchain required files not found, creating..." << std::endl;
		std::ofstream ofsBlock(blockchain::path);
		ofsBlock << "{}";
		std::cout << "\nDone!" << std::endl;
	}
	if(is_empty(ifsPeer)) {
		std::cout << "\n Peer required files not found, creating..." << std::endl;
		clear_peers();
		std::cout << "\nDone!" << std::endl;
	}
	try {	
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
	ifsPeer.close();
	ifsBlock.close();
}

int main() {
	check_files();
	blockchain::init_blockchain();	// Calling start of blockchain
	return 0;
}
