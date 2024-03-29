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


#define PORT 8088
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <random>
#include <thread>
#include <future>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <errno.h>
#include <cpr/cpr.h>
#include <cpr/error.h>
#include <cpr/api.h>
#include <nlohmann/detail/exceptions.hpp>
#include "../include/blockchain.h"
#include "../include/broadcast.h"
#include "../include/blockchain.h"

std::string broadcast::error_handler(std::string message) {
	std::cout << "Failed at : " << message << std::endl;
	std::string error = strerror(errno);
	if(error == "Transport endpoint is already connected") {
		pthread_exit(NULL);
	}
	std::cout << strerror(errno) << std::endl;
	sleep(3);
	return error;
}

int broadcast::check_local_ip(std::string target_ip) { // checks if is own local ip
	std::vector<std::string> ips = get_local_ips();
	for(std::string ip : ips) {
		if(ip == target_ip) {
			return 0; // found!
		}
	}
	return 1;
}

std::vector<std::string> broadcast::get_local_ips() {
	struct ifaddrs *ifaddr;
	std::vector<std::string> ips;
	char ip[NI_MAXHOST];
	if(getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		return {}; // failed to retrieve ifaddr struct
	}
	for(struct ifaddrs *ifacurr = ifaddr; ifacurr != NULL; ifacurr = ifacurr->ifa_next) {
		if(ifacurr->ifa_addr == NULL) {
			continue;
		}
		if(ifacurr->ifa_addr->sa_family == AF_INET) { // checking whether ipv4 or ipv6
			 if(getnameinfo(ifacurr->ifa_addr, (ifacurr->ifa_addr->sa_family == AF_INET) ? sizeof(sockaddr_in) : sizeof(sockaddr_in6), ip, NI_MAXHOST, NULL, 0, NI_NUMERICHOST) != 0) {
				 std::cout << "Failed to retrieve local IP!" << std::endl;
			 }
			ips.push_back(ip);
		}
	}
	return ips;
}

void broadcast::recieve_chain_thread_handler() {
	while(true) {
		std::thread transaction_recieve(broadcast::recieve_chain, true); // changing for debugging
		transaction_recieve.join();
		sleep(20);
	}
}

void broadcast::send_chain_thread_handler() {
	while(true) {
		std::thread broadcaster(broadcast::send_chain, true, false, "", ""); // is original peer/miner, broadcasting blockchain			
		broadcaster.join();
		sleep(20);
	}
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
		error_handler("PEER FILE_EMPTY");
		exit(1);
	}
	return peer;
}

std::string broadcast::retrieve_pending(int n) { // same as retrieve_peer with small changes
	std::ifstream ifs(blockchain::peer_path);
	nlohmann::json j;
	try {
		j = j.parse(ifs);
	} catch(nlohmann::detail::parse_error) {
		blockchain::init_blockchain(0 , (char**)"");
	}
	if(!j["pending_peers"].empty()) {
		return j["pending_peers"][n]; // instead of peers, returning pending_peers
	}
	return "";
}

void broadcast::clear_peers() { 
	std::ofstream ofs(blockchain::peer_path);
	ofs << "{\n    \"peers\" : [],\n    \"pending_peers\" : []\n}" << std::endl;
	ofs.close();
}

