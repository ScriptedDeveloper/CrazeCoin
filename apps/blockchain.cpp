#include "../include/blockchain.h"

int main(int argc, char **argv) {
	blockchain::check_files();
	blockchain::init_blockchain(argc, argv);	// Calling start of blockchain
	return 0;
}
