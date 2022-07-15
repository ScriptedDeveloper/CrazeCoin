#pragma once
#include <iostream>

namespace wallet {
	extern std::string path;
	int init_wallet(int argc, char** argv);
	bool is_empty();
	int show_help();
	int create_wallet();
	int check_parameters(int argc, char **argv, std::string arg);
}

namespace rsa {
	std::string generate_private_key();
	
}
