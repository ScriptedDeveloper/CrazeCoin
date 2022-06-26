#pragma once
#include <iostream>

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
		int test_block();
		int add_block();
};

std::string generate_hash(std::string plain); // generates sha256 hash from a string
