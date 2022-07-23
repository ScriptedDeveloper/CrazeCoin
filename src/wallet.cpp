#include <sstream>
#include <iostream>
#include <cstring>
#include <fstream>
#include <nlohmann/json.hpp>
#include <cryptopp/filters.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/config_int.h>
#include <cryptopp/files.h>
#include "../include/broadcast.h"
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
	std::ifstream ifs("config.json");
	if(blockchain::is_empty(ifs)) {
		std::ofstream ofs("config.json");
		nlohmann::json j;
		std::string addr;
		std::cout << "Please specify the Server URL provided :" << std::endl;
		std::cin >> addr;
		j["url"] = addr;
		ofs << j;
	}
	CryptoPP::RSA::PrivateKey prikey = rsa_wrapper::generate_private_key();
	CryptoPP::RSA::PublicKey publkey(prikey);
	rsa_wrapper::save_public_key("address.bin", publkey);
	rsa_wrapper::save_private_key(argv[1], prikey);
	return 0;
}

CryptoPP::RSA::PrivateKey wallet::open_wallet(std::string filename) {
	std::ifstream ifs(filename);
	if(blockchain::is_empty(ifs)) {
		std::cout << "No wallet found. Please generate one.\nExample: ./wallet FILENAME generate" << std::endl;
		exit(0); // exiting since no wallet has been found.
	}
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

std::string wallet::print_addr() { // prints wallet address
	std::ifstream ifs("address.bin");
	std::stringstream ss_addr;
	ss_addr << ifs.rdbuf();
	return ss_addr.str();
}

int wallet::create_transaction_json(nlohmann::json j) { // creates temporary transaction JSON data
	std::string filename = "transaction.json";
	std::ofstream ofs(filename);
	std::ifstream ifs;
	ofs << j;
	ofs.close();
	return 0;
}

int wallet::send(char **argv) {
	nlohmann::json jtransaction;
	block b("", "");
	std::string timestamp = b.get_timestamp();
	std::string sign_data = std::string(argv[3], argv[4]) + timestamp;
	jtransaction["recieve_addr"] = argv[3];
	jtransaction["amount"] = argv[4];
	jtransaction["send_addr"] = print_addr();
	jtransaction["timestamp"] = timestamp;
	create_transaction_json(jtransaction);
	// having to sign data, and send
	broadcast::send_transaction();
	// having to sign, and broadcast transaction to network
	return 0;
}

int wallet::show_help() {
	std::cout << "Usage: wallet <file> <argument> <additional parameters>\nArguments: send - sends coins to a wallet of choice\ngenerate - generates new wallet to use\nshow - shows wallet address" << std::endl;
	return 0;
}

int wallet::init_wallet(int argc, char **argv) {
	std::string cmd;
	try {
	cmd = std::string(argv[2]); // char pointer to operation
	} catch(...){
		cmd.clear();

	}
	if(argc == 1) {
		std::cout << "wallet : missing operand\nwallet <file> <argument>\nTry wallet help for more information.";
		return 1; // all params not satisfied
	} else if(argc == 2 || argc == 1) {
		if(!check_parameters(argc, argv, "help")) {
			show_help();
			exit(0); // exitting since nothing to do
		}
	}
	path = argv[1]; // changing path to wallet path
	//Construction area
	if(cmd == "show") {
		std::cout << "Your address is : \n" << print_addr() << std::endl;
	} else if(cmd == "generate" && is_empty()) {
		wallet::create_wallet(argv);
	} else {
		show_help();
		exit(0);
	}	
	CryptoPP::RSA::PrivateKey privkey = open_wallet(argv[1]);
	rsa_wrapper::sign_data("test", privkey);
	//Construction area
	return 0;
}

