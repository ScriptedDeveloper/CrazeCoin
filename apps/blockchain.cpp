#include "../include/blockchain.h"

int main() {
	blockchain::check_files();
	blockchain::init_blockchain();	// Calling start of blockchain
	return 0;
}
