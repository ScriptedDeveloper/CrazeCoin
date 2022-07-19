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
	void print_addr(CryptoPP::RSA::PrivateKey privkey); // prints wallet address
	CryptoPP::RSA::PrivateKey open_wallet(std::string filename);
}