int broadcast::fail_emergency_mode() {
	std::cout << "(EMERGENCY MODE) No (pending) peers found, quitting!";
	return 0; // emergency mode has failed, quitting.
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

int broadcast::unsign_pend_peer() { // unsigns pending peer and signs up as a miner
	cpr::Response r_unsign = cpr::Get(cpr::Url{blockchain::peer_tracker + "/unsign_pend_peer"});
	if(r_unsign.status_code == 200) {
		cpr::Response r_signup = cpr::Get(cpr::Url{blockchain::peer_tracker + "/add_peer"});
		if(r_unsign.status_code == 200) {
			return 0;
		}
	}
	return 1;
}

int broadcast::unsign_miner() { // unsigns miner and signs up as a pending peer
	cpr::Response r_unsign = cpr::Get(cpr::Url{blockchain::peer_tracker + "/unsign_pend_peer"});
	if(r_unsign.status_code == 200) {
		cpr::Response r_signup = cpr::Get(cpr::Url{blockchain::peer_tracker + "/add_peer"});
		if(r_unsign.status_code == 200) {
			return 0;
		}
	}
	return 1;
}

int broadcast::check_block(nlohmann::json jblock) {
	if(!jblock.contains("nounce")) {
		return 0; // block is still unmined
	}
	std::pair<std::string, std::string> recieve_amount = {jblock["recieve_addr"], std::to_string((int)jblock["amount"])};	
	block b(jblock["previous_hash"], jblock["recieve_addr"], jblock["send_addr"], jblock["amount"]);
	b.nounce = jblock["nounce"];
	b.timestamp = jblock["timestamp"];
 	b.data = recieve_amount.first + "/" + recieve_amount.second + "/" + (std::string)jblock["send_addr"];
	std::string hash_ = b.verify_block();
	std::cout << b.data << std::endl;
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
	i++;
	j[blocks_num][std::to_string(i)] = jtrans;
	ofchain = std::ofstream(blockchain::path);
	ofchain << std::setw(4) << j << std::endl;
	broadcast_transaction(jtrans); // sending transaction to other nodes so the race starts soon!!
	if(i == blockchain::max_transactions) {
		for(int i = 0; i <= 3; i++) {
			std::string s_i = std::to_string(i);
			block b_trans(j[blocks_num][s_i]["previous_hash"], j[blocks_num][s_i]["recieve_addr"], j[blocks_num][s_i]["send_addr"], j[blocks_num][s_i]["amount"]);
			j = b_trans.mine_transaction(i);
		}
		jtrans["previous_hash"] = blockchain::get_previous_hash(true);
		blocks_num = std::to_string(blockchain::block_number());
		i = 0;
	}
	return 0;
}

int broadcast::save_block(nlohmann::json jblock, bool is_transaction, bool is_recieved_block) {
	nlohmann::json jchain = blockchain::blockchain_json();
	int blocks_num = blockchain::block_number();
	if(!is_transaction && !is_recieved_block) {
		block b(blockchain::get_previous_hash(true), jblock["recieve_addr"], jblock["send_addr"], jblock["amount"]);
		try {
			if(check_block(jblock) != 0) {
				return 1; // block is invalid
			}	
		} catch(...) {
			// ignoring type error, creating a new block since last one is full
		} 
		try {
			blocks_num++;
			broadcast_transaction(jblock);
			b.add_block();
		} catch(nlohmann::json::parse_error) {
			return 1; // block is invalid json
		}
	} else if(is_recieved_block) {	
		blocks_num++;
		jchain["blocks"] = (int)jchain["blocks"] + 1;	
		std::ofstream ofschain(blockchain::path);
		jchain[std::to_string(blocks_num)] = jblock;
		ofschain << jchain.dump(4);
		for(int i = 0; i <= blockchain::max_transactions; i++) {
			std::string jblockstr = jblock.dump(), i_str = std::to_string(i), i_default = std::to_string(i + 1);
			if(std::stoi(i_default) <= 3) {	 // i_default can go up to 4, there are obviously no 4 transactions in a block
				if(!blockchain::verify_transaction(jblock[i_default]).first) { // not checking first transaction because it doesnt contain signature for some reason (fixing l8ter)
					return 1;
				}
			}
			block b(jblock[i_str]["previous_hash"], jblock[i_str]["recieve_addr"], jblock[i_str]["send_addr"], jblock[i_str]["amount"]);
			jchain = b.mine_transaction(i);
		 	jblock = jchain[std::to_string(blocks_num)];
		}
		return 0;
	} else {	
		try {	
			if(!jchain[std::to_string(blocks_num)].contains("success") || jchain[std::to_string(blocks_num)]["success"] == false) {
				if(blockchain::check_balances(jblock["send_addr"]) >= jblock["amount"]) { // checking whether wallet has enough coins
					add_transaction(jblock);
					bool contains = false;
					if(!jblock.contains("peer_amount")) {
						jblock["peer_amount"] = 0; // setting peer amount for list
						contains = true;
					}
				}
			} 
			else {
				//sleep(20); // 20 secs timeout for broadcasting current state of blockchain
				save_block(jblock, false, false); // current block is full, creating a new one
			}

		} catch(...) {
			throw;
			return 1; // something horribly went wrong..
		}
	}
	return 0;
}

int broadcast::pend_to_miner(std::string target_ip) { // puts pending peers to miner 
	std::ifstream ifspeer(blockchain::peer_path);
	std::ofstream ofspeer;
	int i = 0;
	nlohmann::json jpeer;
	std::stringstream ss;
	ss << ifspeer.rdbuf();
	jpeer = raw_to_json(ss.str());
	if(jpeer == 1) {
		std::cout << "[SYSTEM] Error! Peer file is corrupted!" << std::endl;
		return 1;
	}
	for(std::string ip : jpeer["pending_peers"]) {
		if(target_ip == ip) {
			jpeer["pending_peers"].erase(i); // once ip has been found, erase
		}
		i++;
	}
	try {
		jpeer["peer_amount"] = (int)jpeer["peer_amount"] + 1;
	} catch(...) {
		jpeer["peer_amount"] = 1;
	}
	jpeer["peers"].push_back(target_ip); // putting target_ip to peers
	ofspeer = std::ofstream(blockchain::peer_path);
	ofspeer << jpeer; // saving results to file
	return 0;
}


int broadcast::broadcast_transaction(nlohmann::json block) {
	std::ifstream ifspeer(blockchain::peer_path);
	nlohmann::json jpeer = jpeer.parse(ifspeer);
	int peer_index;
	(block.contains("index")) ? peer_index = block["index"] : peer_index = 0; // sets peer index to either json object to 0
	if(check_local_ip(jpeer["peers"][peer_index]) == 0) {
		peer_index++; // if is same ip of machine, skipping
	}
	std::ofstream ofsblock(blockchain::block_path);
	ofsblock << block;
	send_chain(false, false, jpeer["peers"][peer_index], block.dump());
	return 0;
}

int broadcast::recieve_chain(bool is_transaction) { // is_transaction variable for miners to verify transaction and append to blockchain
	bool failed = false;	
	struct sockaddr_in sockaddr;
	int server_fd, opt = 1, new_socket;
 	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	int optional;
	char local_ip[INET_ADDRSTRLEN];
	size_t buff_size;
	char buff[1024]; // recieving blockchain
	std::ofstream ofsBlock;
	std::ifstream ifsblock;
	do {
		if(broadcast::check_emergency_mode() == 1) {
			broadcast::fail_emergency_mode(); // no peers available, failed
		}
		if(server_fd == 0) {
			error_handler("SOCKET CREATION");
			continue;
		}	
		if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) != 0) {
			error_handler("SETSOCKOPT");
			continue;
		}
		sockaddr.sin_port = htons(PORT);
		sockaddr.sin_family = AF_INET;
		sockaddr.sin_addr.s_addr = INADDR_ANY;
		if(bind(server_fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
			error_handler("BIND");
			continue;
		}
		if(listen(server_fd, 5) < 0) {
			error_handler("LISTEN");
			continue;
		}
		socklen_t size = sizeof(sockaddr);
		if((new_socket = accept(server_fd, (struct sockaddr*)&sockaddr, &size)) < 0) {
			error_handler("ACCEPT");
			continue;
		} 	
		inet_ntop(AF_INET, &sockaddr.sin_addr, local_ip, INET_ADDRSTRLEN);
		if(check_local_ip(std::string(local_ip)) == 0 || std::string(local_ip) == "127.0.0.1") { // not accepting transactions from the same IP
			continue;
		}
		std::cout << "Waiting for blockchain..." << std::endl;
		read(new_socket, buff, 1024);
		buff_size = std::atol(buff); // getting buffer size for real blockchain
		char buffchain[buff_size];
		recv(new_socket, buffchain, buff_size, MSG_WAITALL);
		std::string str_buff(buffchain);
		nlohmann::json jblock = raw_to_json(str_buff);
		if(jblock.contains(("success")) && !blockchain::is_blockchain_empty()) {
			if(jblock["success"]) {
				if(save_block(jblock, false, true) != 0) {
					return 1;
				}
				return 0;
			}
		}
		if(is_transaction && !jblock.contains("success") && !jblock.contains("merkle_root")) {
			try {
				std::pair<bool, nlohmann::json> return_val = blockchain::verify_transaction(jblock); // attempting to add transaction
				if(!return_val.first) {
					continue;  // transaction is invalid.
				}
				jblock = return_val.second;
			} catch(...) {
				continue;
			}
		}
		ifsblock.open(blockchain::path);
		if(blockchain::is_blockchain_empty() && jblock.contains("merkle_root")) {
			std::ofstream ofschain(blockchain::path);
			ofschain << str_buff;
		} else if(jblock.contains("success")) {
			if(!blockchain::is_blockchain_empty()) {
				if(save_block(jblock, false, true) == 0) {
					std::cout << "[SYSTEM] Successfully added block!" << std::endl;
					return 0;
				}
				std::cout << "[SYSTEM] Failed to add block!" << std::endl;
			}
			std::cout << "[SYSTEM] Rejected transaction! Missing blockchain." << std::endl;
			return 1;
		} else if(jblock.contains("blocks")) {
			int block_num = blockchain::block_number();
			if(block_num < jblock["blocks"]) {
				for(int i = block_num; i <= jblock["blocks"]; i++) {
					save_block(jblock[std::to_string(i)], false, true); // syncing outdated blockchain
				}
				std::cout << "[SYSTEM] Successfully synced blockchain!" << std::endl;
				return 0;
			}
		} else {
			try {
				if(save_block(jblock, true, false) != 0) {
					return 1; // either invalid json or manipulated block
				}
			} catch(...) {
				continue; // for some reason miner is recieving blockchain when he's sending it, blocking
			}
		}
		unsign_pend_peer();
		close(server_fd);
		break;
	} while(true);
	return 0;
}

