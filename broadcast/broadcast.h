#pragma once
#include <iostream>

std::string retrieve_peer();
int connect_network();
int get_peers();
int send_chain(bool is_blockchain);
int recieve_chain();
int signup_peer();
void clear_peers();
