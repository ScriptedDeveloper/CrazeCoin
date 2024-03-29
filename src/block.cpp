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

#include <cryptopp/files.h>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <sstream>
#include <string>
#include <iomanip>
#include <chrono>
#include <boost/algorithm/string.hpp>
#include <nlohmann/json.hpp>
#include <cryptopp/sha.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include "../include/blockchain.h"
#include "../include/broadcast.h"
#include "../include/block.h"

block::block(std::string previous_hash, std::string recieve_addr, std::string send_addr, int amount){
	this->previous_hash = previous_hash;
	this->nounce = 0;
	this->difficulty = 4; // for now
	this->recieve_addr = recieve_addr;
	this->send_addr = send_addr;
	this->amount = amount;
}

std::string block::generate_hash(std::string plain_text) {
	CryptoPP::SHA256 hash;
	std::string str_hash;
	CryptoPP::HexEncoder encoder(new CryptoPP::StringSink(str_hash));
	std::string digest;
	hash.Update((const CryptoPP::byte*)plain_text.data(), plain_text.size());
	digest.resize(hash.DigestSize());
	hash.Final((CryptoPP::byte*)&digest[0]);
	CryptoPP::StringSource(digest, true, new CryptoPP::Redirector(encoder));
	return boost::algorithm::to_lower_copy(str_hash);
}

std::string block::mine_block() {
	std::string difficulty_str;
	for(int i = 0; i < this->difficulty; i++) {
		difficulty_str.append("0");
	}
	while(this->hash.rfind(difficulty_str, 0) != 0) {
		this->nounce++;
		this->hash = generate_hash(this->previous_hash + this->data + std::to_string(this->nounce) + this->timestamp);
	}
	std::cout << this->previous_hash + this->data + std::to_string(this->nounce) + this->timestamp << std::endl;
	return this->hash;
}

std::string block::verify_block() {
	return generate_hash(this->previous_hash + this->data + std::to_string(this->nounce) + this->timestamp);
}

std::string block::get_timestamp() {
	auto time = std::chrono::system_clock::now();
	return std::to_string(std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch()).count());

}

std::string block::get_merkle_root() {
	nlohmann::json jchain = blockchain::blockchain_json();
	return jchain["merkle_root"];
}

int block::create_block_file(nlohmann::json j) {
	std::ofstream ofsblock("block.json");
	ofsblock << j;
	return 0;
}

nlohmann::json block::set_finished_data(nlohmann::json j, std::string index_str) {
	try {
		this->merkle_root = get_merkle_root();

	} catch(...) {
		// ignoring exception since merkle_root doesn't exist
	}
	j[index_str]["0"]["timestamp"] = this->timestamp;
	j[index_str]["0"]["send_addr"] = this->send_addr;
	j[index_str]["0"]["recieve_addr"] = this->recieve_addr.c_str();
	j[index_str]["0"]["previous_hash"] = this->previous_hash;
	j[index_str]["0"]["difficulty"] = this->difficulty;
	j[index_str]["0"]["amount"] = this->amount;
	if(this->merkle_root.empty()) {	
		j[index_str]["0"]["hash"] = this->hash; 
		j[index_str]["0"]["nounce"] = this->nounce; // adding nounce for other miners to check chain
		j["merkle_root"] = this->hash; // if its the genesis block, use hash of current block
		j[index_str]["success"] = true; // genesis block should only contain 1 transaction
	} else {
		/*
		this->merkle_root =  generate_hash(this->merkle_root + this->previous_hash);
		j["merkle_root"] = this->merkle_root;
		*/
		j[index_str]["success"] = false;
	}
	this->nounce = 0; // clearing nounce for next block
	j["blocks"] = this->index;
	return j;
}

nlohmann::json block::mine_transaction(int trans_num) {
	nlohmann::json jchain = blockchain::blockchain_json();
	std::string str_num = std::to_string(trans_num), index;
	std::pair<std::string, std::string> recieve_amount = {this->recieve_addr, std::to_string(this->amount)};
	std::ofstream ofschain;
	this->data = recieve_amount.first + "/" + recieve_amount.second + "/" + this->send_addr;
	std::cout << this->data << std::endl;
	if(this->previous_hash != "0" && !jchain["merkle_root"].empty()) { // if it's the genesis block, no need to save since it already does it
		index = std::to_string((int)jchain["blocks"]);
		this->merkle_root = jchain["merkle_root"];
		this->timestamp = jchain[index][str_num]["timestamp"];
		if(trans_num == 4 || trans_num == 3) {
			jchain[index]["success"] = true;
		}
		mine_block();
		jchain["merkle_root"] = generate_hash(this->merkle_root + this->hash);
		jchain[index][str_num]["hash"] = this->hash;
		jchain[index][str_num]["nounce"] = this->nounce;
		trans_num++;
		if(trans_num != 4) {
			jchain[index][std::to_string(trans_num)]["previous_hash"] = this->hash; // previous hash needs to be set
		}
		ofschain.open(blockchain::path);
		ofschain << std::setw(4) << jchain << std::endl;
		ofschain.close();
		return jchain;
	}
	mine_block();
	return jchain;
}

int block::add_block() {
	nlohmann::json j, j_new;
	if(this->previous_hash.empty()) {
		return 1; // block data is not initialized
	}
	if(blockchain::is_blockchain_empty()) {
		this->index = 0;
	} else {
		this->index = blockchain::block_number();
		j = blockchain::blockchain_json();
	}
	this->index++;
	std::string index_str = std::to_string(this->index);
	this->timestamp = get_timestamp();
	if(this->previous_hash == "0") { // mining block only if genesis block because block only contains 1 transaction
		mine_transaction(0);
	}
	j_new = set_finished_data(j, index_str);
 	std::ofstream ofChain(blockchain::path);
	ofChain << std::setw(4) << j_new << std::endl; // beautify JSON data before writing to file
	ofChain.close(); // might write a function for just opening/closing blockchain file
	create_block_file(j_new[index_str]);
	return 0;
}