int broadcast::send_chain(bool is_blockchain, bool is_transaction, std::string ip, std::string data) {
	struct sockaddr_in sockaddr;
	int opt = 1, inet, client_fd, isocket = socket(AF_INET, SOCK_STREAM, 0);
	char reciever_ip[INET_ADDRSTRLEN];
	std::string reciever_ip_str, filename; // getting filename to reopen after JSON parse function closes file
	std::ifstream ifs;
	nlohmann::json j;
	if(broadcast::check_emergency_mode() == -1 && !is_transaction && ip.empty()) {
		broadcast::fail_emergency_mode(); // same as recieve_chain
	}
	if(is_blockchain) {
		filename = blockchain::path;
		ifs.open(filename);
	} else if(!is_blockchain && !is_transaction && data.empty()) {
		filename = "block.json";
		ifs.open(filename);
	} else if(!data.empty()) { // if broadcasting transaction over entire network
		j = data;
	} else { // is_transaction bool is true, so opening ifs with transaction JSON
		filename = "transaction.json";
		ifs.open(filename);
	}
	try {
		if(data.empty()) {
			j = j.parse(ifs); // checking if is valid json
		}
	} catch(...) {
		std::cout << ifs.rdbuf();
		pthread_exit(NULL); // parsing block(chain) failed
	}
	if(filename == blockchain::path) {
		if(!j[std::to_string((int)j["blocks"])]["success"]) {
			sleep(20); // retrying in 20 seconds since block hasnt been mined yet
			return 1;
		}	 
	}
	std::ifstream ifs_obj(filename);
	std::stringstream ss_chain;
	ss_chain << ifs_obj.rdbuf(); // putting transaction into raw stringstream format since nlohmann::json::dump is behaving incorrectly
	std::string json_str = (data.empty()) ? ss_chain.str() : data;
	if(isocket < 0) {
		error_handler("SOCKET CREATION");
		send_chain(is_blockchain, is_transaction, ip, data);
	}
	sockaddr.sin_port = htons(PORT);
	sockaddr.sin_family = AF_INET; 
	inet = inet_pton(AF_INET, (ip.empty()) ? retrieve_peer(0).c_str() : ip.c_str(), &sockaddr.sin_addr);
	if(check_local_ip(retrieve_peer(0)) == 0 && ip.empty()) {
		return 1; // debugging purposes
	}
	client_fd = connect(isocket, (struct sockaddr*)&sockaddr, sizeof(sockaddr));
	if(client_fd < 0) {
		error_handler("WAITING FOR CONNECTION");
		send_chain(is_blockchain, is_transaction, ip, data);
	}
	std::string length_str = std::to_string(json_str.length());
	if(send(isocket, length_str.c_str(), 1024, MSG_NOSIGNAL) == -1) { // sending size of buffer
		if(error_handler("SEND") != "Broken pipe") { // ignoring broken pipe
			send_chain(is_blockchain, is_transaction, ip, data);
		}
	}
	if(send(isocket, json_str.c_str(), json_str.length(), MSG_NOSIGNAL) == -1) { 	
		if(error_handler("SEND") != "Broken pipe") { // ignoring broken pipe
			send_chain(is_blockchain, is_transaction, ip, data);
		}
	}
	inet_ntop(AF_INET, &sockaddr.sin_addr, reciever_ip, INET_ADDRSTRLEN);
	reciever_ip_str = std::string(reciever_ip);
	close(isocket);
	shutdown(client_fd, SHUT_RDWR);
	if(check_local_ip(reciever_ip_str) == 0) {
		return 1; // something went wrong with sending the data!
	}
	pend_to_miner(reciever_ip);
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

std::pair<int, nlohmann::json> broadcast::get_peers() {
	std::ifstream ifsPeer(blockchain::peer_path);
	nlohmann::json jpeers, jpending, j;	
	cpr::Response rpeer, rpending, ramount;
	std::ifstream ifspeer(blockchain::peer_path);
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
		if(blockchain::is_empty(ifspeer)) {
			std::cout << "[SYSTEM] Connecting to peer server failed! Opting to emergency mode..." << std::endl;
			return {1, 0};
		}
		std::cout << "[SYSTEM] Connecting to peer server failed! Opting to offline backup..." << std::endl;
		return {0, nlohmann::json::parse(ifspeer)};
	}
	jpeers = jpeers.parse(text_vec[0]);
	jpending = jpending.parse(text_vec[1]);
	j["peers"] = jpeers;
	j["pending_peers"] = jpending;
	j["peer_amount"] = text_vec[2];
	std::ofstream ofsPeer(blockchain::peer_path);
	ofsPeer << j.dump() << std::endl;
	ifsPeer.close();
	ofsPeer.close();
	return {0, j};
}

