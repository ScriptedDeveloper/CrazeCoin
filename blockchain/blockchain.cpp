/*
CrazeCoin, a semi-decentralised work-in-progress CryptoCurrency
Copyright (C) 2022  ScriptedDev
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
	const std::string peer_tracker = "192.168.10.101:6881"; // changing later to correct domain
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
	clear_peers();
	signup_peer();
	get_peers(); // Connecting to other peers
	if(blockchain::is_blockchain_empty()) {
		recieve_chain();
	} else {
		send_chain();
	}
}

void create_json(std::string name) {
	std::ofstream ofs(name);
	ofs << "{}";
	ofs.close();
}

void check_files () {
	std::ifstream ifsPeer(blockchain::peer_path);
	if(is_empty(ifsPeer)) {
		std::cout << "\n Peer required files not found, creating..." << std::endl;
		clear_peers();
		std::cout << "\nDone!" << std::endl;
	}
	try {
		ifsPeer.open(blockchain::peer_path);
		nlohmann::json j1 = j1.parse(ifsPeer);
		ifsPeer.close();
	}
	catch (...){
		std::cout << "Corrupt files found! Cleaning up..." << std::endl;
		std::remove(blockchain::path.c_str());
		std::remove(blockchain::peer_path.c_str());
		check_files();
	}
	ifsPeer.close();
}



int main() {
	check_files();
	blockchain::init_blockchain();	// Calling start of blockchain
	//blockchain::generate_genesis_block("ASD");
	return 0;
}
