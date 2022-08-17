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
#include <cctype>
#include <chrono>
#include <string>
#include <cstdio>
#include <cryptopp/rsa.h>
#include <experimental/filesystem>
#include <cryptopp/filters.h>
#include <cryptopp/pssr.h>
#include <nlohmann/json.hpp>
#include <thread>
#include <vector>
#include "../include/blockchain.h"
#include "../include/block.h"
#include "../include/rsa.h"
#include "../include/broadcast.h"

namespace blockchain {
	const std::string path = std::experimental::filesystem::current_path().u8string() + "/blockchain.json";
	const std::string peer_path = std::experimental::filesystem::current_path().u8string() + "/peers.json";
	const std::string addr_path = std::experimental::filesystem::current_path().u8string() + "/address.bin";
	const std::string peer_tracker = "192.168.10.101:6882"; // changing later to correct domain
	const int max_transactions = 3; // max transactions in a block
}

bool blockchain::is_empty(std::ifstream &ifS) {
	return ifS.peek() == std::ifstream::traits_type::eof();
}

block blockchain::generate_genesis_block() {
	block b("0", retrieve_addr(), "0", 20); // 20 coins will be sent to miner's wallet
	b.add_block();
	return b;
}

int blockchain::add_block(nlohmann::json jblock) {
	block b(get_previous_hash(false), jblock["recieve_addr"], jblock["send_addr"], jblock["amount"]);
	b.add_block();
	return 0;
}

std::string blockchain::retrieve_addr() {
	std::ifstream ifs(addr_path);
	std::stringstream ss_addr;
	ss_addr << ifs.rdbuf();
	return ss_addr.str();
}

bool blockchain::is_blockchain_empty() {
	std::ifstream ifschain(blockchain::path);
	return (bool)is_empty(ifschain);
}

int blockchain::check_chain() {
	nlohmann::json jchain = blockchain::blockchain_json();
	int blocks = jchain["blocks"];
	for(int i = 0; i < blocks; i++) {
		if((nlohmann::json)broadcast::check_block(jchain[std::to_string(blocks)]) != 0) {
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

int blockchain::check_balances(std::string addr) {
	std::ifstream ifschain(path);
	nlohmann::json jchain = jchain.parse(ifschain);
	int blocks = block_number() + 1;
	int balance = -1; // balance of wallet
	for(int i = 1; i < blocks; i++) {
		std::string str_i = std::to_string(i);
		if(jchain[str_i]["recieve_addr"] == addr) {
			balance = jchain[str_i]["amount"];
		}  else if(jchain[str_i]["send_addr"] == addr && balance == -1) {
			std::cout << "[SYSTEM] Error! Blockchain is invalid! Address " << addr << " is sending coins without having coins!" << std::endl;
			exit(1); // balance is not initialized, blockchain is corrupted!
		} else if(jchain[str_i]["send_addr"] == addr) {
			balance = balance - (int)jchain[str_i]["amount"];
		}	
	}
	return balance;
}

nlohmann::json blockchain::blockchain_json() {
	nlohmann::json jchain;
	try {
		 std::ifstream ifschain(blockchain::path);
		jchain = jchain.parse(ifschain);
	} catch(...) {
		return "";
	}
	return jchain;
}

bool blockchain::verify_transaction(nlohmann::json j) {
	CryptoPP::RSA::PublicKey publkey;
	std::vector<std::string> raw_vector, hex_vector = {"signature", "send_addr"};
	std::string timestamp = j["timestamp"];
	std::string amount = std::to_string((int)j["amount"]);
	std::string reciever = j["recieve_addr"];
	//std::string timestamp = std::to_string((int)j["timestamp"]), amount = std::to_string((int)j["amount"]) , reciever = j["recieve_addr"]; // JSON dump function acting weird
	for(int i = 0; i < hex_vector.size(); i++) {
		raw_vector.push_back(rsa_wrapper::raw_hex_decode(j[hex_vector[i]]));
	}
	CryptoPP::StringSource ss(raw_vector[1], true);
	publkey.Load(ss); // loads sender's address (public key) to variable
	std::string data = reciever + "/" + amount + "/" + timestamp;
	CryptoPP::RSASSA_PKCS1v15_SHA_Verifier verifier(publkey);
	bool verify_trans = verifier.VerifyMessage((const CryptoPP::byte*)data.c_str(), data.length(), (const CryptoPP::byte*)raw_vector[0].c_str(), raw_vector[0].size());
	if(verify_trans) {
		return check_balances(j["send_addr"]);
	}
	return verify_trans;
}

void blockchain::init_blockchain() {
	generate_genesis_block();
	if(broadcast::signup_peer() == 0) { // if server doesnt respond, skip
		broadcast::get_peers(); // Connecting to other peers
	}
	if(blockchain::is_blockchain_empty() || blockchain::check_chain() == 1) { // checks also if blockchain is valid
		std::ofstream ofs(blockchain::path); // clearing blockchain content in case check_chain == 1
		broadcast::recieve_chain(false);
	} else {
		while(true) {
			broadcast::recieve_chain(true);
			/*
			std::thread broadcaster(broadcast::send_chain, true, false); // is original peer/miner, broadcasting blockchain
			std::thread transaction_recieve(broadcast::recieve_chain, true); // changing for debugging
			broadcaster.join();
			transaction_recieve.join();
			*/
		}
	}
}

void blockchain::create_json(std::string name) {
	std::ofstream ofs(name);
	ofs << "{}";
	ofs.close();
}

std::string blockchain::get_previous_hash(bool last_block) {
	std::ifstream ifschain(blockchain::path);
	nlohmann::json jchain;
	try {
		jchain = jchain.parse(ifschain);
	} catch(...) {
		std::cout << "Blockchain is corrupted!" << std::endl;
		exit(1); // somethings wrong with the blockchain
	}
	if(last_block) {
		return jchain[std::to_string((int)jchain["blocks"])]["hash"];
	}
	return jchain[std::to_string((int)jchain["blocks"])]["previous_hash"];
}

void blockchain::check_files () {
	std::ifstream ifsaddr(addr_path), ifsPeer(peer_path);
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
	if(is_empty(ifsaddr)) {
		std::ofstream ofsaddr(addr_path);
		std::string addr;
		std::cout << "Seems like you are new here. Please input your wallet address : " << std::endl;
		std::cin >> addr;
		ofsaddr << addr;
		ofsaddr.close();
	}
	ifsPeer.close();
}

int blockchain::add_transaction(nlohmann::json jtransaction) {
	if(!verify_transaction(jtransaction)) {
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
