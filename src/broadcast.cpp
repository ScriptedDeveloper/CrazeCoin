/*
CrazeCoin, a semi-decentralised work-in-progress CryptoCurrency
Copyright (C) 2022  ScriptedDev
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#define PORT 9000
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <thread>
#include <future>
#include <arpa/inet.h>
#include <errno.h>
#include <cpr/cpr.h>
#include <cpr/error.h>
#include <cpr/api.h>
#include <nlohmann/detail/exceptions.hpp>
#include "../include/blockchain.h"
#include "../include/broadcast.h"
#include "../include/blockchain.h"

void broadcast::error_handler(std::string message) {
	std::cout << "Failed at : " << message << std::endl;
	std::cout << strerror(errno) << std::endl;
	sleep(3);
}

std::string broadcast::retrieve_peer(int n) {
	std::ifstream ifs(blockchain::peer_path);
	std::string peer;
	nlohmann::json j;
	try {
		j = j.parse(ifs);
		peer = j["peers"][n]; // returning n element at the moment (array)
	} catch(...) { // in case peer.json has been manipulated or only pending peers
		blockchain::create_json(blockchain::peer_path);
		blockchain::init_blockchain();	
	}
	return peer;
}

std::string broadcast::retrieve_pending(int n) { // same as retrieve_peer with small changes
	std::ifstream ifs(blockchain::peer_path);
	nlohmann::json j;
	try {
		j = j.parse(ifs);
	} catch(nlohmann::detail::parse_error) {
		blockchain::init_blockchain();
	}
	return j["pending_peers"][n]; // instead of peers, returning pending_peers
}

void broadcast::clear_peers() { 
	std::ofstream ofs(blockchain::peer_path);
	ofs << "{\n    \"peers\" : [],\n    \"pending_peers\" : []\n}" << std::endl;
	ofs.close();
}

void broadcast::fail_emergency_mode() {
	std::cout << "(EMERGENCY MODE) No (pending) peers found, quitting!";
	exit(1); // emergency mode has failed, quitting.
}

int broadcast::check_emergency_mode() {
	std::ifstream ifs(blockchain::peer_path);
	nlohmann::json jpeers;
	try {
		jpeers = jpeers.parse(ifs);
	} catch (...) {
		return 2; // invalid json
	}
	if(jpeers["peers"].empty()){ // peers JSON file is empty
		return 1; // peers empty
	} else if(jpeers["pending_peers"].empty()) {
		return -1; // pending_peers empty
	} else if(jpeers["pending_peers"].empty() && jpeers["peers"].empty()) {
		broadcast::fail_emergency_mode(); // everything empty, failed mode
	}
	return 0;
}

int broadcast::check_block(nlohmann::json jblock) {
	std::string hash = jblock["hash"];
	std::pair<std::string, std::string> recieve_amount = {jblock["recieve_addr"], std::to_string((int)jblock["amount"])};
	block b(jblock["previous_hash"], jblock["recieve_addr"], jblock["send_addr"], jblock["amount"]);
	b.timestamp = jblock["timestamp"];
	b.nounce = jblock["nounce"];
	b.data = recieve_amount.first + "/" + recieve_amount.second + "/" + (std::string)jblock["send_addr"];
	std::string hash_ = b.verify_block();
	std::cout << (std::string)jblock["previous_hash"] + b.data + std::to_string(b.nounce) + b.timestamp << std::endl;
	if(hash_ != jblock["hash"]) {
		return 1; // block is invalid and has been rejected
	}
	return 0; // block is valid, allowing
}

int broadcast::add_transaction(nlohmann::json jtrans) { // adds transaction to block
	nlohmann::json j = blockchain::blockchain_json();
	std::string blocks_num = std::to_string(blockchain::block_number());
	std::ofstream ofchain;
	int i = 0; // checks amount of transactions
	bool first_time = false, exists = j[blocks_num].contains("success"); // first_time checks if block doesnt have proper array structure yet
	if(exists && j[blocks_num]["success"]) {
		return 1; // block is full
	} else if(!exists) {
		j[blocks_num]["success"] = false;
		first_time = true;
	}
	if(!first_time) {
		i = blockchain::get_transaction_num(blocks_num);
	}
	if(i == blockchain::max_transactions) {
		//j[blocks_num]["success"] = true;		
		for(int i = 0; i <= 3; i++) {
			std::string s_i = std::to_string(i);
			block b_trans(j[blocks_num][s_i]["previous_hash"], j[blocks_num][s_i]["recieve_addr"], j[blocks_num][s_i]["send_addr"], j[blocks_num][s_i]["amount"]);
			j = b_trans.mine_transaction(i);
		}
		return 0;
	}
	i++;
	j[blocks_num][std::to_string(i)] = jtrans;
	ofchain = std::ofstream(blockchain::path);
	ofchain << std::setw(4) << j << std::endl;
	return 0;
}

int broadcast::save_block(nlohmann::json jblock, bool is_transaction) {
	std::ofstream ofchain;	
	nlohmann::json jchain = blockchain::blockchain_json();
	int blocks_num = blockchain::block_number();
	std::string addr = jblock["recieve_addr"];
	if(!is_transaction) {
		block b(blockchain::get_previous_hash(true), jblock["recieve_addr"], jblock["send_addr"], jblock["amount"]);
		try {
			if(check_block(jblock) != 0) {
				return 1; // block is invalid
			}	
		} catch(nlohmann::json_abi_v3_11_2::detail::type_error) {
			// ignoring type error, creating a new block since last one is full
		} 
		try {
			blocks_num++;
			b.add_block();
		} catch(nlohmann::json::parse_error) {
			return 1; // block is invalid json
		}
	}
	else {	
		//try {	
			if(!jchain[std::to_string(blocks_num)].contains("success") || jchain[std::to_string(blocks_num)]["success"] == false) {
				if(blockchain::check_balances(jblock["send_addr"]) >= jblock["amount"]) { // checking whether wallet has enough coins
					jblock.erase("signature");
					jblock.erase("signature_len"); // unneeded since sender/reciever addr is added into hash and signature has already been verifed
					add_transaction(jblock);
				}
			} 
			else {
				save_block(jblock, false); // current block is full, creating a new one
			}

		//} catch(...) {
		//return 1; // something horribly went wrong..
		}
	
	return 0;
}

nlohmann::json broadcast::raw_to_json(std::string raw){
	nlohmann::json j_data;
	int retries = 0;
	while(true) {
		try {	
			j_data = j_data.parse(raw);
			break;
		} catch(...) {
			raw.pop_back(); // trying to remove garbage character and try again
			if(retries == 400) {
				return 1; // parsing failed, char array is not valid JSON
			}
		}
	}
	return j_data;
}

int broadcast::recieve_chain(bool is_transaction) { // is_transaction variable for miners to verify transaction and append to blockchain
	bool failed = false;	
	struct sockaddr_in sockaddr;
	int isocket, client_fd;
 	isocket = socket(AF_INET, SOCK_STREAM, 0);
	int optional, inet;
	size_t buff_size;
	char buff[1024]; // recieving blockchain
	std::ofstream ofsBlock;
	std::ifstream ifsblock;
	do {
		if(broadcast::check_emergency_mode() == 1) {
			broadcast::fail_emergency_mode(); // no peers available, failed
		}
		if(isocket < 0) {
			error_handler("SOCKET CREATION");
			continue;
		}
		sockaddr.sin_port = htons(PORT);
		sockaddr.sin_family = AF_INET;
		sockaddr.sin_addr.s_addr = INADDR_ANY;
		inet = inet_pton(AF_INET, retrieve_peer(0).c_str(), &sockaddr.sin_addr);
		if(inet <= 0) {
			error_handler("INET_PTON : return code " + std::to_string(inet));
			continue;
		}
		client_fd = connect(isocket, (struct sockaddr*)&sockaddr, sizeof(sockaddr));
		if(client_fd < 0) {
			error_handler("WAITING FOR CONNECTION");
			continue;
		}
		std::cout << "Waiting for blockchain..." << std::endl;
		read(isocket, buff, 1024);
		buff_size = std::atol(buff); // getting buffer size for real blockchain
		char buffchain[buff_size];
		read(isocket, buffchain, buff_size);
		std::string str_buff(buffchain);
		nlohmann::json jblock;
		if(is_transaction) {
			try {
				std::pair<bool, nlohmann::json> return_val = blockchain::verify_transaction(raw_to_json(str_buff)); // attempting to add transaction
				if(!return_val.first) {
					continue;  // transaction is invalid.
				}
				jblock = return_val.second;
			} catch(...) {
				continue;
			}
		}
		ifsblock.open(blockchain::path);
		if(blockchain::is_empty(ifsblock)) {
			ifsblock.close();
			ofsBlock.open(blockchain::path);
			continue;
		} else {
			if(save_block(jblock, true) != 0) {
				return 1; // either invalid json or manipulated block
			}
		}
		break;
	} while(true);
	return 0;
}


int broadcast::send_chain(bool is_blockchain, bool is_transaction) {
	struct sockaddr_in sockaddr;
	int opt = 1, new_socket, server_fd = socket(AF_INET, SOCK_STREAM, 0);
	std::string filename; // getting filename to reopen after JSON parse function closes file
	std::ifstream ifs;
	if(broadcast::check_emergency_mode() == -1 && !is_transaction) {
		broadcast::fail_emergency_mode(); // same as recieve_chain
	}
	if(is_blockchain) {
		filename = blockchain::path;
		ifs.open(filename);
	} else if(!is_blockchain && !is_transaction) {
		filename = "block.json";
		ifs.open(filename);
	} else { // is_transaction bool is true, so opening ifs with transaction JSON
		filename = "transaction.json";
		ifs.open(filename);
	}
	try {
		nlohmann::json j;
		j = j.parse(ifs); // checking if is valid json
	} catch(...) {
		std::cout << ifs.rdbuf();
		exit(1); // parsing block(chain) failed
	}
	std::ifstream ifs_obj(filename);
	std::stringstream ss_chain;
	ss_chain << ifs_obj.rdbuf(); // putting transaction into raw stringstream format since nlohmann::json::dump is behaving incorrectly
	std::string json_str = ss_chain.str();
	std::string next_peer;
	if(!is_transaction && is_blockchain){
		next_peer = retrieve_pending(0); // declaring var only if its a miner broadcasting blockchain
	} else {
		next_peer = retrieve_peer(0);
	}
	/*if(check_node(retrieve_peer(0)) == 1) { Thanks to ISP only 1 IP for all devices!
		next_peer = retrieve_peer(1);
	}
	*/
	if(server_fd == 0) {
		error_handler("SERVER_FD");
		send_chain(is_blockchain, is_transaction);
	}
	if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) != 0) {
		error_handler("SETSOCKOPT");
		send_chain(is_blockchain, is_transaction);
	}
	sockaddr.sin_port = htons(PORT);
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = INADDR_ANY;
	if(bind(server_fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
		error_handler("BIND");
		send_chain(is_blockchain, is_transaction);
	}
	if(listen(server_fd, 5) < 0) {
		error_handler("LISTEN");
		send_chain(is_blockchain, is_transaction);
	}
	std::cout << "\nConnecting to peer : " << next_peer << std::endl;
	socklen_t size = sizeof(sockaddr);
	if((new_socket = accept(server_fd, (struct sockaddr*)&sockaddr, &size)) < 0) {
		error_handler("ACCEPT");
	} 
	std::string length_str = std::to_string(json_str.length());
	if(send(new_socket, length_str.c_str(), 1024, 0) == -1) { // sending size of buffer
		error_handler("SEND");
	}
	if (send(new_socket, json_str.c_str(), json_str.length(), 0) == -1) { 
		error_handler("SEND");
	}
	close(new_socket);
	shutdown(server_fd, SHUT_RDWR);
	return 0;
}

