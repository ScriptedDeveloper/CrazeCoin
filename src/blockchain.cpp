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
#include <cryptopp/rsa.h>
#include <experimental/filesystem>
#include <cryptopp/filters.h>
#include <cryptopp/pssr.h>
#include <nlohmann/json.hpp>
#include <vector>
#include "../include/blockchain.h"
//#include <libtorrent/torrent_handle.hpp>
#include "../include/block.h"
#include "../include/rsa.h"
#include "../include/broadcast.h"

namespace blockchain {
	std::string path = std::experimental::filesystem::current_path().u8string() + "/blockchain.json";
	std::string torrent_file = std::experimental::filesystem::current_path().u8string() + "/discovery.torrent";
	std::string peer_path = std::experimental::filesystem::current_path().u8string() + "/peers.json";
	const std::string peer_tracker = "192.168.10.104:6882"; // changing later to correct domain
}

bool blockchain::is_empty(std::ifstream &ifS) {
	return ifS.peek() == std::ifstream::traits_type::eof();
}

block blockchain::generate_genesis_block(std::string data) {
	block b("0", data);
	b.index = 0;
	b.add_block();
	return b;
}

bool blockchain::is_blockchain_empty() {
	std::ifstream ifChain(blockchain::path);
	auto ss = std::stringstream();
	ss << ifChain.rdbuf();
	ifChain.close();
	if(ss.str() != "{}" && !ss.str().empty()) {
		return false;
	}
	return (bool)is_empty(ifChain);
}

int blockchain::check_chain() {
	nlohmann::json jchain = blockchain::blockchain_json();
	int blocks = jchain["blocks"];
	for(int i = 0; i < blocks; i++) {
		if(broadcast::check_block(jchain[std::to_string(blocks)]) != 0) {
			return 1; // some block has failed the check, blockchain is compromised!
		}
	}
	return 0;
}

int blockchain::block_number() {
	std::ifstream ifschain(blockchain::path);
	nlohmann::json jchain = jchain.parse(ifschain);
	return jchain["blocks"];
}



nlohmann::json blockchain::blockchain_json() {
	std::ifstream ifschain(blockchain::path);
	nlohmann::json jchain = jchain.parse(ifschain);
	return jchain;
}

int blockchain::verify_transaction(nlohmann::json j) {
	CryptoPP::RSA::PublicKey publkey;
	std::vector<std::string> raw_vector, hex_vector = {"signature", "send_addr"};
	std::string timestamp = j["timestamp"], amount = j["amount"] , reciever = j["recieve_addr"]; // JSON dump function acting weird
	for(int i = 0; i < hex_vector.size(); i++) {
		raw_vector.push_back(rsa_wrapper::raw_hex_decode(j[hex_vector[i]]));
	}
	CryptoPP::StringSource ss(raw_vector[1], true);
	publkey.Load(ss); // loads sender's address (public key) to variable
	std::string data = reciever + "/" + amount + "/" + timestamp;
	CryptoPP::RSASSA_PKCS1v15_SHA_Verifier verifier(publkey);
	bool verify_trans = verifier.VerifyMessage((const CryptoPP::byte*)data.c_str(), data.length(), (const CryptoPP::byte*)raw_vector[0].c_str(), raw_vector[0].size());
	return (int)verify_trans;
}

void blockchain::init_blockchain() {
	if(broadcast::signup_peer() == 0) { // if server doesnt respond, skip
 		broadcast::clear_peers(); // clearing only if server is available
		broadcast::get_peers(); // Connecting to other peers
	}
	if(blockchain::is_blockchain_empty() || blockchain::check_chain() == 1) { // checks also if blockchain is valid
		std::ofstream ofs(blockchain::path); // clearing blockchain content in case check_chain == 1
		broadcast::recieve_chain(false);
	} else {
		//broadcast::send_chain(true, false); // is original peer/miner, broadcasting blockchain
		broadcast::recieve_chain(true); // changing for debugging
	}
}

void blockchain::create_json(std::string name) {
	std::ofstream ofs(name);
	ofs << "{}";
	ofs.close();
}

void blockchain::check_files () {
	std::ifstream ifsPeer(blockchain::peer_path);
	if(blockchain::is_empty(ifsPeer)) {
		std::cout << "\n Peer required files not found, creating..." << std::endl;
		broadcast::clear_peers();
		std::cout << "\nDone!" << std::endl;
	}
	try {
		ifsPeer.open(blockchain::peer_path);
		nlohmann::json j1 = j1.parse(ifsPeer);
		ifsPeer.close();
	}
	catch (...){
		std::cout << "Corrupt files found! Cleaning up..." << std::endl;
		std::remove(blockchain::peer_path.c_str());
		check_files();
	}
	ifsPeer.close();
}


int blockchain::add_transaction(nlohmann::json jtransaction) {
	if(verify_transaction(jtransaction) == 1) {
		return 1; // verifying transaction failed
	}

	return 0;
}

/* might be useful for later
int blockchain::check_transaction_format(std::string format, nlohmann::json j) { // parses string to check for values
	std::vector<std::string> vector_format;
	std::string current_member; // specifies member of vector, e.g amount
	for(int i = 0; format[i] != '/'; i++) {
		current_member += format[i];
		format.erase(0, 1);
	}
	format.erase(0, 1); // erasing backslash
	vector_format.push_back(current_member);
	current_member.clear();
	if(!format.empty()) {
		check_transaction_format(format, j); // parsing remaining members
	}
	for(std::string member : vector_format) {
		for(std::string j_member : j) {
			if(j_member != member) {
				return 1; // invalid data sent to miner, aborting
			}
		}
	}
	return 0;
}
*/
