#pragma once
#include <iostream>
#include <nlohmann/json.hpp>

class block {
	public:
		// Class variables
		std::string merkle_root;  // hash of all blocks, see bitcoin docs
		std::string data;
		std::string timestamp; // in unix time
		std::string send_addr; // address of sender
		std::string recieve_addr; // address of reciever
		int nounce, index, amount;
		// Constructor
		block(std::string previous_hash, std::string recieve_addr, std::string send_addr, int amount);
		//Class functions
		std::string generate_hash(std::string plain_text);	
		std::string mine_block();
		std::string get_merkle_root();
	 	static std::string get_timestamp();
		std::string verify_block();
		int test_block();
		int add_block();
		bool genesis;
		
	private:
		std::string hash;
		std::string previous_hash;
		int difficulty; // adding an automated difficulty function later
		nlohmann::json set_data(nlohmann::json j, std::string json_str);
		int create_block_file(nlohmann::json j);

};

std::string generate_hash(std::string plain); // generates sha256 hash from a string
