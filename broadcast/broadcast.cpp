#define PORT 50000
#include <ios>
#include <cstring>
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
#include <libtorrent/entry.hpp>
#include <libtorrent/alert.hpp>
#include <libtorrent/fwd.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/peer_info.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/session.hpp>
#include <nlohmann/json.hpp> // simply put torrent handle in json file.. problem solved
#include <libtorrent/torrent_info.hpp>
#include "../blockchain/blockchain.h"
#include "broadcast.h"
// Network is basically a Torrent.


std::string retrieve_peer() {
	std::ifstream ifs(blockchain::peer_path);
	nlohmann::json j = j.parse(ifs);
	return j[0]; // returning first element at the moment
}

void clear_peers() {
	std::ofstream ofs(blockchain::peer_path);
	ofs << "{}";
	ofs.close();
}

int recieve_chain() {
	struct sockaddr_in sockaddr;
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	int optional;
	char *buff; // recieving blockchain
	if(server_fd == 0) {
		return 1;
	}
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optional, sizeof(optional));
	sockaddr.sin_port = htons(PORT);
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = INADDR_ANY;
	if(bind(server_fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
		return 1;
	}
	if(listen(server_fd, 5) < 0) {
		return 1;
	}
	if(accept(server_fd, (struct sockaddr*)&sockaddr, (socklen_t*)sizeof(sockaddr)) < 0) {
		return 1;
	}
	read(server_fd, buff, 2048);
	std::cout << buff << std::endl;
	return 0;
}

int send_chain() {
	struct sockaddr_in sockaddr;
	int client_fd = socket(AF_INET, SOCK_STREAM, 0);
	std::ifstream ifs(blockchain::path);
	nlohmann::json j = j.parse(ifs);
	std::string json_str = j.dump();
	std::string next_peer = retrieve_peer();
	char json_char[std::strlen(json_str.c_str())];
	std::strncpy(json_char, json_str.c_str(), sizeof(json_char));
	json_char[sizeof(json_char)] = '\0';
	if(client_fd == 0) {
		return 1;
	}
	sockaddr.sin_port = htons(PORT);
	sockaddr.sin_family = AF_INET;
	if(inet_pton(AF_INET, next_peer.c_str(), &sockaddr.sin_addr) <= 0) {
		return 1;
	}
	if(connect(client_fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) != 0) {
		return 1;
	}
	send(client_fd, json_char, strlen(json_char), 0);
	return 0;
}

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
			std::cout << s.ip.address();
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



