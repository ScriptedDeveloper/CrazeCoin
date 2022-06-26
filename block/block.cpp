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

#include <fstream>
#include <iostream>
#include <cstdint>
#include <sstream>
#include <string>
#include <iomanip>
#include <chrono>
#include <nlohmann/json.hpp>
#include <openssl/sha.h>
#include "../blockchain/blockchain.h"
#include "../broadcast/broadcast.h"
#include "block.h"

block::block(std::string previous_hash, std::string data){
	this->previous_hash = previous_hash;
	this->data = data;
	this->nounce = 0;
	this->difficulty = 4; // for now
}

std::string block::generate_hash(std::string plain_text) {
	uint8_t hash[SHA224_DIGEST_LENGTH];
	SHA256_CTX sha;
	SHA256_Init(&sha);
	SHA256_Update(&sha, plain_text.c_str(), plain_text.size());
	SHA256_Final(hash, &sha);
	std::stringstream stream;
	for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
		stream << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
	}
	return stream.str();
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
	return this->hash;
}


std::string block::get_timestamp() {
	auto time = std::chrono::system_clock::now();
	return std::to_string(std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch()).count());

}

int block::add_block(){
	if(this->data.empty() || this->previous_hash.empty()) {
		return 1; // block data is not initialized
	}

	std::string index_str = std::to_string(index);
	nlohmann::json j;
	this->index++;
	this->timestamp = get_timestamp();
	mine_block();
	j[index_str]["timestamp"] = this->timestamp;
	j[index_str]["hash"] = this->hash;
	j[index_str]["data"] = this->data;
	j[index_str]["previous_hash"] = this->previous_hash;
	j[index_str]["difficulty"] = this->difficulty;
	j[index_str]["nounce"] = this->nounce; // adding nounce for other miners to check chain
	this->nounce = 0; // clearing nounce for next block
	if(this->merkle_root.empty()) {
		j["merkle_root"] = this->hash; // if its the genesis block, use hash of current block
	
	} else {
		j["merkle_root"] = generate_hash(this->merkle_root + this->previous_hash);
	}
	j["blocks"] = this->index;
 	std::ofstream ofChain(blockchain::path, std::ios_base::app);
	ofChain << j;
	ofChain.close(); // might write a function for just opening/closing blockchain file
	//std::cout << send_chain();
	return 0;
}

int block::test_block() {
	this->data = "test";
	add_block();
	return 0;
}


