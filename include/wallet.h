#pragma once
#include <iostream>
#include <cryptopp/rsa.h>


namespace wallet {
	extern std::string path;
	int init_wallet(int argc, char** argv);
	bool is_empty();
	int show_help();
	int create_wallet(char **argv);
	int check_parameters(int argc, char **argv, std::string arg);
	int open(std::string filename);
	CryptoPP::RSA::PrivateKey open_wallet(std::string filename);
}

namespace rsa {
	std::string generate_private_key();
	
}
