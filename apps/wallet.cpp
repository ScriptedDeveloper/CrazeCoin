#include "../include/wallet.h"


int main(int argc, char** argv) {
	if(argc == 0) {
		std::cout << "No parameters specified." << std::endl;
		exit(0);
	}
	wallet::init_wallet(argc, argv); // see include/wallet.h for code.
	return 0;
}
