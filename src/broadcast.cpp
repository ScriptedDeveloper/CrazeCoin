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
		peer = j["peers"][n]; // returning n element at the moment
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
	ofs << "{\"peers\" : [], \"pending_peers\" : []}";
	ofs.close();
}

void broadcast::fail_emergency_mode() {
	std::cout << "(EMERGENCY MODE) No (pending) peers found, quitting!";
	exit(1); // Emergency mode has failed, quitting.
}


int broadcast::check_emergency_mode() {
	std::ifstream ifs(blockchain::peer_path);
	nlohmann::json jpeers = jpeers.parse(ifs);
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
	block b(jblock["previous_hash"], jblock["recieve_addr"], jblock["send_addr"], jblock["amount"], false);
	b.timestamp = jblock["timestamp"];
	b.nounce = jblock["nounce"];
	if(b.verify_block() != jblock["hash"]) {
		return 1; // block is invalid and has been rejected
	}
	return 0; // block is valid, allowing
}

int broadcast::save_block(nlohmann::json jblock) {
	std::ofstream ofchain;
	std::ifstream ifschain(blockchain::path);
	block b(jblock["previous_hash"], jblock["recieve_addr"], jblock["send_addr"], jblock["amount"], false);
	if(check_block(jblock) != 0) {
		return 1; // block is invalid
	}
	try {
		nlohmann::json jchain = jchain.parse(ifschain);
		ifschain.close();
		int blocks_num = jchain["blocks"];
		blocks_num++;
		jchain["blocks"] = blocks_num;
		jchain[std::to_string(blocks_num)] = jblock;
		ofchain.open(blockchain::path);
		ofchain << jchain; // saving edits to blockchain

	} catch(nlohmann::json::parse_error) {
		return 1; // blockchain is invalid json
	}
	return 0;
}

nlohmann::json broadcast::raw_to_json(char raw[]){
	nlohmann::json j_data;
	try {	
		std::ofstream ofs("test");
		ofs << raw;
		ofs.close();
		j_data = j_data.parse(raw);
	
	} catch(...) {
		return 1; // parsing failed, char array is not valid JSON
	}
	return j_data;
}


int broadcast::recieve_chain(bool is_transaction) { // is_transaction variable for miners to verify transaction and append to blockchain
	struct sockaddr_in sockaddr;
	int isocket, client_fd;
 	isocket = socket(AF_INET, SOCK_STREAM, 0);
	int optional, inet;
	char buff[1024] = {0}; // recieving blockchain
	std::ofstream ofsBlock;
	std::ifstream ifsblock;
	if(broadcast::check_emergency_mode() == 1) {
		broadcast::fail_emergency_mode(); // no peers available, failed
	}
	if(isocket < 0) {
		error_handler("SOCKET CREATION");
		recieve_chain(is_transaction);
	}
	/*if(check_node(retrieve_peer(0)) == 1) { Thanks to ISP only 1 IP for all devices!
		next_peer = retrieve_peer(1);
	}*/
	sockaddr.sin_port = htons(PORT);
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = INADDR_ANY;
	inet = inet_pton(AF_INET, retrieve_peer(0).c_str(), &sockaddr.sin_addr);
	if(inet <= 0) {
		error_handler("INET_PTON : return code " + std::to_string(inet));
		recieve_chain(is_transaction);
	}
	client_fd = connect(isocket, (struct sockaddr*)&sockaddr, sizeof(sockaddr));
	if(client_fd < 0) {
		error_handler("CONNECT");
		recieve_chain(is_transaction);
	}
	std::cout << "Waiting for blockchain..." << std::endl;
	read(isocket, buff, 1024);
	buff[strlen(buff)] = '\0';
	if(is_transaction) {
		blockchain::add_transaction(raw_to_json(buff)); // attempting to add transaction
	}
	ifsblock.open(blockchain::path);
	if(blockchain::is_empty(ifsblock)) {
		ifsblock.close();
		ofsBlock.open(blockchain::path);
		std::cout << buff << std::endl;
		ofsBlock << buff;
	} else {
		nlohmann::json jblock;
		try {
			jblock = jblock.parse(buff);

		} catch(nlohmann::json::parse_error) {
			return 1; // block is invalid json, failing
		}
		if(save_block(jblock) != 0) {
			return 1; // either invalid json or manipulated block
		}
	}
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
	const char *json_char = json_str.c_str();
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
	if (send(new_socket, json_char, strnlen(json_char, json_str.size()), 0) == -1) { // * strnlen function causes issues since not sending correct data * 
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
	ofsPeer << j;
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

