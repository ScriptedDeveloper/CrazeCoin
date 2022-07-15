#include <iostream>
#include <fstream>
#include "../include/blockchain.h"
#include "../include/rsa.h"
#include "../include/wallet.h"

namespace wallet {
	std::string path;
}

bool wallet::is_empty() {
	std::ifstream ifswallet(wallet::path);
	if(!ifswallet.good()) {
		return true; // file doesn't exist
	}
	return blockchain::is_empty(ifswallet);
}

int wallet::create_wallet() {
	CryptoPP::RSA::PrivateKey prikey = rsa_wrapper::generate_private_key();
	rsa_wrapper::save_private_key("wallet", prikey);
	return 0;
}

int wallet::check_parameters(int argc, char **argv, std::string arg) {
	for(int i = 0; i < argc; i++) {
		if(argv[i] == arg) {
			return 0; // arg has been found
		}
	}
	return 1; // arg has not been found
}

int wallet::show_help() {
	std::cout << "Usage: wallet <file> <argument> <additional parameters>\nArguments: send - sends coins to a wallet of choice\n generate - generates new wallet to use\nshow - shows wallet address" << std::endl;
	return 0;
}

int wallet::init_wallet(int argc, char ** argv) {
	if(argc != 2 && argc != 1) {
		std::cout << "wallet : missing operand\nwallet <file> <argument>\nTry wallet help for more information.";
		return 1; // all params not satisfied
	} else if(argc == 1) {
		if(check_parameters(argc, argv, "help")) {
			show_help();
		}
	}
	if(wallet::is_empty()) {
		wallet::create_wallet();
	}
	return 0;
}