int broadcast::send_transaction() {
	int peer_amount, rand_number;
	std::random_device rd;
	std::uniform_int_distribution<int> rand;
	std::mt19937 mt(rd());	
	std::pair<int, nlohmann::json> peers = get_peers(); // getting peers to connect with
	try {
		peer_amount = nlohmann::json::parse(std::ifstream(blockchain::peer_path))["peer_amount"];
	} catch(...) {
		std::ifstream ifspeer(blockchain::peer_path);
		if(!blockchain::is_empty(ifspeer)) {
			nlohmann::json jpeer = jpeer.parse(ifspeer);
			for(std::string ip : jpeer["peers"]) {
				peer_amount++;
			}
			jpeer["peer_amount"] = peer_amount;
			std::ofstream ofspeer(blockchain::peer_path);
			ofspeer << jpeer; // saving peer amount so it doesnt have to count ips every time
		} else {
			std::cout << "[SYSTEM] Error! No peer file found!" << std::endl;
			return 1;
		}
	}
	if(peer_amount > 0) {
	rand = std::uniform_int_distribution<int>(0, peer_amount--);
	rand_number = rand(rd);
	} else {
		rand_number = 0; // peer_amount is already 0, no need to bother
	}
	if(peers.first == 0) {	
		if(rand_number > peer_amount) {
			send_transaction(); // trying again, preventing segfault
		}
		std::vector<std::string> peer_vec = peers.second["peers"];
		if(send_chain(false, true, peer_vec[rand_number]) == 1){ // broadcasting transaction to network
			return 1; // broadcast failed, aborting
		} 
		return 0;
	}
	
	return 1;
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

