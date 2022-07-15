#include "../include/blockchain.h"

int main() {
	blockchain::check_files();
	//blockchain::generate_genesis_block("ASD"); // blockchain won't start without genesis block
	blockchain::init_blockchain();	// Calling start of blockchain
	/*
	broadcast::send_chain(true);
	std::ifstream ifs(blockchain::path);
	nlohmann::json j_ = j_.parse(ifs);
	int blocks = j_["blocks"];
	std::string blocks_string = std::to_string(blocks);
	block b(j_[blocks_string]["hash"], "ASD2");
	b.add_block();
	broadcast::send_chain(false);
	*/
	return 0;
}