int broadcast::signup_peer() {
	cpr::Response rpeer_server;
	std::ifstream ifs(blockchain::path);
	if (blockchain::is_empty(ifs)) {
		rpeer_server = cpr::Get(cpr::Url{blockchain::peer_tracker + "/add_pending_peers"}, cpr::Timeout{20000});
	} else {
		rpeer_server = cpr::Get(cpr::Url{blockchain::peer_tracker + "/add_peer"}, cpr::Timeout{20000});
	}
	if(rpeer_server.status_code != 200 && rpeer_server.status_code != 203) {
		return 1; // signing up failed, emergency mode engaged soon!
	}
	ifs.close();
	return 0;
}

int broadcast::get_peers() {
	std::ifstream ifsPeer(blockchain::peer_path);
	nlohmann::json jpeers, jpending, j;	
	cpr::Response rpeer, rpending, ramount;
	//std::string txt_peer, txt_pending;
	std::vector<std::string> text_vec(3); // 0 = peers ips, 1 = pending ips, 2 = amount of peers
	std::vector<cpr::Response> response_vec(3); // same as above
	std::cout << "[SYSTEM] Connecting to peer server.." << std::endl;
	int retrieves = 0; // after 3 retries opting to emergency mode
	do {
		if(retrieves == 3) {
			break; // max retries already
		} else if(retrieves >= 1) {
			sleep(2); // wait a little time before retrying..
		}
		retrieves++;
		response_vec[0] = cpr::Get(cpr::Url{blockchain::peer_tracker + "/get_peers"}); // issue somewhere in here at wallet
		response_vec[1] = cpr::Get(cpr::Url{blockchain::peer_tracker + "/pending_peers"}); // retrieving pending peers
		response_vec[2] = cpr::Get(cpr::Url{blockchain::peer_tracker + "/peers"});
		for(int i = 0; i < 2; i++) {
			text_vec[i] = response_vec[i].text; // assigning all response texts to other vector
		}
		/// add here a thread to recieve_chain to accept transactions
	} while(response_vec[0].status_code != 200 || response_vec[2].status_code != 200 || text_vec[0] == "{}" || text_vec[2] == "1"); // checking in case the list of peers/pending peers are empty
	if(retrieves >= 3) {
		std::cout << "[SYSTEM] Connecting to peer server failed! Opting to emergency mode..." << std::endl;
		return 1;
	}
	jpeers = jpeers.parse(text_vec[0]);
	jpending = jpending.parse(text_vec[1]);
	j["peers"] = jpeers;
	j["pending_peers"] = jpending;
	std::ofstream ofsPeer(blockchain::peer_path);
	ofsPeer << j.dump() << std::endl;
	ifsPeer.close();
	ofsPeer.close();
	return 0;
}

