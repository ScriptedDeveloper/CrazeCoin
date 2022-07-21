#pragma once
#include <iostream>
#include <nlohmann/json.hpp>

class block {
	public:
		// Class variables
		std::string hash;
		std::string timestamp;
		std::string previous_hash;
		std::string merkle_root;  // hash of all blocks, see bitcoin docs
		std::string data;
		int nounce;
		int index;
		int difficulty; // adding an automated difficulty function later

		// Constructor

		block(std::string previous_hash, std::string data);
		
		//Class functions
		
		std::string generate_hash(std::string plain_text);	
		std::string mine_block();
	 	std::string get_timestamp();
		std::string verify_block();
		int test_block();
		int add_block();
		nlohmann::json set_data(nlohmann::json j, std::string json_str);
		int create_block_file(nlohmann::json j);
};

std::string generate_hash(std::string plain); // generates sha256 hash from a string
