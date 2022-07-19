#include <iostream>
#include <cstring>
#include <fstream>
#include <cryptopp/filters.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/config_int.h>
#include <cryptopp/files.h>
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

int wallet::create_wallet(char **argv) {
	CryptoPP::RSA::PrivateKey prikey = rsa_wrapper::generate_private_key();
	CryptoPP::RSA::PublicKey publkey(prikey);
	rsa_wrapper::save_public_key("address.bin", publkey);
	rsa_wrapper::save_private_key(argv[1], prikey);
	return 0;
}

CryptoPP::RSA::PrivateKey wallet::open_wallet(std::string filename) {
	std::ifstream ifs(filename);
	CryptoPP::RSA::PrivateKey privkey;
	path = filename; // changing filename local to namespace
	rsa_wrapper::load_private_key(path, privkey);
	return privkey;
}	

int wallet::check_parameters(int argc, char **argv, std::string arg) {
	for(int i = 1; i < argc; i++) {
		if(argv[i] == arg) {
			return 0; // arg has been found
		}
	}
	return 1; // arg has not been found
}

void wallet::print_addr(CryptoPP::RSA::PrivateKey privkey) { // prints wallet address
	CryptoPP::RSA::PublicKey publkey(privkey);
	std::cout << "Your wallet address is " << "test"<< std::endl; 
}

int wallet::show_help() {
	std::cout << "Usage: wallet <file> <argument> <additional parameters>\nArguments: send - sends coins to a wallet of choice\n generate - generates new wallet to use\nshow - shows wallet address" << std::endl;
	return 0;
}

int wallet::init_wallet(int argc, char **argv) {
	std::string cmd = std::string(argv[2]); // char pointer to operation
	if(argc == 1) {
		std::cout << "wallet : missing operand\nwallet <file> <argument>\nTry wallet help for more information.";
		return 1; // all params not satisfied
	} else if(argc == 2 || argc == 1) {
		if(!check_parameters(argc, argv, "help")) {
			show_help();
		}
	}
	path = argv[1]; // changing path to wallet path
	if(wallet::is_empty()) {
		wallet::create_wallet(argv);
	}
	//Construction area
	CryptoPP::RSA::PrivateKey privkey = open_wallet(argv[1]);
	rsa_wrapper::sign_data("test", privkey);
	if(cmd == "recieve") {
		print_addr(privkey);
	}
	//Construction area
	return 0;
}

