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
#include "block.h"

std::string generate_hash(std::string plain_text) {
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

block::block(std::string previous_hash, std::string data){
	this->previous_hash = previous_hash;
	this->data = data;
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

int block::add_block(block b){
	std::cout << blockchain::path;
	if(this->data.empty() || this->previous_hash.empty()) {
		return 1; // block data not initialized
	}

	auto time = std::chrono::system_clock::now();
	std::string index_str = std::to_string(index);
	nlohmann::json j;
	this->index++;
	this->timestamp = std::to_string(std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch()).count());
	this->hash = generate_hash(index_str + timestamp + previous_hash + data);
	j[index_str]["timestamp"] = this->timestamp;
	j[index_str]["hash"] = this->hash;
	j[index_str]["data"] = this->data;
	j[index_str]["previous_hash"] = this->previous_hash;
	j["blocks"] = this->index;
 	std::ofstream ofChain(blockchain::path, std::ios_base::app);
	ofChain << j;
	ofChain.close(); // might write a function for just opening/closing blockchain file
	return 0;
}