int broadcast::send_transaction() {
	get_peers(); // getting peers to connect with
	if(send_chain(false, true) == 1){ // broadcasting transaction to network
		return 1; // broadcast failed, aborting
	} 	
	return 0;
}
/*
int get_peers() {
	lt::session s;
	lt::add_torrent_params p;
	lt::torrent_handle t;
	std::string path_temp = blockchain::path;
	path_temp.erase(path_temp.size() - 15, path_temp.size());
	std::ofstream o(blockchain::peer_path);
	lt::error_code ec;
	nlohmann::json j;
	std::vector<libtorrent::peer_info> vpeer_info;
	std::vector<std::string> peer_ips;
	std::vector<lt::alert*> alerts;
	p.save_path = path_temp;
	p.ti = std::make_shared<lt::torrent_info>(blockchain::torrent_file);
	t = s.add_torrent(p, ec);
	int i = 0;
	while(i != 3){ 
		t.get_peer_info(vpeer_info);
		for(libtorrent::peer_info s : vpeer_info) {
			if(i == 3) {
				goto done;
			}
			j[i] = s.ip.address().to_string(); 
			if(!j.empty()) {
				i++;
			}
		}
		if(i != 3 && !j.empty()) {
			break;
		}
	}
	done:		
		o << j;
		o.close();

	return 0;
}


*/

