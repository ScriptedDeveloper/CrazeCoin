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
#include <cpr/api.h>
//#include <libtorrent/entry.hpp> // Network is basically a Torrent.
//#include <libtorrent/alert.hpp>
//#include <libtorrent/fwd.hpp>
//#include <libtorrent/bencode.hpp>
//#include <libtorrent/peer_info.hpp>
//#include <libtorrent/torrent_handle.hpp>
//#include <libtorrent/session.hpp>
//#include <libtorrent/torrent_info.hpp>
#include <nlohmann/json.hpp>
#include "../blockchain/blockchain.h"
#include "broadcast.h"


void error_handler(std::string message) {
	std::cout << "Failed at : " << message << std::endl;
	std::cout << strerror(errno) << std::endl;
	sleep(3);
}

std::string retrieve_peer(int n) {
	std::ifstream ifs(blockchain::peer_path);
	std::string peer;
	nlohmann::json j;
	try {
		j = j.parse(ifs);
		peer = j["peers"][n]; // returning n element at the moment
	} catch(...) { // in case peer.json has been manipulated or only pending peers
		create_json(blockchain::peer_path);
		blockchain::init_blockchain();	
	}
	return peer;
}

std::string retrieve_pending(int n) { // same as retrieve_peer with small changes
	std::ifstream ifs(blockchain::peer_path);
	nlohmann::json j;
	try {
		j = j.parse(ifs);
	} catch(nlohmann::detail::parse_error) {
		blockchain::init_blockchain();
	}
	return j["pending_peers"][n]; // instead of peers, returning pending_peers
}

void clear_peers() { 
	std::ofstream ofs(blockchain::peer_path);
	ofs << "{\"peers\" : [], \"pending_peers\" : []}";
	ofs.close();
}


/*
int check_node(std::string ip) { // optimal solution for now, will implement senders/recievers list later
	cpr::Response rip = cpr::Get(cpr::Url{"https://api.ipify.org/"});
	if(ip == rip.text) {
		return 1;
	}
	return 0;
}
*/

int check_block(nlohmann::json jblock) {
	std::string prev_hash = jblock["previous_hash"];
	block b(prev_hash, jblock["data"]);
	b.timestamp = jblock["timestamp"];
	b.nounce = jblock["nounce"];
	b.data = jblock["data"];
	if(b.verify_block() != jblock["hash"]) {
		return 1; // block is invalid and has been rejected
	}
	return 0; // block is valid, allowing
}

static int save_block(nlohmann::json jblock) {
	std::ofstream ofchain;
	std::ifstream ifschain(blockchain::path);
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


int recieve_chain() {
	struct sockaddr_in sockaddr;
	int isocket, client_fd;
 	isocket = socket(AF_INET, SOCK_STREAM, 0);
	int optional, inet;
	char buff[1024] = {0}; // recieving blockchain
	std::ofstream ofsBlock;
	std::ifstream ifsblock;
	if(isocket < 0) {
		error_handler("SOCKET CREATION");
		recieve_chain();
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
		recieve_chain();
	}
	client_fd = connect(isocket, (struct sockaddr*)&sockaddr, sizeof(sockaddr));
	if(client_fd < 0) {
		error_handler("CONNECT");
		recieve_chain();
	}
	std::cout << "Waiting for blockchain..." << std::endl;
	read(isocket, buff, 1024);
	buff[strlen(buff)] = '\0';
	ifsblock.open(blockchain::path);
	if(is_empty(ifsblock)) {
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


int send_chain(bool is_blockchain) {
	struct sockaddr_in sockaddr;
	int opt = 1, new_socket, server_fd = socket(AF_INET, SOCK_STREAM, 0);
	nlohmann::json j;
	std::ifstream ifs;
	if(is_blockchain) {
		ifs.open(blockchain::path);
	} else {
		ifs.open("block.json");
	}
	try {
		j = j.parse(ifs);
	} catch(...) {
		std::cout << ifs.rdbuf();
		exit(1); // parsing block(chain) failed
	}
	std::string json_str = j.dump();
	std::string next_peer = retrieve_pending(0);
	char json_char[1024];
	int i;
	for(i = 0; i < json_str.size(); i++) {
		json_char[i] = json_str[i];
	}
	json_char[i+1] = '\0';
	/*if(check_node(retrieve_peer(0)) == 1) { Thanks to ISP only 1 IP for all devices!
		next_peer = retrieve_peer(1);
	}
	*/
	if(server_fd == 0) {
		error_handler("SERVER_FD");
		send_chain(is_blockchain);
	}
	if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) != 0) {
		error_handler("SETSOCKOPT");
		send_chain(is_blockchain);
	}
	sockaddr.sin_port = htons(PORT);
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = INADDR_ANY;
	if(bind(server_fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
		error_handler("BIND");
		send_chain(is_blockchain);
	}
	if(listen(server_fd, 5) < 0) {
		error_handler("LISTEN");
		send_chain(is_blockchain);
	}
	std::cout << "\nConnecting to peer : " << next_peer << std::endl;
	socklen_t size = sizeof(sockaddr);
	if((new_socket = accept(server_fd, (struct sockaddr*)&sockaddr, &size)) < 0) {
		error_handler("ACCEPT");
	} 
	if (send(new_socket, json_char, strlen(json_char), 0) == -1) {
		error_handler("SEND");
	}
	close(new_socket);
	shutdown(server_fd, SHUT_RDWR);
	return 0;
}

int signup_peer() {
	cpr::Response rpeer_server;
	std::ifstream ifs(blockchain::path);
	if (is_empty(ifs)) {
		rpeer_server = cpr::Get(cpr::Url{blockchain::peer_tracker + "/add_pending_peers"});
	} else {
		rpeer_server = cpr::Get(cpr::Url{blockchain::peer_tracker + "/add_peer"});
	}
	ifs.close();
	return 0;
}

int get_peers() {
	std::ifstream ifsPeer(blockchain::peer_path);
	nlohmann::json jpeers, jpending, j = j.parse(ifsPeer);
	cpr::Response rpeer, rpending;
	int st_peer = rpeer.status_code, st_pend = rpending.status_code;
	std::string txt_peer = rpeer.text, txt_pending = rpending.text;
	sleep(2); // crashing purposes
	while(st_peer != 200 && st_pend != 200 && txt_peer.empty()) { // checking in case the list of peers/pending peers are empty
		rpeer = cpr::Get(cpr::Url{blockchain::peer_tracker + "/get_peers"});
		rpending = cpr::Get(cpr::Url{blockchain::peer_tracker + "/pending_peers"}); // retrieving pending peers
		txt_peer = rpeer.text;
		txt_pending = rpending.text;
		sleep(2); // avoiding too much traffic
	}
	jpeers = jpeers.parse(txt_peer);
	jpending = jpending.parse(txt_pending);
	j["peers"] = jpeers;
	j["pending_peers"] = jpending;
	std::ofstream ofsPeer(blockchain::peer_path);
	ofsPeer << j;
	ifsPeer.close();
	ofsPeer.close();
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

