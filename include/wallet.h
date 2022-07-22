#pragma once
#include <iostream>
#include <nlohmann/json.hpp>
#include <cryptopp/rsa.h>


namespace wallet {
	extern std::string path;
	int init_wallet(int argc, char** argv);
	bool is_empty();
	int send(char **argv);
	int show_help();
	int create_wallet(char **argv);
	int check_parameters(int argc, char **argv, std::string arg);
	int open(std::string filename);
	std::string print_addr(); // prints wallet address
	CryptoPP::RSA::PrivateKey open_wallet(std::string filename);
	int create_transaction_json(nlohmann::json j);
}

