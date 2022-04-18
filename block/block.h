#pragma once
#include <iostream>

class block {
	public:
		// Class variables
		std::string hash;
		std::string timestamp;
		std::string previous_hash;
		std::string data;
		int index;

		// Constructor

		block(std::string previous_hash, std::string data);
		
		//Class functions
		
		std::string generate_hash(std::string plain_text);	
		int add_block(block b);
};

std::string sha256_generate(std::string plain); // generates sha256 hash from a string
